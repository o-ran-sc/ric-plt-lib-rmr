// :vim ts=4 sw=4 noet:
/*
==================================================================================
	Copyright (c) 2019 Nokia
	Copyright (c) 2018-2019 AT&T Intellectual Property.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
==================================================================================
*/

/*
	Mnemonic:	lsender.c
	Abstract:	This is a simple sender, slimiar to sender.c, except that a timestamp
				is placed into the messages such that latency measurements can be 
				made.
				The message format is 'binary' defined by the lc_msg struct.

				Parms:	argv[1] == number of msgs to send (10)
						argv[2] == delay		(mu-seconds, 1000000 default)
						argv[3] == listen port

				Sender will send for at most 20 seconds, so if nmsgs and delay extend
				beyond that period the total number of messages sent will be less
				than n.

	Date:		18 April 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <pthread.h>


#include <rmr/rmr.h>

#define TRACE_SIZE 40		// bytes in header to provide for trace junk
#define	SUCCESS		(-1)

/*
	Thread data
*/
typedef struct tdata {
	int		id;					// the id we'll pass to RMr mt-call function NOT the thread id
	int		n2send;				// number of messages to send
	int		delay;				// ms delay between messages
	void*	mrc;				// RMr context
	int		state;
	int*	in_bins;			// latency count bins
	int*	out_bins;
	int		nbins;				// number of bins allocated
	long long in_max;
	long long out_max;
	int		out_oor;			// out of range count
	int		in_oor;
	int		in_bcount;				// total messages tracked in bins
	int		out_bcount;				// total messages tracked in bins
} tdata_t;


/*
	The message type placed into the payload.
*/
typedef struct lc_msg {
	struct timespec out_ts;			// time just before call executed
	struct timespec turn_ts;		// time at the receiver,  on receipt
	struct timespec in_ts;			// time received back by the caller
	int		out_retries;			// number of retries required to send
	int		turn_retries;			// number of retries required to send
} lc_msg_t;

// --------------------------------------------------------------------------------


static int sum( char* str ) {
	int sum = 0;
	int	i = 0;

	while( *str ) {
		sum += *(str++) + i++;
	}

	return sum % 255;
}

static void print_stats( tdata_t* td, int out, int hist ) {
	int sum;					// sum of latencies
	int csum = 0;				// cutoff sum
	int i95 = 0;				// bin for the 95th count
	int i99 = 0;				// bin for the 99th count
	int mean = -1;
	int cutoff_95;				// 95% of total messages
	int cutoff_99;				// 99% of total messages
	int	oor;
	int max;
	int j;

	if( out ) {
		cutoff_95 = .95 * (td->out_oor + td->out_bcount);
		cutoff_99 = .95 * (td->out_oor + td->out_bcount);
		oor = td->out_oor;
		max = td->out_max;
	} else {
		cutoff_95 = .95 * (td->in_oor + td->in_bcount);
		cutoff_99 = .95 * (td->in_oor + td->in_bcount);
		oor = td->in_oor;
		max = td->in_max;
	}

	sum = 0;
	for( j = 0; j < td->nbins; j++ ) {
		if( csum < cutoff_95 ) {
			i95++;
		}
		if( csum < cutoff_99 ) {
			i99++;
		}

		if( out ) {
			csum += td->out_bins[j];
			sum += td->out_bins[j] * j;
		} else {
			csum += td->in_bins[j];
			sum += td->in_bins[j] * j;
		}
	}

	if( out ) {
		if( td->out_bcount ) {
			mean = sum/(td->out_bcount);
		}
	} else {
		if( td->in_bcount ) {
			mean = sum/(td->in_bcount);
		}
	}

	if( hist ) {
		for( j = 0; j < td->nbins; j++ ) {
			fprintf( stderr, "%3d %d\n", j, out ? td->out_bins[j] : td->in_bins[j] );
		}
	}

	fprintf( stderr, "%s: oor=%d max=%.2fms  mean=%.2fms  95th=%.2fms 99th=%.2f\n", 
		out ? "out" : " in", oor, (double)max/1000000.0, (double)mean/100.0, (double) i95/100.0, i99/100.0 );
}

/*
	Given a message, compute the in/out and round trip latencies.
*/
static void compute_latency( tdata_t* td, lc_msg_t* lcm ) {
	long long out;
	long long turn;
	long long in;
	double rtl;		// round trip latency
	double outl;	// caller to receiver latency (out)
	double inl;		// receiver to caller latency (in)
	int bin;

	if( lcm == NULL || td == NULL ) {
		return;
	}

	out = (lcm->out_ts.tv_sec * 1000000000) + lcm->out_ts.tv_nsec;
	in = (lcm->in_ts.tv_sec * 1000000000) + lcm->in_ts.tv_nsec;
	turn = (lcm->turn_ts.tv_sec * 1000000000) + lcm->turn_ts.tv_nsec;

	if( in - turn > td->in_max ) {
		td->in_max = in - turn;
	}
	if( turn - out > td->out_max ) {
		td->out_max = turn-out;
	}
	
	bin = (turn-out) / 10000;			// 100ths of ms

#ifdef PRINT
	outl = ((double) turn - out) / 1000000.0;		// convert to ms
	inl = ((double) in - turn) / 1000000.0;
	rtl = ((double) in - out) / 1000000.0;

	fprintf( stderr, "outl = %5.3fms   inl = %5.3fms  rtl = %5.3fms bin=%d\n", outl, inl, rtl, bin );
#else

	bin = (turn - out) / 10000;			// 100ths of ms
	if( bin < td->nbins ) {
		td->out_bins[bin]++;
		td->out_bcount++;
	} else {
		td->out_oor++;
	}

	bin = (in - turn) / 10000;			// 100ths of ms
	if( bin < td->nbins ) {
		td->in_bins[bin]++;
		td->in_bcount++;
	} else {
		td->in_oor++;
	}

#endif
}

/*
	Compute the elapsed time between ts1 and ts2.
*/
static int elapsed( struct timespec* start_ts, struct timespec* end_ts ) {
	long long start;
	long long end;
	int bin;

	start = ( start_ts->tv_sec * 1000000000) + start_ts->tv_nsec;
	end = ( end_ts->tv_sec * 1000000000) + end_ts->tv_nsec;

	bin = (end - start) / 1000000;			// ms

	return bin;
}

/*
	The main thing.
*/
static void* send_msgs( void* mrc, int n2send, int delay, int retry ) {
	lc_msg_t*	lcm;						// pointer at the payload as a struct
	rmr_mbuf_t*		sbuf;					// send buffer
	int		count = 0;
	int		rt_count = 0;					// number of messages that had a retry on first send attempt
	int		good_count = 0;
	int		drops = 0;
	int		fail_count = 0;					// # of failure sends after first successful send
	int		successful = 0;					// set to true after we have a successful send
	char	xbuf[1024];						// build transaction string here
	int		xaction_id = 1;
	char*	tok;
	int		state = 0;
	struct timespec start_ts;
	struct timespec end_ts;
	int		mtype = 0;

	if( mrc == NULL ) {
		fprintf( stderr, "send_msg: bad mrc\n" );
	}

	sbuf = rmr_alloc_msg( mrc, 256 );	// alloc first send buffer; subsequent buffers allcoated on send

	snprintf( xbuf, 200, "%31d", xaction_id );
	while( count < n2send ) {								// we send n messages after the first message is successful
		lcm = (lc_msg_t *) sbuf->payload;

		rmr_bytes2xact( sbuf, xbuf, 32 );

		sbuf->mtype = 0;
		sbuf->mtype = mtype++;												// all go with the same type
		if( mtype > 9 ) {
			mtype = 0;
		}

		sbuf->len =  sizeof( *lcm );
		sbuf->state = RMR_OK;
		lcm->out_retries = 0;
		lcm->turn_retries = 0;
		clock_gettime( CLOCK_REALTIME, &lcm->out_ts );					// mark time out
		sbuf = rmr_send_msg( mrc, sbuf );

		if( sbuf && sbuf->state == RMR_ERR_RETRY ) {					// send not accepted
			if( retry || count == 0  ) {
				rt_count++;												// # messages that we retried beyond rmr's retry
			} else {
				if( delay ) 
					usleep( delay );
				fail_count++;			// send failed because we drop it
			}
		}

		count++;
		if( sbuf != NULL ) {
			if( ! successful ) {
				switch( sbuf->state ) {
					case RMR_OK:
						clock_gettime( CLOCK_REALTIME, &start_ts );
						successful = 1;
						good_count++;
						break;

					default:
						fprintf( stderr, "<SM> send error: rmr-state=%d ernro=%d\n", sbuf->state, errno );
						sleep( 1 );
						break;
				}
			} else {
				good_count += sbuf->state == RMR_OK;
			}
		} else {
			sbuf = rmr_alloc_msg( mrc, 512 );				// must have a sedn buffer at top
			drops++;
		}

		//if( count < n2send  &&  (count % 100) == 0  &&  delay > 0 ) {
		if( count < n2send && delay > 0 ) {
			if( count % 500 ) {
				usleep( delay );
			}
		}
	}
						
	clock_gettime( CLOCK_REALTIME, &end_ts );

	fprintf( stderr, "<SM> sending finished attempted=%d good=%d fails=%d rt=%d elapsed=%d ms, \n", count, good_count, fail_count, rt_count, elapsed( &start_ts, &end_ts ) );
	return NULL;
}

int main( int argc, char** argv ) {
	void* mrc;      						// msg router context
	rmr_mbuf_t*	rbuf = NULL;				// received on 'normal' flow
	char*	listen_port = "43086";			// largely unused here
	long	timeout = 0;
	int		delay = 100000;					// usec between send attempts
	int		nmsgs = 10;						// number of messages to send
	int		rmr_retries = 0;				// number of retries we allow rmr to do

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		delay = atoi( argv[2] );
	}
	if( argc > 4 ) {
		listen_port = argv[4];
	}
	if( argc > 3 ) {
		rmr_retries = atoi( argv[3] );
	}

	fprintf( stderr, "<LSEND> listen port: %s; sending %d messages; delay=%d\n", listen_port, nmsgs, delay );

	if( (mrc = rmr_init( listen_port, 1400, RMRFL_MTCALL )) == NULL ) {		// initialise with multi-threaded call enabled
		fprintf( stderr, "<LSEND> unable to initialise RMr\n" );
		exit( 1 );
	}

	fprintf( stderr, "\nsetting rmr retries: %d\n", rmr_retries );
	//if( rmr_retries != 1 ) {
		rmr_set_stimeout( mrc, rmr_retries );
	//}

	timeout = time( NULL ) + 20;		// give rmr 20s to find the route table (shouldn't need that much)
	while( ! rmr_ready( mrc ) ) {		// must have a route table before we can send; wait til RMr says it has one
		fprintf( stderr, "<LSEND> waiting for rmr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<LSEND> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<LSEND> rmr is ready; starting sender retries=%d\n", rmr_retries );

	send_msgs( mrc, nmsgs, delay, rmr_retries );

	fprintf( stderr, "pausing for drain\n" );
	sleep( 3 );
	fprintf( stderr, "closing down\n" );
	rmr_close( mrc );

	return 0;
}


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
	Mnemonic:	lcaller.c
	Abstract:	This is a simple sender which will send a series of messages using
				rmr_call().   Similar to caller.c the major difference is that
				a timestamp is placed into the message and the receiver is expected
				to add a timestamp before executing an rts call.  We can then
				compute the total round trip latency as well as the forward send
				latency.

				Overall, N threads are started each sending the desired number
				of messages and expecting an 'ack' for each. Each ack is examined
				to verify that the thread id placed into the message matches (meaning
				that the ack was delivered by RMr to the correct thread's chute.

				The message format is 'binary' defined by the lc_msg struct.

				Parms:	argv[1] == number of msgs to send (10)
						argv[2] == delay		(mu-seconds, 1000000 default)
						argv[3] == number of threads (3)
						argv[4] == listen port

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
			fprintf( stderr, "<LCALLER> hist: bin[%03d] %d\n", j, out ? td->out_bins[j] : td->in_bins[j] );
		}
	}

	fprintf( stderr, "<LCALLER> %s: oor=%d max=%.2fms  mean=%.2fms  95th=%.2fms 99th=%.2f\n", 
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
	Executed as a thread, this puppy will generate calls to ensure that we get the
	response back to the right thread, that we can handle threads, etc.
*/
static void* mk_calls( void* data ) {
	lc_msg_t*	lcm;						// pointer at the payload as a struct
	tdata_t*	control;
	rmr_mbuf_t*		sbuf;					// send buffer
	int		count = 0;
	int		ack_count = 0;
	int		rt_count = 0;					// number of messages requiring a spin retry
	int		ok_msg = 0;						// received messages that were sent by us
	int		bad_msg = 0;					// received messages that were sent by a different thread
	int		drops = 0;
	int		fail_count = 0;					// # of failure sends after first successful send
	int		successful = 0;					// set to true after we have a successful send
	char	xbuf[1024];						// build transaction string here
	int		xaction_id = 1;
	char*	tok;
	int		state = 0;

	if( (control  = (tdata_t *) data) == NULL ) {
		fprintf( stderr, "thread data was nil; bailing out\n" );
	}
	//fprintf( stderr, "<THRD> thread started with parallel call id=%d sending=%d delay=%d\n", control->id, control->n2send, control->delay );

	sbuf = rmr_alloc_msg( control->mrc, 512 );	// alloc first send buffer; subsequent buffers allcoated on send

	usleep( rand() % 777 );					// stagger starts a bit so that they all don't pile up on the first connections

	while( count < control->n2send ) {								// we send n messages after the first message is successful
		lcm = (lc_msg_t *) sbuf->payload;

		snprintf( xbuf, 200, "%31d", xaction_id );
		xaction_id += control->id;
		rmr_bytes2xact( sbuf, xbuf, 32 );

		sbuf->mtype = 5;												// all go with the same type
		sbuf->len =  sizeof( *lcm );
		sbuf->state = RMR_OK;
		lcm->out_retries = 0;
		lcm->turn_retries = 0;
		clock_gettime( CLOCK_REALTIME, &lcm->out_ts );					// mark time out
		sbuf = rmr_mt_call( control->mrc, sbuf, control->id, 1000 );	// send it (send returns an empty payload on success, or the original payload on fail/retry)

		if( sbuf && sbuf->state == RMR_ERR_RETRY ) {					// number of times we had to spin to send
			rt_count++;
		}
		while( sbuf != NULL && sbuf->state == RMR_ERR_RETRY ) {				// send blocked; keep trying
			lcm->out_retries++;
			sbuf = rmr_mt_call( control->mrc, sbuf, control->id, 100 );		// call and wait up to 100ms for a response
		}

		count++;
		if( sbuf != NULL ) {
			switch( sbuf->state ) {
				case RMR_OK:														// we should have a buffer back from the sender here
					lcm = (lc_msg_t *) sbuf->payload;
					clock_gettime( CLOCK_REALTIME, &lcm->in_ts );					// mark time back
					successful = 1;
					compute_latency( control, lcm );
					
					ack_count++;
					//fprintf( stderr, "%d  have received %d\n", control->id, count );
					break;

				default:
					fprintf( stderr, "<LCALLER> unexpected error: tid=%d rmr-state=%d ernro=%d\n", control->id, sbuf->state, errno );
					sbuf = rmr_alloc_msg( control->mrc, 512 );			// allocate a sendable buffer
					if( successful ) {
						fail_count++;							// count failures after first successful message
					} else {
						// some error (not connected likely), don't count this
						sleep( 1 );
					}
					break;
			}
		} else {
			//fprintf( stderr, "<THRD> tid=%-2d call finished, no sbuf\n", control->id );
			sbuf = rmr_alloc_msg( control->mrc, 512 );				// loop expects an subf
			drops++;
		}

		if( control->delay > 0 ) {
			usleep( control->delay );
		}
	}

	control->state = SUCCESS;
	fprintf( stderr, "<THRD> %d finished sent %d, received %d  messages\n", control->id, count, ack_count );
	return NULL;
}

int main( int argc, char** argv ) {
	void* mrc;      						// msg router context
	rmr_mbuf_t*	rbuf = NULL;				// received on 'normal' flow
	struct	epoll_event events[1];			// list of events to give to epoll
	struct	epoll_event epe;				// event definition for event to listen to
	int     ep_fd = -1;						// epoll's file des (given to epoll_wait)
	int		rcv_fd;    						// file des that NNG tickles -- give this to epoll to listen on
	int		nready;							// number of events ready for receive
	char*	listen_port = "43086";
	long	timeout = 0;
	int		delay = 100000;					// usec between send attempts
	int		nmsgs = 10;						// number of messages to send
	int		nthreads = 3;
	int		cutoff;
	int		sum;
	tdata_t*	cvs;						// vector of control blocks
	int			i;
	int			j;
	pthread_t*	pt_info;					// thread stuff
	int 	failures = 0;
	int		pings = 0;						// number of messages received on normal channel

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		delay = atoi( argv[2] );
	}
	if( argc > 3 ) {
		nthreads = atoi( argv[3] );
	}
	if( argc > 4 ) {
		listen_port = argv[4];
	}

	fprintf( stderr, "<CALL> listen port: %s; sending %d messages; delay=%d\n", listen_port, nmsgs, delay );

	if( (mrc = rmr_init( listen_port, 1400, RMRFL_MTCALL )) == NULL ) {		// initialise with multi-threaded call enabled
		fprintf( stderr, "<CALL> unable to initialise RMr\n" );
		exit( 1 );
	}

	//rmr_init_trace( mrc, TRACE_SIZE );

	if( (rcv_fd = rmr_get_rcvfd( mrc )) >= 0 ) {							// epoll only available from NNG -- skip receive later if not NNG
		if( rcv_fd < 0 ) {
			fprintf( stderr, "<CALL> unable to set up polling fd\n" );
			exit( 1 );
		}
		if( (ep_fd = epoll_create1( 0 )) < 0 ) {
			fprintf( stderr, "<CALL> [FAIL] unable to create epoll fd: %d\n", errno );
			exit( 1 );
		}
		epe.events = EPOLLIN;
		epe.data.fd = rcv_fd;

		if( epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe ) != 0 )  {
			fprintf( stderr, "<CALL> [FAIL] epoll_ctl status not 0 : %s\n", strerror( errno ) );
			exit( 1 );
		}
	} else {
		rmr_set_rtimeout( mrc, 0 );			// for nano we must set the receive timeout to 0; non-blocking receive
	}


	cvs = malloc( sizeof( tdata_t ) * nthreads );
	pt_info = malloc( sizeof( pthread_t ) * nthreads );
	if( cvs == NULL ) {
		fprintf( stderr, "<CALL> unable to allocate control vector\n" );
		exit( 1 );	
	}


	timeout = time( NULL ) + 20;		// give rmr 20s to find the route table (shouldn't need that much)
	while( ! rmr_ready( mrc ) ) {		// must have a route table before we can send; wait til RMr says it has one
		fprintf( stderr, "<CALL> waiting for rmr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<CALL> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<CALL> rmr is ready; starting threads\n" );

	for( i = 0; i < nthreads; i++ ) {
		cvs[i].mrc = mrc;
		cvs[i].id = i + 2;				// we pass this as the call-id to rmr, so must be >1
		cvs[i].delay = delay;
		cvs[i].n2send = nmsgs;
		cvs[i].state = 1;

		cvs[i].nbins = 100;
		cvs[i].out_bins = (int *) malloc( sizeof( int ) * cvs[i].nbins );
		cvs[i].in_bins = (int *) malloc( sizeof( int ) * cvs[i].nbins );
		memset( cvs[i].out_bins, 0, sizeof( int ) * cvs[i].nbins );
		memset( cvs[i].in_bins, 0, sizeof( int ) * cvs[i].nbins );

		pthread_create( &pt_info[i], NULL, mk_calls, &cvs[i] );		// kick a thread
	}

	timeout = time( NULL ) + 20;
	i = 0;
	while( nthreads > 0 ) {
		if( cvs[i].state < 1 ) {			// states 0 or below indicate done. 0 == failure, -n == success
			//print_stats( &cvs[i], 1, i == 0 );
			print_stats( &cvs[i], 1, 0 );
			print_stats( &cvs[i], 0, 0 );

			nthreads--;
			if( cvs[i].state == 0 ) {
				failures++;
			}
			i++;
		} else {
		//	sleep( 1 );
			rbuf = rmr_torcv_msg( mrc, rbuf, 1000 );
			if( rbuf != NULL && rbuf->state != RMR_ERR_RETRY ) {
				pings++;
				rmr_free_msg( rbuf );
				rbuf = NULL;
			}
		}
		if( time( NULL ) > timeout ) {
			failures += nthreads;
			fprintf( stderr, "<CALL> timeout waiting for threads to finish; %d were not finished\n", nthreads );
			break;
		}
	}

	fprintf( stderr, "<CALL> [%s] failing threads=%d  pings reeived=%d\n", failures == 0 ? "PASS" : "FAIL",  failures, pings );
	sleep( 2 );
	rmr_close( mrc );

	return failures > 0;
}


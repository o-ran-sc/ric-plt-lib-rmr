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
	Mnemonic:	mt_listener.c
	Abstract:	This simple application runs multiple "listener" threads. Each thread
				receives from a single RMR context to validate the ability spin
				several listening threads in an application.

				Message format is:
					ck1 ck2|<msg-txt> @ tid<nil>

				Ck1 is the simple check sum of the msg-text (NOT includeing <nil>)
				Ck2 is the simple check sum of the trace data which is a nil terminated
				series of bytes.
				tid is the thread id assigned by the main thread.

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
#include "time_tools.c"		// our time based test tools

#define TRACE_SIZE 40		// bytes in header to provide for trace junk
#define WBUF_SIZE 2048

/*
	Thread data
*/
typedef struct tdata {
	int	id;					// the id we'll pass to RMr mt-call function NOT the thread id
	int n2get;				// number of messages to expect
	int delay;				// max delay waiting for n2get messages
	void* mrc;				// RMr context
	int	state;
} tdata_t;



// --------------------------------------------------------------------------------


static int sum( char* str ) {
	int sum = 0;
	int i = 0;

	while( *str ) {
		sum += *(str++) + i++;
	}

	return sum % 255;
}

/*
	Split the message at the first sep and return a pointer to the first
	character after.
*/
static char* split( char* str, char sep ) {
	char*	s;

	s = strchr( str, sep );

	if( s ) {
		return s+1;
	}

	//fprintf( stderr, "<RCVR> no pipe in message: (%s)\n", str );
	return NULL;
}

/*
	Executed as a thread, this puppy will listen for messages and report
	what it receives.
*/
static void* mk_calls( void* data ) {
	tdata_t*	control;
	rmr_mbuf_t*		msg = NULL;					// message
	int*	count_bins = NULL;
	char*	wbuf = NULL;
	char	buf2[128];
	int		i;
	int		state = 0;
	char*	msg_data;						// bits after checksum info in payload
	long	good = 0;						// counters
	long	bad = 0;
	long	bad_tr = 0;
	long	count = 0;						// total msgs received
	struct timespec	start_ts;
	struct timespec	end_ts;
	int		elap;							// elapsed time to receive messages
	time_t	timeout;

	count_bins = (int *) malloc( sizeof( int ) * 11 );

	wbuf = (char *) malloc( sizeof( char ) * WBUF_SIZE );

	if( (control  = (tdata_t *) data) == NULL ) {
		fprintf( stderr, "thread data was nil; bailing out\n" );
	}
	fprintf( stderr, "<THRD> id=%d thread receiver started expecting=%d messages timeout=%d seconds\n", 
		control->id, control->n2get, control->delay );

	timeout = time( NULL ) + control->delay;			// max time to wait for a good message
	while( count < control->n2get ) {		// wait for n messages -- no timeout
		msg = rmr_torcv_msg( control->mrc, msg, 1000 );				//  pop after ~1 second

		if( msg ) {
			//fprintf( stderr, "<THRD> id=%d got type=%d state=%s msg=(%s)\n", control->id, msg->mtype, msg->state == RMR_OK ? "OK" : "timeout", msg->payload );
			if( msg->state == RMR_OK ) {
				if( good == 0 ) {				// mark time of first good message
					set_time( &start_ts );
				}
				set_time( &end_ts );		// mark the time of last good message

				if( (msg_data = split( msg->payload, '|'  )) != NULL ) {
					if( sum( msg_data ) == atoi( (char *) msg->payload ) ) {
						good++;
					} else {
						fprintf( stderr, "<RCVR> chk sum bad: computed=%d expected;%d (%s)\n", sum( msg_data ), 
							atoi( msg->payload ), msg_data );
						bad++;
					}

					if( (msg_data = split( msg->payload, ' ' )) != NULL ) {			// data will point to the chksum for the trace data
						state = rmr_get_trace( msg, wbuf, 1024 );				// should only copy upto the trace size; we'll check that
						if( state > 128 || state < 0 ) {
							fprintf( stderr, "trace data size listed unexpectedly long: %d\n", state );
						} else {
							if( state  &&  sum( wbuf ) != atoi( msg_data ) ) {
								fprintf( stderr, "<RCVR> trace chk sum bad: computed=%d expected;%d len=%d (%s)\n", sum( wbuf ), 
										atoi( msg_data ), state, wbuf );
								bad_tr++;
							}
						}
					}
				} else {
					good++;		// nothing to check, assume good
				}
				count++;

				if( msg->mtype >= 0 && msg->mtype <= 10 ) {
					count_bins[msg->mtype]++;
				}
			}
		} else {
			fprintf( stderr, "<THRD> id=%d timeout with nil msg\n", control->id );
		}

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<THRD> id=%d timeout before receiving %d messages\n", control->id, control->n2get );
			break;
		}
	}
	elap = elapsed( &start_ts, &end_ts, ELAP_MS );
	if( elap > 0 ) {
		fprintf( stderr, "<THRD> id=%d received %ld messages in %d ms rate = %ld msg/sec\n", control->id, count, elap, (count/elap)*1000 );
	} else {
		fprintf( stderr, "<THRD> id=%d runtime too short to compute received rate\n", control->id );
	}

	snprintf( wbuf, WBUF_SIZE, "<THRD> id=%d histogram: ", control->id );		// build histogram so we can write with one fprintf call
	for( i = 0; i < 11; i++ ) {
		snprintf( buf2, sizeof( buf2 ), "%5d ", count_bins[i] );
		strcat( wbuf, buf2 );
	}
	fprintf( stderr, "%s\n", wbuf );

	fprintf( stderr, "<THRD> id=%d %ld messages %ld good %ld bad\n", control->id, count, good, bad );

	control->state = bad > 0 ? -1 : 0;						// set to indicate done and <0 to indicate some failure
	control->state += count < control->n2get ? -2 : 0;
	return NULL;
}

int main( int argc, char** argv ) {
	void* mrc;      						// msg router context
	rmr_mbuf_t*	rbuf = NULL;				// received on 'normal' flow
	struct	epoll_event events[1];			// list of events to give to epoll
	struct	epoll_event epe;				// event definition for event to listen to
	int     ep_fd = -1;						// epoll's file des (given to epoll_wait)
	char*	listen_port = "43086";
	long	timeout = 0;					// time the main thread will pop if listeners have not returned
	int		delay = 30;						// max time to wait for n messages
	int		nmsgs = 10;						// number of messages to expect
	int		nthreads = 3;					// default number of listener threads
	tdata_t*	cvs;						// vector of control blocks
	int			i;
	pthread_t*	pt_info;					// thread stuff
	int 	failures = 0;

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

	fprintf( stderr, "<MTL> listen port: %s; sending %d messages; delay=%d\n", listen_port, nmsgs, delay );

	if( (mrc = rmr_init( listen_port, 1400, 0 )) == NULL ) {
		fprintf( stderr, "<MTL> unable to initialise RMr\n" );
		exit( 1 );
	}

	rmr_init_trace( mrc, TRACE_SIZE );

	cvs = malloc( sizeof( tdata_t ) * nthreads );
	pt_info = malloc( sizeof( pthread_t ) * nthreads );
	if( cvs == NULL ) {
		fprintf( stderr, "<MTL> unable to allocate control vector\n" );
		exit( 1 );	
	}

	timeout = time( NULL ) + 20;		// give rmr 20s to find the route table (shouldn't need that much)
	while( ! rmr_ready( mrc ) ) {		// must have a route table before we can send; wait til RMr says it has one
		fprintf( stderr, "<MTL> waiting for rmr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<MTL> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<MTL> rmr is ready; starting threads\n" );

	for( i = 0; i < nthreads; i++ ) {
		cvs[i].mrc = mrc;
		cvs[i].id = i + 2;				// we pass this as the call-id to rmr, so must be >1
		cvs[i].delay = delay;
		cvs[i].n2get = nmsgs;
		cvs[i].state = 1;

		fprintf( stderr, "kicking %d i=%d\n", i+2, i );
		pthread_create( &pt_info[i], NULL, mk_calls, &cvs[i] );		// kick a thread
	}

	timeout = time( NULL ) + 300;	// wait up to 5 minutes
	i = 0;
	while( nthreads > 0 ) {
		if( cvs[i].state < 1 ) {			// states 0 or below indicate done. 0 == good; <0 is failure
			nthreads--;
			if( cvs[i].state < 0 ) {
				failures++;
			}
			i++;
		}
		
		if( time( NULL ) > timeout ) {
			failures += nthreads;
			fprintf( stderr, "<MTL> timeout waiting for threads to finish; %d were not finished\n", nthreads );
			break;
		}

		sleep( 1 );
	}

	fprintf( stderr, "<MTL> [%s] failing threads=%d\n", failures == 0 ? "PASS" : "FAIL",  failures );
	rmr_close( mrc );

	return failures > 0;
}


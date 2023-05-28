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
	Mnemonic:	caller.c
	Abstract:	This is a simple sender which will send a series of messages using
				rmr_call().  N threads are started each sending the desired number
				of messages and expecting an 'ack' for each. Each ack is examined
				to verify that the thread id placed into the message matches (meaning
				that the ack was delivered by RMr to the correct thread's chute.

				In addition, the main thread listens for messages in order to verify
				that a main or receiving thread can receive messages concurrently
				while call acks are pending and being processed.

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

#define TRACE_SIZE 40		// bytes in header to provide for trace junk
#define WBUF_SIZE 2048

/*
	Thread data
*/
typedef struct tdata {
	int	id;					// the id we'll pass to RMr mt-call function NOT the thread id
	int n2send;				// number of messages to send
	int delay;				// ms delay between messages
	void* mrc;				// RMr context
	int	state;
} tdata_t;



// --------------------------------------------------------------------------------


static int sum( char* str ) {
	int sum = 0;
	int	i = 0;

	while( *str ) {
		sum += *(str++) + i++;
	}

	return sum % 255;
}



/*
	Executed as a thread, this puppy will generate calls to ensure that we get the
	response back to the right thread, that we can handle threads, etc.
*/
static void* mk_calls( void* data ) {
	tdata_t*	control;
	rmr_mbuf_t*		sbuf;					// send buffer
	int		count = 0;
	int		rt_count = 0;					// number of messages requiring a spin retry
	int		ok_msg = 0;						// received messages that were sent by us
	int		bad_msg = 0;					// received messages that were sent by a different thread
	int		drops = 0;
	int		fail_count = 0;					// # of failure sends after first successful send
	int		successful = 0;					// set to true after we have a successful send
	char*	wbuf = NULL;
	char	xbuf[1024];						// build transaction string here
	char	trace[1024];
	int		xaction_id = 1;
	char*	tok;
	int		state = 0;

	wbuf = (char *) malloc( sizeof( char ) * WBUF_SIZE );

	if( (control  = (tdata_t *) data) == NULL ) {
		fprintf( stderr, "thread data was nil; bailing out\n" );
	}
	//fprintf( stderr, "<THRD> thread started with parallel call id=%d sending=%d delay=%d\n", control->id, control->n2send, control->delay );

	sbuf = rmr_alloc_msg( control->mrc, 512 );	// alloc first send buffer; subsequent buffers allcoated on send

	memset( trace, 0, sizeof( trace ) );
	while( count < control->n2send ) {								// we send n messages after the first message is successful
		snprintf( trace, 100, "%lld", (long long) time( NULL ) );
		rmr_set_trace( sbuf, trace, TRACE_SIZE );					// fully populate so we dont cause a buffer realloc

		snprintf( wbuf, WBUF_SIZE, "count=%d tr=%s %d stand up and cheer! @ %d", count, trace, rand(), control->id );
		snprintf( sbuf->payload, 300, "%d %d|%s", sum( wbuf ), sum( trace ), wbuf );
		snprintf( xbuf, 200, "%31d", xaction_id );
		rmr_bytes2xact( sbuf, xbuf, 32 );

		sbuf->mtype = 5;								// mtype is always 5 as the test receiver acks just mtype 5 messages
		sbuf->len =  strlen( sbuf->payload ) + 1;		// our receiver likely wants a nice acsii-z string
		sbuf->state = 0;
		sbuf = rmr_mt_call( control->mrc, sbuf, control->id, 1000 );	// send it (send returns an empty payload on success, or the original payload on fail/retry)

		if( sbuf && sbuf->state == RMR_ERR_RETRY ) {					// number of times we had to spin to send
			rt_count++;
		}
		while( sbuf != NULL && sbuf->state == RMR_ERR_RETRY ) {				// send blocked; keep trying
			sbuf = rmr_mt_call( control->mrc, sbuf, control->id, 5000 );	// call and wait up to 5s for a response
		}

		if( sbuf != NULL ) {
			switch( sbuf->state ) {
				case RMR_OK:							// we should have a buffer back from the sender here
					successful = 1;
					if( (tok = strchr( sbuf->payload, '@' )) != NULL ) {
						if( atoi( tok+1 ) == control->id ) {
							//fprintf( stderr, "<THRD> tid=%-2d ok  ack\n", control->id );
							ok_msg++;
						} else {
							bad_msg++;
							//fprintf( stderr, "<THRD> tid=%-2d bad ack %s\n", control->id, sbuf->payload );
						}
					}
					//fprintf( stderr, "<THRD> tid=%-2d call returned valid msg: %s\n", control->id, sbuf->payload );
					// future -- verify that we see our ID at the end of the message
					count++;
					break;

				default:
					fprintf( stderr, "<CALLR> unexpected error: tid=%d rmr-state=%d ernro=%d\n", control->id, sbuf->state, errno );
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
			count++;
		}

		if( control->delay > 0 ) {
			usleep( control->delay );
		}
	}

	state = 1;
	if( ok_msg < (control->n2send-1) || bad_msg > 0 ) {		// allow one drop to pass
		state = 0;
	}
	if( count < control->n2send ) {
		state = 0;
	}

	control->state = -state;				// signal inactive to main thread; -1 == pass, 0 == fail
	fprintf( stderr, "<THRD> [%s]  tid=%-2d sent=%d  ok-acks=%d bad-acks=%d  drops=%d failures=%d retries=%d\n",
		state ? "PASS" : "FAIL",  control->id, count, ok_msg, bad_msg, drops, fail_count, rt_count );


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
	tdata_t*	cvs;						// vector of control blocks
	int			i;
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

	rmr_init_trace( mrc, TRACE_SIZE );

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

		pthread_create( &pt_info[i], NULL, mk_calls, &cvs[i] );		// kick a thread
	}

	timeout = time( NULL ) + 20;
	i = 0;
	while( nthreads > 0 ) {
		if( cvs[i].state < 1 ) {			// states 0 or below indicate done. 0 == failure, -n == success
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
	rmr_close( mrc );

	return failures > 0;
}


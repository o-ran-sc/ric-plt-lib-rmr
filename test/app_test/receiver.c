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
	Mnemonic:	rmr_rcvr.c
	Abstract:	This is a very simple receiver that does nothing but listen
				for messages and write stats every so often to the tty.

				The receiver expects messages which have some trace information
				and a message format of:
					ck1 ck2|<msg text><nil>

				ck1 is a simple checksum of the message text (NOT including the
				nil at the end of the string.

				ck2 is a simple checksum of the trace data which for the purposes
				of testing is assumed to have a terminating nil to keep this simple.

				Good messages are messages where both computed checksums match
				the ck1 and ck2 values.

				The receiver will send an 'ack' message back to the sender for
				all type 5 messages received.

				The sender and receiver can be run on the same host/container
				or on different hosts. The route table is the key to setting
				things up properly.  See the sender code for rt information.

				Define these environment variables to have some control:
					RMR_SEED_RT -- path to the static routing table
					RMR_RTG_SVC -- port to listen for RTG connections

				Compile time options
				if -DMTC is defined on the compile command, then RMr is initialised
				with the multi-threaded receive thread rather than using the same
				process receive function. All other functions in the receiver are
				the same.

	Date:		18 April 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <rmr/rmr.h>

static int sum( char* str ) {
	int sum = 0;
	int	i = 0;

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

	fprintf( stderr, "<RCVR> no pipe in message: (%s)\n", str );
	return NULL;
}

int main( int argc, char** argv ) {
	void* mrc;      					// msg router context
	rmr_mbuf_t* msg = NULL;				// message received
	int i;
	int		state;
	int		errors = 0;
	char*	listen_port = "4560";
	long count = 0;						// total received
	long good = 0;						// good palyload buffers
	long bad = 0;						// payload buffers which were not correct
	long bad_tr = 0;					// trace buffers that were not correct
	long bad_sid = 0;					// bad subscription ids
	long timeout = 0;
	char*	data;
	int		nmsgs = 10;					// number of messages to stop after (argv[1] overrides)
	int		rt_count = 0;				// retry count
	long ack_count = 0;					// number of acks sent
	int		count_bins[11];				// histogram bins based on msg type (0-10)
	char	wbuf[1024];					// we'll pull trace data into here, and use as general working buffer
	char	sbuf[128];					// short buffer

	data = getenv( "RMR_RTG_SVC" );
	if( data == NULL ) {
		setenv( "RMR_RTG_SVC", "19289", 1 );		// set one that won't collide with the sender if on same host
	}

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		listen_port = argv[2];
	}

	memset( count_bins, 0, sizeof( count_bins ) );

	fprintf( stderr, "<RCVR> listening on port: %s for a max of %d messages\n", listen_port, nmsgs );

#ifdef MTC
	mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_MTCALL ); // start RMr in mt-receive mode

#else
	mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_NONE );	// start your engines!
#endif
	if( mrc == NULL ) {
		fprintf( stderr, "<RCVR> ABORT:  unable to initialise RMr\n" );
		exit( 1 );
	}

	timeout = time( NULL ) + 20;
	while( ! rmr_ready( mrc ) ) {								// wait for RMr to load a route table
		fprintf( stderr, "<RCVR> waiting for RMr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<RCVR> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<RCVR> rmr now shows ready, listening begins\n" );

	timeout = time( NULL ) + 20;
	while( count < nmsgs ) {
		msg = rmr_torcv_msg( mrc, msg, 1000 );				// wait for about 1s so that if sender never starts we eventually escape

		if( msg ) {
			if( msg->state == RMR_OK ) {
				if( (data = split( msg->payload, '|'  )) != NULL ) {
					if( sum( data ) == atoi( (char *) msg->payload ) ) {
						good++;
					} else {
						fprintf( stderr, "<RCVR> chk sum bad: computed=%d expected;%d (%s)\n", sum( data ), atoi( msg->payload ), data );
						bad++;
					}
				}

				if( (data = split( msg->payload, ' ' )) != NULL ) {			// data will point to the chksum for the trace data
					state = rmr_get_trace( msg, wbuf, 1024 );				// should only copy upto the trace size; we'll check that
					if( state > 128 || state < 1 ) {
						fprintf( stderr, "trace data size listed unexpectedly long: %d\n", state );
					}
					if( sum( wbuf ) != atoi( data ) ) {
						fprintf( stderr, "<RCVR> trace chk sum bad: computed=%d expected;%d len=%d (%s)\n", sum( wbuf ), atoi( data ), state, wbuf );
						bad_tr++;
					}
				}
				count++;									// messages received for stats output

				if( msg->mtype < 3 ) {							// count number of properly set subscription id
					if( msg->sub_id != msg->mtype * 10 ) {
						bad_sid++;
					}
				}

				if( msg->mtype >= 0 && msg->mtype <= 10 ) {
					count_bins[msg->mtype]++;
				}

				if( msg->mtype == 5 ) {											// send an ack; sender will count but not process, so data in message is moot
					msg = rmr_rts_msg( mrc, msg );
					rt_count = 1000;
					while( rt_count > 0 && msg != NULL && msg->state == RMR_ERR_RETRY ) {		// to work right in nano we need this :(
						if( ack_count < 1 ) {									// 1st ack, so we need to connect, and we'll wait for that
							sleep( 1 );
						}
						rt_count--;
						msg = rmr_rts_msg( mrc, msg );							// we don't try to resend if this returns retry
					}
					if( msg && msg->state == RMR_OK ) {							// if it eventually worked
						ack_count++;
					}
				}
			}
		}

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "receiver timed out\n" );
			errors++;
			break;
		}
	}

	wbuf[0] = 0;
	for( i = 0; i < 11; i++ ) {
		snprintf( sbuf, sizeof( sbuf ), "%6d ", count_bins[i] );
		strcat( wbuf, sbuf );
	}

	fprintf( stderr, "<RCVR> mtype histogram: %s\n", wbuf );
	fprintf( stderr, "<RCVR> [%s] %ld messages;  good=%ld  acked=%ld bad=%ld  bad-trace=%ld bad-sub_id=%ld\n", 
		!!(errors + bad + bad_tr) ? "FAIL" : "PASS", count, good, ack_count, bad, bad_tr, bad_sid );

	sleep( 2 );									// let any outbound acks flow before closing

	rmr_close( mrc );
	return !!(errors + bad + bad_tr);			// bad rc if any are !0
}


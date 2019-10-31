// vim: ts=4 sw=4 noet:
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
	Mnemonic:	ex_rts_receiver.c
	Abstract:	This receiver is a bit more specitalised with respect to 
				expclicitly testing the abilty to expand a message payload
				length when it is necessary for an appliction to send a response
				to a message originator where the rts payload is larger than the
				received message.

				This specific test is accomplished by responding to all messages
				with a response which is 1024 bytes in length. This means that
				any message received which is smaller than 1K bytes will have to
				be expanded.  Further, all 1024 bytes will be generated into the
				response, with a checksum, and the expectation is that the message
				originator (assumed to be the v_sender) will verify that the 
				message size is as expected, and that the checksum matches.

				This test is concerned only with functionality, and not latency
				or speed, and as such there as not been any attempt to make the 
				building of responses etc. efficent.
				
				Compile time options:
						DEBUG: write extra output about bad messages etc.
						MTC:	enable the multi-thraded call support in RMR

	Date:		28 October 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <rmr/rmr.h>

#define HDR_SIZE 64							// the size of the header we place into the message
#define MSG_SIZE 1024						// the size of the message that will be sent (hdr+payload)
#define DATA_SIZE (MSG_SIZE-HDR_SIZE)		// the actual 'data' length returned in the ack msg


#ifndef DEBUG
#define DEBUG 0
#endif

#include "test_support.c"					// checksum, header gen, etc.

int main( int argc, char** argv ) {
	void* mrc;      					// msg router context
	rmr_mbuf_t* msg = NULL;				// message received
	int 	i;
	int		j;
	int		state;
	int		errors = 0;
	char*	listen_port = "4560";
	long	count = 0;					// total received
	long	good = 0;					// good palyload buffers
	long	bad = 0;					// payload buffers which were not correct
	long	bad_sid = 0;				// bad subscription ids
	long	resized = 0;				// number of messages we had to resize before replying
	long	timeout = 0;
	long	rpt_timeout = 0;			// next stats message
	char*	data;
	int		nmsgs = 10;					// number of messages to stop after (argv[1] overrides)
	int		rt_count = 0;				// retry count
	long	ack_count = 0;				// number of acks sent
	int		count_bins[11];				// histogram bins based on msg type (0-10)
	char	wbuf[1024];					// we'll pull trace data into here, and use as general working buffer
	char	sbuf[128];					// short buffer
	char	ack_header[64];				// we'll put checksum and maybe other stuff in the header of the response for validation
	char	ack_data[DATA_SIZE];		// data randomly generated for each response
	int		need;						// amount of something that we need
	int		sv;							// checksum valu

	data = getenv( "RMR_RTG_SVC" );
	if( data == NULL ) {
		setenv( "RMR_RTG_SVC", "19289", 1 );		// set one that won't collide with the sender if on same host
	}

	if( argc > 1 ) {
		listen_port = argv[1];
	}

	memset( count_bins, 0, sizeof( count_bins ) );

	fprintf( stderr, "<RCVR> listening on port: %s for a max of %d messages\n", listen_port, nmsgs );

#ifdef MTC
	fprintf( stderr, "<RCVR> starting in multi-threaded mode\n" );
	mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_MTCALL ); // start RMr in mt-receive mode
#else
	fprintf( stderr, "<RCVR> starting in direct receive mode\n" );
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
	while( 1 ) {
		msg = rmr_torcv_msg( mrc, msg, 1000 );				// break about every 1s so that if sender never starts we eventually escape

		if( msg ) {
			if( msg->state == RMR_OK ) {
				if( validate_msg( msg->payload, msg->len ) ) {		// defrock the header, then verify lengths and chksum
					good++;
				} else {
					bad++;
				}
				count++;										// total messages received for stats output

				if( msg->mtype >= 0 && msg->mtype <= 10 ) {
					count_bins[msg->mtype]++;
				}

				need = generate_payload( ack_header, ack_data, 0, 0 );		// create an ack w/ random payload in payload, and set data in header
				if( rmr_payload_size( msg ) < need ) {					// received message too small
					resized++;
					msg = rmr_realloc_payload( msg, need, 0, 0 );		// reallocate the message with a payload big enough
					if( msg == NULL ) {
						fprintf( stderr, "[ERR] realloc returned a nil pointer\n" );
						continue;
					}
				}

				fill_payload( msg, ack_header, 0, ack_data, 0 );		// push headers (with default lengths) into message
				msg->mtype = 99;
				msg->sub_id = -1;
				msg->len = need;

				msg = rmr_rts_msg( mrc, msg );							// return our ack message
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

				timeout = time( NULL ) +20;
			}
		}

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<RCVR> stopping, no recent messages received\n" );
			break;
		} else {
			if( time( NULL ) > rpt_timeout ) {
				fprintf( stderr, "<RCVR> %ld msgs=%ld good=%ld  acked=%ld bad=%ld resized=%ld\n", (long) time( NULL ), count, good, ack_count, bad, resized );

				rpt_timeout = time( NULL ) + 5;
			}
		}
	}

	wbuf[0] = 0;
	for( i = 0; i < 11; i++ ) {
		snprintf( sbuf, sizeof( sbuf ), "%6d ", count_bins[i] );
		strcat( wbuf, sbuf );
	}

	fprintf( stderr, "<RCVR> mtype histogram: %s\n", wbuf );
	fprintf( stderr, "<RCVR> [%s] %ld messages;  good=%ld  acked=%ld bad=%ld  resized=%ld bad-sub_id=%ld\n", 
		!!(errors + bad) ? "FAIL" : "PASS", count, good, ack_count, bad, resized,  bad_sid );

	sleep( 2 );									// let any outbound acks flow before closing

	rmr_close( mrc );
	return !!(errors + bad);			// bad rc if any are !0
}


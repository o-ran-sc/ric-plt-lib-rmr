// : vi ts=4 sw=4 noet :
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
	Mmemonic:	rmr_api_static_test.c
	Abstract:	Specific tests related to the API functions in rmr_nng.c/rmr.c.
				This should be included by a driver, but only the main RMr
				driver and there likely not be a specific stand alone driver
				for just this small set of tests because of the depth of the
				library needed to test at this level.

				The message buffer specific API tests are in a different static
				module.  API functions tested here are:
					 rmr_close
					 rmr_get_rcvfd
					 rmr_ready
					 rmr_init
					 rmr_set_rtimeout
					 rmr_set_stimeout
					 rmr_rcv_specific
					 rmr_torcv_msg
					 rmr_rcv_msg
					 rmr_call
					 rmr_rts_msg
					 rmr_send_msg
					 rmr_mtosend_msg
					 rmr_free_msg

	Author:		E. Scott Daniels
	Date:		5 April 2019
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include "../src/common/include/rmr.h"
#include "../src/common/include/rmr_agnostic.h"
//#include "../src/common/src/ring_static.c"

/*
	Send a 'burst' of messages to drive some send retry failures to increase RMr coverage
	by handling the retry caee.
*/
static void send_n_msgs( void* ctx, int n ) {
	rmr_mbuf_t*	msg;			// message buffers
	int i;

	msg = rmr_alloc_msg( ctx,  1024 );
	if( ! msg ) {
		return;
	}

	for( i = 0; i < n; i++ ) {
		//fprintf( stderr, "mass send\n" );
		msg->len = 100;
		msg->mtype = 1;
		msg->state = 999;
		errno = 999;
		msg = rmr_send_msg( ctx, msg );
	}
}

static int rmr_api_test( ) {
	int		errors = 0;
	void*	rmc;				// route manager context
	void*	rmc2;				// second context for non-listener init
	rmr_mbuf_t*	msg;			// message buffers
	rmr_mbuf_t*	msg2;
	int		v = 0;					// some value
	char	wbuf[128];
	int		i;
	int		state;

	v = rmr_ready( NULL );
	errors += fail_if( v != 0, "rmr_ready returned true before initialisation" );

	if( (rmc = rmr_init( "4560", 1024, FL_NOTHREAD )) == NULL ) {
		fail_if_nil( rmc, "rmr_init returned a nil pointer" );
		return 1;
	}

	if( (rmc2 = rmr_init( ":6789", 1024, FL_NOTHREAD )) == NULL ) {		// init without starting a thread
		errors += fail_if_nil( rmc, "rmr_init returned a nil pointer for non-threaded init" );
	}

	free_ctx( rmc2 );			// coverage

	if( (rmc2 = rmr_init( NULL, 1024, FL_NOTHREAD )) == NULL ) {			// drive default port selector code
		errors += fail_if_nil( rmc, "rmr_init returned a nil pointer when driving for default port" );
	}

	v = rmr_ready( rmc );		// unknown return; not checking at the moment

	msg = rmr_alloc_msg( NULL,  1024 );									// should return nil pointer
	errors += fail_not_nil( msg, "rmr_alloc_msg didn't return nil when given nil context" );

	msg = rmr_alloc_msg( rmc, 2048 );				// allocate larger than default size given on init
	errors += fail_if_nil( msg, "rmr_alloc_msg returned nil msg pointer" );

	v = rmr_payload_size( NULL );
	errors += fail_if( v >= 0, "rmr_payload_size returned valid size for nil message" );
	errors += fail_if( errno == 0, "rmr_payload_size did not set errno on failure" );

	v = rmr_payload_size( msg );
	if( v >= 0 ) {
		errors += fail_not_equal( v, 2048, "rmr_payload_size returned invalid size (a) instead of expected size (b)" );
		errors += fail_if( errno != 0, "rmr_payload_size did not clear errno on success" );
	} else {
		errors += fail_if( v < 0, "rmr_payload_size returned invalid size for good message" );
	}

	v = rmr_get_rcvfd( NULL );
	errors += fail_if( v >= 0, "rmr_get_rcvfd returned a valid file descriptor when given nil context" );
	v = rmr_get_rcvfd( rmc );
	errors += fail_if( v < 0, "rmr_get_rcvfd did not return a valid file descriptor" );

	msg2 = rmr_send_msg( NULL, NULL );			// drive for coverage
	errors += fail_not_nil( msg2, "send_msg returned msg pointer when given a nil message and context" );

	msg->state = 0;
	msg = rmr_send_msg( NULL, msg );
	errors += fail_if( msg->state == 0, "rmr_send_msg did not set msg state when msg given with nil context" );

	// --- sends will fail with a no endpoint error until a dummy route table is set, so we test fail case first.
	msg->len = 100;
	msg->mtype = 1;
	msg->state = 999;
	errno = 999;
	msg = rmr_send_msg( rmc, msg );
	errors += fail_if_nil( msg, "send_msg_ did not return a message on send" );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_ERR_NOENDPT, "send_msg did not return no endpoints before rtable added" );
		errors += fail_if( errno == 0, "send_msg did not set errno" );
	}

	gen_rt( rmc );		// --- after this point there is a dummy route table so send and rts calls should be ok

	msg->len = 100;
	msg->mtype = 1;
	msg->state = 999;
	errno = 999;
	msg = rmr_send_msg( rmc, msg );
	errors += fail_if_nil( msg, "send_msg_ did not return a message on send" );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "send_msg returned bad status for send that should work" );
		errors += fail_if( errno != 0, "send_msg set errno for send that should work" );
		v = rmr_payload_size( msg );
		errors += fail_if( v != 2048, "send_msg did not allocate new buffer with correct size" );
	}

	rmr_set_stimeout( NULL, 0 );
	rmr_set_stimeout( rmc, 20 );
	rmr_set_stimeout( rmc, -1 );
	rmr_set_rtimeout( NULL, 0 );
	rmr_set_rtimeout( rmc, 20 );
	rmr_set_rtimeout( rmc, -1 );

	msg2 = rmr_rcv_msg( NULL, NULL );
	errors += fail_if( msg2 != NULL, "rmr_rcv_msg returned msg when given nil context and msg" );

	msg2 = rmr_rcv_msg( rmc, NULL );
	errors += fail_if( msg2 == NULL, "rmr_rcv_msg returned nil msg when given nil msg" );
	if( msg2 ) {
		errors += fail_not_equal( msg2->state, RMR_OK, "receive given nil message did not return msg with good state" );
	}

	msg = rmr_rcv_msg( rmc, msg );
	if( msg ) {
		errors += fail_if( msg->state != RMR_OK, "rmr_rcv_msg did not return an ok state" );
		errors += fail_not_equal( msg->len, 129, "rmr_rcv_msg returned message with invalid len" );
	} else {
		errors += fail_if_nil( msg, "rmr_rcv_msg returned a nil pointer" );
	}

	rmr_rts_msg( NULL, NULL );			// drive for coverage
	rmr_rts_msg( rmc, NULL );
	errors += fail_if( errno == 0, "rmr_rts_msg did not set errno when given a nil message" );

	msg->state = 0;
	msg = rmr_rts_msg( NULL, msg );			// should set state in msg
	errors += fail_if_equal( msg->state, 0, "rmr_rts_msg did not set state when given valid message but no context" );
	

	msg = rmr_rts_msg( rmc, msg );			// return the buffer to the sender
	errors += fail_if_nil( msg, "rmr_rts_msg did not return a message pointer" );
	errors += fail_if( errno != 0, "rmr_rts_msg did not reset errno" );


	snprintf( msg->xaction, 17, "%015d", 16 );		// dummy transaction id (emulation generates, this should arrive after a few calls to recv)

	msg->state = 0;
	msg = rmr_call( NULL, msg );
	errors += fail_if( msg->state == 0, "rmr_call did not set message state when given message with nil context" );

	msg->mtype = 0;
	msg = rmr_call( rmc, msg );						// this call should return a message as we can anticipate a dummy message in
	errors += fail_if_nil( msg, "rmr_call returned a nil message on call expected to succeed" );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "rmr_call did not properly set state on successful return" );
		errors += fail_not_equal( errno, 0, "rmr_call did not properly set errno (a) on successful return" );
	}

	snprintf( wbuf, 17, "%015d", 14 );				// if we call receive we should find this in the first 15 tries
	for( i = 0; i < 16; i++ ) {
		msg = rmr_rcv_msg( rmc, msg );
		if( msg ) {
			if( strcmp( wbuf, msg->xaction ) == 0 ) {		// found the queued message
				break;
			}
			fprintf( stderr, "<INFO> msg: %s\n", msg->xaction );
		} else {
			errors += fail_if_nil( msg, "receive returnd nil msg while looking for queued message" );
		}
	}

	errors += fail_if( i >= 16, "did not find expected message on queue" );

	if( ! msg ) {
		msg = rmr_alloc_msg( rmc, 2048 );				// something buggered above; get a new one
	}
	msg = rmr_call( rmc, msg );							// make a call that we never expect a response on
	errors += fail_not_nil( msg, "rmr_call returned a non-nil message on call expected not to receive a response" );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "rmr_call did not properly set state on queued message receive" );
		errors += fail_if( errno != 0, "rmr_call did not properly set errno on queued message receivesuccessful" );
	}

	msg = rmr_call( rmc, msg );						// this should "timeout" because the message xaction id won't ever appear again
	errors += fail_not_nil( msg, "rmr_call returned a non-nil message on call expected to fail" );
	errors += fail_if( errno == 0, "rmr_call did not set errno on failure" );

	rmr_free_msg( NULL ); 			// drive for coverage; nothing to check
	rmr_free_msg( msg2 );


	msg2 = rmr_torcv_msg( NULL, NULL, 10 );
	errors += fail_not_nil( msg2, "rmr_torcv_msg returned a pointer when given nil information" );
	msg2 = rmr_torcv_msg( rmc, NULL, 10 );
	errors += fail_if_nil( msg2, "rmr_torcv_msg did not return a message pointer when given a nil old msg" );

	// ---  test timeout receive; our dummy epoll function will return 1 ready on first call and 0 ready (timeout emulation) on second
	// 		however we must drain the swamp (queue) first, so run until we get a timeout error, or 20 and report error if we get to 20.
	msg = NULL;
	for( i = 0; i < 40; i++ ) {
		msg = rmr_torcv_msg( rmc, msg, 10 );
		errors += fail_if_nil( msg, "torcv_msg returned nil msg when message expected" );
		if( msg ) {
			if( msg->state == RMR_ERR_TIMEOUT ) {			// queue drained and we've seen both states from poll if we get a timeout
				break;
			}
		}
	}
	errors += fail_if( i >= 40, "torcv_msg never returned a timeout" );


	// ---- trace things that are not a part of the mbuf_api functions and thus must be tested here
	state = rmr_init_trace( NULL, 37 );						// coverage test nil context
	errors += fail_not_equal( state, 0, "attempt to initialise trace with nil context returned non-zero state (a)" );
	errors += fail_if_equal( errno, 0, "attempt to initialise trace with nil context did not set errno as expected" );

	state = rmr_init_trace( rmc, 37 );
	errors += fail_if_equal( state, 0, "attempt to set trace len in context was not successful" );
	errors += fail_not_equal( errno, 0, "attempt to set trace len in context did not clear errno" );

	msg = rmr_tralloc_msg( rmc, 1024, 17, "1904308620110417" );
	errors += fail_if_nil( msg, "attempt to allocate message with trace data returned nil message" );
	state = rmr_get_trace( msg, wbuf, 17 );
	errors += fail_not_equal( state, 17, "len of trace data (a) returned after msg allocation was not expected size (b)" );
	state = strcmp( wbuf, "1904308620110417" );
	errors += fail_not_equal( state, 0, "trace data returned after tralloc was not correct" );

	em_send_failures = 1;
	send_n_msgs( rmc, 30 );			// send 30 messages with emulation failures
	em_send_failures = 0;


	rmr_close( NULL );			// drive for coverage
	rmr_close( rmc );			// no return to check; drive for coverage


	if( ! errors ) {
		fprintf( stderr, "<INFO> all RMr API tests pass\n" );
	}
	return !!errors;
}

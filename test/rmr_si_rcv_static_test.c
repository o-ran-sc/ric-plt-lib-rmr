// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2019-2021 Nokia
	    Copyright (c) 2018-2021 AT&T Intellectual Property.

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
	Mmemonic:	rmr_si_api_static_test.c
	Abstract:	Specific tests related to the API functions in rmr_si.c.
				This should be included by a driver which invokes the 'main'
				test function here: rmr_api_test.

				This test set applies only to the outward facting API functions
				in the rmr_si.c module (mostly because the context for SI is
				different).

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
	Date:		14 April 2020	(AKD)
*/

#define DEBUG 2

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "rmr.h"
#include "rmr_agnostic.h"

static int rmr_rcv_test( ) {
	int		errors = 0;
	void*	rmc;				// route manager context
	void*	rmc2;				// second context for non-listener init
	rmr_mbuf_t*	msg;			// message buffers
	rmr_mbuf_t*	msg2;
	int		v = 0;					// some value
	char	wbuf[128];
	int		i;
	int		state;
	int		max_tries;			// prevent a sticking in any loop

	v = rmr_ready( NULL );
	errors += fail_if( v != 0, "rmr_ready returned true before initialisation "  );

	em_set_long_hostname( 1 );
	if( (rmc = rmr_init( "4560", 1024, FL_NOTHREAD )) == NULL ) {
		fail_if_nil( rmc, "rmr_init returned a nil pointer "  );
		return 1;
	}

	msg = rmr_alloc_msg( NULL,  1024 );									// should return nil pointer
	errors += fail_not_nil( msg, "rmr_alloc_msg didn't return nil when given nil context "  );
	gen_rt( rmc );				// gen dummy route table

	if( ! rmr_ready( rmc ) ) {
		fprintf( stderr, "\nPANIC!  rmr isn't showing ready after loading a rt table\n\n" );
		return errors+1;
	}

	msg = rmr_alloc_msg( rmc, 2048 );				// get a buffer with a transport header
	msg->len = 500;
	msg->mtype = 1;
	msg->state = 999;
	msg->tp_state = 999;
	errno = 999;
	snprintf( msg->payload, 500, "Stand up and cheer from OU to Oh yea! (5)" );

	msg = rmr_send_msg( rmc, msg );
	errors += fail_if_nil( msg, "send_msg_ did not return a message on send "  );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "send_msg returned bad status for send that should work "  );
		errors += fail_if( errno != 0, "send_msg set errno for send that should work "  );
		v = rmr_payload_size( msg );
		errors += fail_if( v != 2048, "send_msg did not allocate new buffer with correct size "  );
		errors += fail_if( msg->tp_state == 999, "send_msg did not set tp_state (2)" );
	}

	rmr_set_stimeout( NULL, 0 );		// not supported, but funciton exists, so drive away
	rmr_set_stimeout( rmc, 20 );
	rmr_set_stimeout( rmc, -1 );
	rmr_set_rtimeout( NULL, 0 );
	rmr_set_rtimeout( rmc, 20 );
	rmr_set_rtimeout( rmc, -1 );

	max_tries = 10;						// there shouldn't be more than 10 queued at this point
	while( (msg2 = rmr_torcv_msg( rmc, msg2, 200 )) != NULL ) {
		if( msg2->state != RMR_OK || max_tries <= 0 ) {
			break;
		}

		max_tries--;
	}

	msg2 = rmr_rcv_msg( NULL, NULL );
	errors += fail_if( msg2 != NULL, "rmr_rcv_msg returned msg when given nil context and msg "  );

	send_n_msgs( rmc, 1 );			// ensure there is a message to read
	msg2 = rmr_rcv_msg( rmc, NULL );
	errors += fail_if( msg2 == NULL, "rmr_rcv_msg returned nil msg when given nil msg "  );
	if( msg2 ) {
		if( msg2->state != RMR_ERR_EMPTY ) {
			errors += fail_not_equal( msg2->state, RMR_OK, "receive given nil message did not return msg with good state (not empty) "  );
		}
	}

	send_n_msgs( rmc, 1 );			// ensure there is a message to read (len is set to 100)
	msg = rmr_rcv_msg( rmc, msg );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "rmr_rcv_msg did not return an ok state "  );
		errors += fail_not_equal( msg->len, 100, "rmr_rcv_msg returned message with invalid len "  );
	} else {
		errors += fail_if_nil( msg, "rmr_rcv_msg returned a nil pointer "  );
	}

	rmr_rts_msg( NULL, NULL );			// drive for coverage
	errors += fail_if( errno == 0, "rmr_rts_msg did not set errno when given a nil context "  );
	rmr_rts_msg( rmc, NULL );
	errors += fail_if( errno == 0, "rmr_rts_msg did not set errno when given a nil message "  );

	msg->state = 0;
	msg = rmr_rts_msg( NULL, msg );			// should set state in msg
	if( msg ) {
		errors += fail_if_equal( msg->state, 0, "rmr_rts_msg did not set state when given valid message but no context "  );
	} else {
		errors += fail_if_nil( msg,  "rmr_rts_msg returned a nil msg when given a good one" );
	}

	msg = rmr_rts_msg( rmc, msg );			// return the buffer to the sender
	errors += fail_if_nil( msg, "rmr_rts_msg did not return a message pointer "  );
	errors += fail_not_equal( msg->state, 0, "rts_msg did not return a good state (a) when expected" );
	errors += fail_not_equal( errno, 0, "rmr_rts_msg did not reset errno (a) expected (b)"  );

	msg->state = 0;
	msg = rmr_call( NULL, msg );
	errors += fail_if( msg->state == 0, "rmr_call did not set message state when given message with nil context "  );

	em_set_rcvcount( 0 );							// reset message counter
	em_set_rcvdelay( 1 );							// force slow msg rate during mt testing
	em_disable_call_flg();							// cause reflected message to appear to be a response

	snprintf( msg->xaction, 17, "%015d", 16 );
	msg->mtype = 0;
	msg->sub_id = -1;
	msg->len = 234;
	msg = rmr_call( rmc, msg );
	errors += fail_if_nil( msg, "rmr_call returned a nil message on call expected to succeed "  );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "rmr_call did not properly set state on successful return "  );
		errors += fail_not_equal( errno, 0, "rmr_call did not properly set errno (a) on successful return "  );
	}

	if( ! msg ) {
		msg = rmr_alloc_msg( rmc, 2048 );				// something buggered above; get a new one
	}

	em_allow_call_flg();								// cause reflected message to appear to be a call
	snprintf( msg->xaction, 17, "%015d", 16 );			// ensure there is an xaction id
	msg->mtype = 0;
	msg->sub_id = -1;
	msg->len = 345;
	msg = rmr_call( rmc, msg );							// make a call that we never expect a response on (nil pointer back)
	errors += fail_not_nil( msg, "rmr_call returned message on call expected not to receive a response "  );
	errors += fail_if( errno == 0, "rmr_call did not set errno on failure "  );

	rmr_free_msg( NULL ); 			// drive for coverage; nothing to check
	rmr_free_msg( msg2 );

	msg2 = rmr_torcv_msg( NULL, NULL, 10 );
	errors += fail_not_nil( msg2, "rmr_torcv_msg returned a pointer when given nil information "  );
	msg2 = rmr_torcv_msg( rmc, NULL, 10 );
	errors += fail_if_nil( msg2, "rmr_torcv_msg did not return a message pointer when given a nil old msg "  );

	// ---  test timeout receive; our dummy epoll function will return 1 ready on first call and 0 ready (timeout emulation) on second
	// 		however we must drain the swamp (queue) first, so run until we get a timeout error, or 20 and report error if we get to 20.
	msg = NULL;
	for( i = 0; i < 40; i++ ) {
		msg = rmr_torcv_msg( rmc, msg, 10 );
		errors += fail_if_nil( msg, "torcv_msg returned nil msg when message expected "  );
		if( msg ) {
			if( msg->state == RMR_ERR_TIMEOUT || msg->state == RMR_ERR_EMPTY ) {		// queue drained and we've seen both states from poll if we get a timeout
				break;
			}
		}
	}
	errors += fail_if( i >= 40, "torcv_msg never returned a timeout "  );

	((uta_ctx_t *)rmc)->shutdown = 1;
	rmr_close( NULL );			// drive for coverage
	rmr_close( rmc );			// no return to check; drive for coverage


	// ------- receive specific is deprecated, but we still test to keep sonar happy ---------------

	rmr_rcv_specific( NULL, NULL, "12345", 0 );		// drive for error handling coverage
	rmr_rcv_specific( NULL, msg, "12345", 2 );

	strncpy( wbuf, "dummy message", sizeof( wbuf ) );
	msg = mk_populated_msg( 1024, 0, 0, -1, strlen( wbuf ) + 1 );
	strncpy( msg->payload, wbuf, msg->len );
	msg = rmr_send_msg( rmc, msg );						// give specific something to chew on

	strncpy( msg->payload, wbuf, msg->len );
	msg->mtype = 0;
	rmr_str2xact( msg, "12345" );						// should allow rcv to find it
	msg = rmr_send_msg( rmc, msg );

	msg = rmr_rcv_specific( rmc, NULL, "12345", 2 );
	if( msg ) {
		errors += fail_if( msg->state != 0, "rmr_rcv_specific failed to find the expected message" );
	} else {
		errors++;
		fprintf( stderr, "<FAIL> rcv specific expected to return a message and did not\n" );
	}

	strncpy( wbuf, "dummy message", sizeof( wbuf ) );
	msg = mk_populated_msg( 1024, 0, 0, -1, strlen( wbuf ) + 1 );
	strncpy( msg->payload, wbuf, msg->len );
	msg = rmr_send_msg( rmc, msg );						// give specific something to chew on

	fprintf( stderr, "<INFO> starting rmr_rcv_specific test for no expected message\n" );
	strncpy( msg->payload, wbuf, msg->len );
	msg->mtype = 0;
	rmr_str2xact( msg, "72345" );						// rcv should not find it
	msg = rmr_send_msg( rmc, msg );
	msg = rmr_rcv_specific( rmc, msg, "12345", 2 );
	fail_if_nil( msg, "rmr_rcv_specific expected to retun nil message did  not" );

	// --------------- phew, done ------------------------------------------------------------------------------

	if( ! errors ) {
		fprintf( stderr, "<INFO> all RMR receive tests pass\n" );
	} else {
		fprintf( stderr, "<INFO> receive tests failures noticed \n" );
	}

	return errors;
}

// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2019-2020 Nokia
	    Copyright (c) 2018-2020 AT&T Intellectual Property.

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

				Not all message/call functions can be tested here because of the
				callback nature of SI.  There is a specific rcv test static module
				for those tests.

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
#include <pthread.h>
#include <semaphore.h>

#include "rmr.h"
#include "rmr_agnostic.h"

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
	int		max_tries;			// prevent a sticking in any loop

	v = rmr_ready( NULL );
	errors += fail_if( v != 0, "rmr_ready returned true before initialisation "  );

	em_set_long_hostname( 1 );
	if( (rmc = rmr_init( "4560", 1024, FL_NOTHREAD )) == NULL ) {
		fail_if_nil( rmc, "rmr_init returned a nil pointer "  );
		return 1;
	}

	setenv( "RMR_SRC_ID", "somehost", 1 );								// context should have this as source
	if( (rmc2 = rmr_init( ":6789", 1024, FL_NOTHREAD )) == NULL ) {		// init without starting a thread
		errors += fail_if_nil( rmc, "rmr_init returned a nil pointer for non-threaded init "  );
	}

	fprintf( stderr, "<INFO> with RMR_SRC_ID env set, source name in context: (%s)\n", ((uta_ctx_t *) rmc2)->my_name );
	v = strcmp( ((uta_ctx_t *) rmc2)->my_name, "somehost:6789" );
	errors += fail_not_equal( v, 0, "source name not set from environment variable (see previous info)" );
	free_ctx( rmc2 );			// coverage

	unsetenv( "RMR_SRC_ID" );												// context should NOT have our artificial name
	if( (rmc2 = rmr_init( NULL, 1024, FL_NOTHREAD )) == NULL ) {			// drive default port selector code
		errors += fail_if_nil( rmc, "rmr_init returned a nil pointer when driving for default port "  );
	}

	fprintf( stderr, "<INFO> after unset of RMR_SRC_ID, source name in context: (%s)\n", ((uta_ctx_t *) rmc2)->my_name );
	v = strcmp( ((uta_ctx_t *) rmc2)->my_name, "somehost:6789" );
	errors += fail_if_equal( v, 0, "source name smells when removed from environment (see previous info)" );
	free_ctx( rmc2 );			// attempt to reduce leak check errors

	v = rmr_ready( rmc );		// unknown return; not checking at the moment

	msg = rmr_alloc_msg( NULL,  1024 );									// should return nil pointer
	errors += fail_not_nil( msg, "rmr_alloc_msg didn't return nil when given nil context "  );


	msg = rmr_alloc_msg( rmc, 2048 );				// allocate larger than default size given on init
	errors += fail_if_nil( msg, "rmr_alloc_msg returned nil msg pointer "  );
	if( msg ) {
		rmr_get_srcip( msg, wbuf );
		errors += fail_if_equal( 0, strlen( wbuf ), "rmr_get_srcip did not did not return string with length (b) after alloc_msg" );
		fprintf( stderr, "<INFO> ip: %s\n", wbuf );
	}


	v = rmr_payload_size( NULL );
	errors += fail_if( v >= 0, "rmr_payload_size returned valid size for nil message "  );
	errors += fail_if( errno == 0, "rmr_payload_size did not set errno on failure "  );

	v = rmr_payload_size( msg );
	if( v >= 0 ) {
		errors += fail_not_equal( v, 2048, "rmr_payload_size returned invalid size (a) instead of expected size (b) "  );
		errors += fail_if( errno != 0, "rmr_payload_size did not clear errno on success "  );
	} else {
		errors += fail_if( v < 0, "rmr_payload_size returned invalid size for good message "  );
	}



	v = rmr_get_rcvfd( NULL );
	errors += fail_if( v >= 0, "rmr_get_rcvfd returned a valid file descriptor when given nil context "  );
	v = rmr_get_rcvfd( rmc );
	errors += fail_if( v < 0, "rmr_get_rcvfd did not return a valid file descriptor "  );

	msg2 = rmr_send_msg( NULL, NULL );
	errors += fail_not_nil( msg2, "send_msg returned msg pointer when given a nil message and context "  );

	msg->state = 0;
	msg = rmr_send_msg( NULL, msg );
	errors += fail_if( msg->state == 0, "rmr_send_msg did not set msg state when msg given with nil context "  );

	// --- sends will fail with a no endpoint error until a dummy route table is set, so we test fail case first.
	msg->len = 100;
	msg->mtype = 1;
	msg->state = 999;
	msg->tp_state = 999;
	errno = 999;
	snprintf( msg->payload, 100, "Stand up and cheer from OU to Oh yea! (1)" );

	msg = rmr_send_msg( rmc, msg );
	errors += fail_if_nil( msg, "send_msg_ did not return a message on send "  );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_ERR_NOENDPT, "send_msg did not return no endpoints before rtable added "  );
		errors += fail_if( errno == 0, "send_msg did not set errno "  );
		errors += fail_if( msg->tp_state == 999, "send_msg did not set tp_state (1)" );
	}

	gen_rt( rmc );		// --- after this point there is a dummy route table so send and rts calls should be ok

	if( ! rmr_ready( rmc ) ) {
		fprintf( stderr, "\nPANIC!  rmr isn't showing ready after loading a rt table\n\n" );
		return errors;
	}

	state = init_mtcall( NULL );					// drive for coverage
	errors += fail_not_equal( state, 0, "init_mtcall did not return false (a) when given a nil context pointer" );

	rmr_free_msg( msg );
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

		fprintf( stderr, ">>>> len=%d state=%d (%s)\n", msg2->len, msg2->state, msg2->payload );
		max_tries--;
	}

	// ----- the queue load and disc cb tests should be last! -----------------------------
	for( i = 0; i < 4000; i++ ) {			// test ring drop
		if( msg == NULL ) {
			msg = rmr_alloc_msg( rmc, 2048 );				// get a buffer with a transport header
		}
		msg->len = 500;
		msg->mtype = 1;
		msg->state = 999;
		msg->tp_state = 999;
		errno = 999;
		snprintf( msg->payload, 500, "Stand up and cheer from OU to Oh yea! msg=%d", i );

		msg = rmr_send_msg( rmc, msg );
		errors += fail_if_nil( msg, "send_msg_ did not return a message on send "  );
	}

	mt_disc_cb( rmc, 0 );			// disconnect callback for coverage
	mt_disc_cb( rmc, 100 );			// with a fd that doesn't exist


	// ---- trace things that are not a part of the mbuf_api functions and thus must be tested here -------
	state = rmr_init_trace( NULL, 37 );						// coverage test nil context
	errors += fail_not_equal( state, 0, "attempt to initialise trace with nil context returned non-zero state (a) "  );
	errors += fail_if_equal( errno, 0, "attempt to initialise trace with nil context did not set errno as expected "  );

	state = rmr_init_trace( rmc, 37 );
	errors += fail_if_equal( state, 0, "attempt to set trace len in context was not successful "  );
	errors += fail_not_equal( errno, 0, "attempt to set trace len in context did not clear errno "  );

	msg = rmr_tralloc_msg( rmc, 1024, 17, "1904308620110417" );
	errors += fail_if_nil( msg, "attempt to allocate message with trace data returned nil message "  );
	state = rmr_get_trace( msg, wbuf, 17 );
	errors += fail_not_equal( state, 17, "len of trace data (a) returned after msg allocation was not expected size (b) "  );
	state = strcmp( wbuf, "1904308620110417" );
	errors += fail_not_equal( state, 0, "trace data returned after tralloc was not correct "  );

	em_send_failures = 1;
	send_n_msgs( rmc, 30 );			// send 30 messages with emulation failures
	em_send_failures = 0;


	((uta_ctx_t *)rmc)->shutdown = 1;
	rmr_close( NULL );			// drive for coverage
	rmr_close( rmc );			// no return to check; drive for coverage


	// --------------- nil pointer exception checks
	rmr_rcv_specific( NULL, NULL, "foo", 0 );
	rmr_mt_rcv( NULL, NULL, 0 );
	mt_call( NULL, NULL, 0, 1, NULL );
	rmr_mt_call( NULL, NULL, 0, 1 );
	rmr_set_low_latency( NULL );
	rmr_set_fack( NULL );


	msg2 = rmr_alloc_msg( rmc,  1024 );
	msg2 = rmr_rcv_msg( NULL, msg2 );
	if( msg2 != NULL ) {
		errors += fail_if( msg2->state == RMR_OK, "nil context check for rcv msg returned OK" );
	}
	msg2 = rmr_torcv_msg( NULL, msg2, 200 );
	if( msg2 != NULL ) {
		errors += fail_if( msg2->state == RMR_OK, "nil context check for torcv msg returned OK" );
	}

	//  ----- thread start coverage ---------------------------------------------------------------------------
	setenv( "RMR_WARNINGS", "1", 1 );	// force non-default branches during these tests
	setenv( "RMR_SRC_NAMEONLY", "1", 1 );

	rmr_init( ":6789", 1024, 0 );		// threaded mode with defined/default RM target
	setenv( "RMR_RTG_SVC", "-1", 1 );	// force into static table mode
	rmr_init( ":6789", 1024, 0 );		// threaded mode with static table

	// --------------- phew, done ------------------------------------------------------------------------------

	if( ! errors ) {
		fprintf( stderr, "<INFO> all RMr API tests pass\n" );
	}
	return !!errors;
}

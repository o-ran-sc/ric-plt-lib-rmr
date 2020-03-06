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
	Mmemonic:	wormhole_static.c
	Abstract:	Specific tests for wormhole. This module is included directly by
				the test driver at compile time.

	Author:		E. Scott Daniels
	Date:		3 April 2019
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


/*
	Note that the last tests in this function destroy the context and message so
	any tests added MUST be ahead of those tests.
*/
static int worm_test( ) {
	uta_ctx_t* ctx;			// context needed to test load static rt
	char	wbuf[1024];
	int errors = 0;			// number errors found
	int		state = 0;
	int	i;
	void* p;

	rmr_mbuf_t*	mbuf;		// mbuf to send to peer
	int		whid = -1;
	int		last_whid;

	ctx = mk_dummy_ctx();
	ctx->my_name = strdup( "tester" );
	ctx->my_ip = strdup( "30.4.19.86:1111" );

	gen_rt( ctx );

	whid = rmr_wh_open( NULL, NULL );
	errors += fail_not_equal( whid, -1, "call to wh_open with invalid values did not return bad whid" );


	whid = rmr_wh_open( ctx, NULL );
	errors += fail_not_equal( whid, -1, "call to wh_open with invalid target did not return bad whid" );

	whid = rmr_wh_open( ctx, "" );
	errors += fail_not_equal( whid, -1, "call to wh_open with empty target did not return bad whid" );

	whid = rmr_wh_open( ctx, "localhost:89219" );
	errors += fail_if_equal( whid, -1, "call to wh_open with valid target failed" );

	rmr_wh_close( ctx, 4 );					// test for coverage only; [5] should have nil pointer
	rmr_wh_close( ctx, 50 );					// test for coverage only; more than allocated reference

	last_whid = whid;
	whid = rmr_wh_open( ctx, "localhost:89219" );
	errors += fail_not_equal( whid, last_whid, "call to wh_open with duplicate target did not return the same whid" );

	for( i = 0; i < 20; i++ ) {												// test ability to extend the table
		snprintf( wbuf, sizeof( wbuf ), "localhost:864%02d", i );			// new address for each so whid is different
		whid = rmr_wh_open( ctx, wbuf );
		snprintf( wbuf, sizeof( wbuf ), "call to wh_open failed for iteration = %d", i );
		errors += fail_if_equal( whid, -1, wbuf );
		if( i ) {
			snprintf( wbuf, sizeof( wbuf ), "call to wh_open for iteration = %d returned same whid: %d", i, whid );
			errors += fail_if_equal( whid, last_whid, wbuf );
		}

		last_whid = whid;
	}

	rmr_wh_close( ctx, 3 );		// close one, then open a new one to verify that hole is found
	whid = rmr_wh_open( ctx, "localhost:21961" );
	errors += fail_not_equal( whid, 3, "attempt to fill in a hole didn't return expected" );

	p = rmr_wh_send_msg( NULL, 0, NULL );			// tests for coverage
	fail_not_nil( p, "wh_send_msg returned a pointer when given nil context and message" );

	p = rmr_wh_send_msg( ctx, 0, NULL );
	fail_not_nil( p, "wh_send_msg returned a pointer when given nil message with valid context" );

	mbuf = rmr_alloc_msg( ctx, 2048 );			// get an muf to pass round
	errors += fail_if_nil( mbuf, "unable to allocate mbuf for send tests (giving up on send tests)" );

	mbuf->state = 0;
	mbuf = rmr_wh_send_msg( NULL, 0, mbuf );
	if( mbuf ) {
		fail_if_equal( mbuf->state, 0, "wh_send_msg returned a zero state when given a nil context" );
	}
	fail_if_nil( mbuf, "wh_send_msg returned a nil message buffer when given a nil context"  );


	while( mbuf ) {
		if( !(mbuf = rmr_wh_send_msg( ctx, 50, mbuf )) ) {		// test for coverage
			errors += fail_if_nil( mbuf, "send didn't return an mbuf (skip rest of send tests)" );
			break;
		}

		mbuf = rmr_wh_send_msg( ctx, 4, mbuf );
		errors += fail_not_equal( mbuf->state, RMR_OK, "valid wormhole send failed" );
		errors += fail_not_equal( errno, 0, "errno after valid wormhole send was not 0" );

		rmr_wh_close( ctx, 4 );
		mbuf = rmr_wh_send_msg( ctx, 4, mbuf );
		rmr_wh_send_msg( ctx, 4, mbuf );
		errors += fail_not_equal( mbuf->state, RMR_ERR_WHID, "send on closed wormhole didn't set correct state in msg" );

		break;
	}

	// ---- wormhole state -----------
	state = rmr_wh_state( NULL, 0 );
	errors += fail_if_equal( state, RMR_OK, "wh_state with bad context did not return error" );

	state = rmr_wh_state( ctx, -1 );
	errors += fail_if_equal( state, RMR_OK, "wh_state with bad whid did not return error" );

	whid = rmr_wh_open( ctx, "localhost:9219" );
	if( ! fail_if_equal( whid, -1, "skipping some wh_state tests: no whid returned" ) ) {
		state = rmr_wh_state( ctx, whid );
		errors += fail_not_equal( state, RMR_OK, "wh_state on an open wh did not return OK" );

		rmr_wh_close( ctx, whid );
		state = rmr_wh_state( ctx, whid );
		errors += fail_if_equal( state, RMR_OK, "wh_state on a closed wh returned OK" );
		whid = -1;
	}

	// -----------------------------------------------------------------------
	// WARNING:  these tests destroy the context, so they MUST be last
	if( mbuf ) {			// only if we got an mbuf
		errno = 0;
		mbuf->header = NULL;
		mbuf = rmr_wh_send_msg( ctx, 5, mbuf );		// coverage test on mbuf header check
		errors += fail_not_equal( errno, EBADMSG, "wh_send didn't set errno after bad mbuf send" );
		errors += fail_not_equal( mbuf->state, RMR_ERR_NOHDR, "send with bad header did now set msg state correctly" );

		errno = 0;
		wh_nuke( ctx );
		ctx->wormholes = NULL;
		mbuf = rmr_wh_send_msg( ctx, 4, mbuf );		// coverage test on mbuf header check
		errors += fail_not_equal( errno, EINVAL, "wh_send didn't set errno after send without wormole reference" );
		errors += fail_not_equal( mbuf->state, RMR_ERR_NOWHOPEN, "wh_send didn't set msg state after send without wormole reference" );

		rmr_free_msg( mbuf );
	}

	if( ctx ) {
		free( ctx->my_name );
		free( ctx );
	}

	return !!errors;			// 1 or 0 regardless of count
}

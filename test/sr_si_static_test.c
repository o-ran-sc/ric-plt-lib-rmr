// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2020-2021 Nokia
	    Copyright (c) 2020-2021 AT&T Intellectual Property.

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
	Mmemonic:	sr_si_static_test.c
	Abstract:	Test the send/receive funcitons. These are meant to be included at compile
				time by the test driver.

	Author:		E. Scott Daniels
	Date:		21 February 2020
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

// ----------- local test support ----------------------------------------------------------
#define CLONE	1			// convenience constants for payload realloc tests
#define NO_CLONE 0
#define COPY 1
#define NO_COPY 0

/*
	Drive the send and receive functions.  We also drive as much of the route
	table collector as is possible without a real rtg process running somewhere.

	Send and receive functions are indirectly exercised from the rmr_si_static_test
	module as it tests the user facing send/receive/call/rts functions. These tests
	should exercise specific cases for the internal functions as they will not
	specifically be driven elsewhere.

	This requires the gen_rt funcition that is in the test_gen_rt module and should
	have been included by the test module(s) which include this.
*/
static int sr_si_test() {
	uta_ctx_t* ctx;			// two context structs needed to test route table collector
	uta_ctx_t* pctx;
	uta_ctx_t*	real_ctx;	// real one to force odd situations for error testing
	int errors = 0;			// number errors found
	rmr_mbuf_t*	mbuf;		// mbuf to send/receive
	rmr_mbuf_t*	mb2;		// second mbuf when needed
	int		whid = -1;
	int		last_whid;
	int		state;
	int		nn_dummy_sock;					// dummy needed to drive send
	int		size;
	int		i;
	int		flags = 0;						// flags needed to pass to rtc funcitons
	void*	p;
	char*	payload_str;

	ctx = mk_dummy_ctx();									// in the si world we need some rings in the context
	pctx = mk_dummy_ctx();

	ctx->max_plen = RMR_MAX_RCV_BYTES + sizeof( uta_mhdr_t );
	ctx->max_mlen = ctx->max_plen + sizeof( uta_mhdr_t );
	ctx->my_name = strdup( "dummy-test" );
	ctx->my_ip = strdup( "30.4.19.86:1111" );

	gen_rt( ctx );								// forces a static load with some known info since we don't start the rtc()
	gen_rt( ctx );								// force a second load to test cloning

	p = rt_ensure_ep( NULL, "foo" );				// drive for coverage
	errors += fail_not_nil( p,  "rt_ensure_ep did not return nil when given nil route table" );

	state = rmr_ready( NULL );
	errors += fail_if_true( state, "reported ready when given a nil context" );
	state = rmr_ready( ctx );
	errors += fail_if_false( state, "reported not ready when it should be" );

	/*
		rcv_msg under SI is deprecated -- do not attempt to call
	mbuf = rcv_msg( ctx, NULL );
	errors += fail_if_nil( mbuf, "no mbuf returned on receive test" );
	*/

	mbuf = rmr_alloc_msg( ctx, 2048 );
	mbuf->len = 10;
	mbuf->mtype = 1;

	mb2 = clone_msg( mbuf );
	errors += fail_if_nil( mb2, "clone message returned nil pointer" );
	errors += fail_not_equal( mbuf->flags, mb2->flags, "clone did not duplicate flags" );
	errors += fail_not_equal( mbuf->alloc_len, mb2->alloc_len, "clone did not dup alloc-len" );
	errors += fail_not_equal( mbuf->state, mb2->state, "clone did not dup state" );
	rmr_free_msg( mb2 );

	mbuf = rmr_send_msg( NULL, mbuf );
	errors += fail_if_nil( mbuf, "send with nil context but buffere didn't return buffer" );
	if( mbuf ) {
		errors += fail_not_equal( mbuf->state, RMR_ERR_BADARG, "send with buffer but nil context didn't return right state" );
	} else {
		mbuf = rmr_alloc_msg( ctx, 2048 );
	}

	//size = 2048 - em_hdr_size();		// emulated receive allocates 2K buffers -- subtract off header size
	size = 2048;
	state = rmr_payload_size( mbuf );
	errors += fail_not_equal( state, size, "payload size didn't return expected value" );	// receive should always give 4k buffer

	rmr_free_msg( mbuf );


	// -------- drive rtc in a 'static' (not pthreaded) mode to get some coverage; no 'results' to be verified -----
	/*
		It is impossible to drive the message loop of the rtc from a unit test because
		we cannot generate a message that will arrive on its private RMR context.
	*/

	setenv( ENV_RTG_RAW, "0", 1 );								// rtc is never raw under SI
	setenv( ENV_VERBOSE_FILE, ".ut_rmr_verbose", 1 );			// allow for verbose code in rtc to be driven
	i = open( ".ut_rmr_verbose", O_RDWR | O_CREAT, 0654 );
	if( i >= 0 ) {
		write( i, "2\n", 2 );
		close( i );
	}
	ctx->shutdown = 1;			// should force rtc to quit on first pass
	rtc( NULL );				// coverage test with nil pointer

	rtc_file( NULL );			// the static file only collector
	rtc_file( ctx );

	setenv( "RMR_RTREQ_FREQ", "400", 1 );	// force error checking code in rtc to resort to default
	rtc( ctx );

	setenv( "RMR_CTL_PORT", "43086", 1 );	// force defined branch in rtc
	rtc( ctx );

	setenv( "RMR_RTG_SVC", "4567", 1 );		// drive for edge case coverage to ensure no nil pointer etc
	rtc( ctx );
	setenv( "RMR_RTG_SVC", "tcp:4567", 1 );
	rtc( ctx );
	setenv( "RMR_RTG_SVC", "tcp:4567:error", 1 );
	rtc( ctx );
	setenv( "RMR_RTG_SVC", "localhost:4589", 1 );		// should force a request to be sent though no reponse back.
	rtc( ctx );

	payload_str = "newrt|start|abc-def\nmse|10|-1|host1:43086\nmse|20|-1|host1:43086\nnewrt|end|2\n";
	mbuf = mk_populated_msg( 1024, 0, 20, -2, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	rtc_parse_msg( ctx, pctx, mbuf, 5, &flags );

	mbuf = mk_populated_msg( 1024, 0, 90, -2, strlen( payload_str ) );		// drive with invalid message type for coverage
	rtc_parse_msg( ctx, pctx, mbuf, 5, &flags );


	// ------------- reallocation tests ------------------------------------------------------------
	// we use mk_populated_msg() to create a message with mid/sid/plen pushed into the transport
	// header to simulate a message having been sent and received which is what causes this data
	// to push into the wire packet.

	payload_str = "Stand Up and Cheer; OU78-82";


	mbuf = mk_populated_msg( 1024, 0, 99, 100, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	mb2 = realloc_payload( mbuf, 512, NO_COPY, NO_CLONE );
	errors += fail_if_nil( mb2, "realloc_payload (no copy) returned a nil pointer when reallocating with smaller size" );
	errors += fail_if_false( mbuf == mb2, "non-clone realloc payload (no copy) of smaller size did not return the same buffer" );

	mb2 = realloc_payload( NULL, 512, NO_COPY, NO_CLONE );
	errors += fail_not_nil( mb2, "realloc payload did not return nil pointer when passed nil mbuf" );

	mb2 = realloc_payload( mbuf, 0, NO_COPY, NO_CLONE );
	errors += fail_not_nil( mb2, "realloc payload did not return nil pointer when passed bad len" );

	fprintf( stderr, "<TEST> no copy/no clone test starts\n" );
	mb2 = realloc_payload( mbuf, 2048, NO_COPY, NO_CLONE );
	errors += fail_if_false( mbuf == mb2, "realloc payload (no copy) of larger size did not return the same msg buffer(1)" );
	errors += fail_not_equal( mb2->mtype, -1, "realloc payload (no copy) did not reset mtype(a) to expected(b) value" );
	errors += fail_not_equal( mb2->sub_id, -1, "realloc payload (no copy) did not reset sub-id(a) to expected(b) value" );
	errors += fail_if_nil( mb2, "realloc payload returned (no copy) a nil pointer when increasing payload len" );
	errors += fail_not_equal( mb2->len, 0, "realloc payload payload len(a) not expected(b):" );
	errors += fail_not_equal( rmr_payload_size( mb2), 2048, "realloc payload alloc len(a) not expected(b)" );

	fprintf( stderr, "<TEST> copy/no clone test starts\n" );
	mbuf = mk_populated_msg( 1024, 0, 99, 100, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	mb2 = realloc_payload( mbuf, 2048, COPY, NO_CLONE );
	errors += fail_if_false( mbuf == mb2, "non-clone realloc payload (copy) of larger size did not return the same msg buffer(2)" );
	errors += fail_if_nil( mb2, "realloc payload (copy) returned a nil pointer when increasing payload len)" );
	errors += fail_not_equal( mb2->mtype, 99, "realloc payload (copy) did not reset mtype(a) to expected(b) value" );
	errors += fail_not_equal( mb2->sub_id, 100, "realloc payload (copy) did not reset sub-id(a) to expected(b) value" );
	errors += fail_if_equal( mb2->len, 0, "realloc payload (copy) msg len(a) not expected(b)" );
	errors += fail_not_equal( rmr_payload_size( mb2), 2048, "realloc payload (copy) alloc len(a) not expected(b)" );
	errors += fail_not_equal( strncmp( payload_str, mb2->payload, strlen( payload_str )), 0, "realloc payload(copy) didn't copy payload" );

	fprintf( stderr, "<TEST> copy/clone test starts requested buffer smaller than original\n" );
	mbuf = mk_populated_msg( 1024, 0, 99, 100, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	mb2 = realloc_payload( mbuf, 512, COPY, CLONE );
	errors += fail_if_true( mbuf == mb2, "realloc payload (clone+copy) of larger size did not return different message buffers" );
	errors += fail_if_nil( mb2, "realloc payload (clone+copy) returned a nil pointer when increasing payload len)" );
	errors += fail_not_equal( mb2->mtype, 99, "realloc payload (clone+copy) did not reset mtype(a) to expected(b) value" );
	errors += fail_not_equal( mb2->sub_id, 100, "realloc payload (clone+copy) did not reset sub-id(a) to expected(b) value" );
	errors += fail_not_equal( mb2->len, strlen( payload_str ), "realloc payload (clone+copy) msg len(a) not expected(b)" );
	errors += fail_not_equal( rmr_payload_size( mb2), 1024, "realloc payload (clone+copy) alloc len(a) not expected(b)" );
	errors += fail_not_equal( strncmp( payload_str, mb2->payload, strlen( payload_str )), 0, "realloc payload(clone+copy) didn't copy payload" );

	// with a clone, we must verify that original message looks sane too
	errors += fail_not_equal( mbuf->mtype, 99, "realloc payload (clone+copy) validation of unchanged mbuf->mtype fails" );
	errors += fail_not_equal( mbuf->sub_id, 100, "realloc payload (clone+copy) validation of unchanged mbuf->subid fails" );
	errors += fail_not_equal( mbuf->len, strlen( payload_str ), "realloc payload (clone+copy) validation of unchanged payload len fails" );
	errors += fail_not_equal( rmr_payload_size( mbuf ), 1024, "realloc payload (clone+copy) validation of unchanged alloc length fails" );
	errors += fail_not_equal( strncmp( payload_str, mbuf->payload, strlen( payload_str )), 0, "realloc payload(clone+copy) validation of unchanged payload fails" );


	fprintf( stderr, "<TEST> copy/clone test starts requested buf is larger than original\n" );
	mbuf = mk_populated_msg( 1024, 0, 99, 100, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	mb2 = realloc_payload( mbuf, 2048, COPY, CLONE );
	errors += fail_if_true( mbuf == mb2, "realloc payload(clone+copy/lg) of larger size did not return different message buffers" );
	errors += fail_if_nil( mb2, "realloc payload (clone+copy/lg) returned a nil pointer when increasing payload len)" );
	errors += fail_not_equal( mb2->mtype, 99, "realloc payload (clone+copy/lg) did not reset mtype(a) to expected(b) value" );
	errors += fail_not_equal( mb2->sub_id, 100, "realloc payload (clone+copy/lg) did not reset sub-id(a) to expected(b) value" );
	errors += fail_not_equal( mb2->len, strlen( payload_str ), "realloc payload (clone+copy/lg) msg len(a) not expected(b)" );
	errors += fail_not_equal( rmr_payload_size( mb2), 2048, "realloc payload (clone+copy/lg) alloc len(a) not expected(b)" );
	errors += fail_not_equal( strncmp( payload_str, mb2->payload, strlen( payload_str )), 0, "realloc payload(clone+copy/lg) didn't copy payload" );

	// with a clone, we must verify that original message looks sane too
	errors += fail_not_equal( mbuf->mtype, 99, "realloc payload (clone+copy/lg) validation of unchanged mbuf->mtype fails" );
	errors += fail_not_equal( mbuf->sub_id, 100, "realloc payload (clone+copy/lg) validation of unchanged mbuf->subid fails" );
	errors += fail_not_equal( mbuf->len, strlen( payload_str ), "realloc payload (clone+copy/lg) validation of unchanged payload len fails" );
	errors += fail_not_equal( rmr_payload_size( mbuf ), 1024, "realloc payload (clone+copy/lg) validation of unchanged alloc length fails" );
	errors += fail_not_equal( strncmp( payload_str, mbuf->payload, strlen( payload_str )), 0, "realloc payload(clone+copy/lg) validation of unchanged payload fails" );

	// original message should be unharmed, and new message should have no type/sid or payload len; total alloc len should be requested enlargement
	fprintf( stderr, "<TEST> no copy/clone test starts requested buf is larger than original\n" );
	mbuf = mk_populated_msg( 1024, 0, 99, 100, strlen( payload_str ) );
	memcpy( mbuf->payload, payload_str, mbuf->len );
	mb2 = realloc_payload( mbuf, 2048, NO_COPY, CLONE );
	errors += fail_if_true( mbuf == mb2, "realloc payload (clone+nocopy) of larger size did not return different message buffers" );
	errors += fail_if_nil( mb2, "realloc payload (clone+nocopy) returned a nil pointer when increasing payload len)" );
	errors += fail_not_equal( mb2->mtype, -1, "realloc payload (clone+nocopy) did not reset mtype(a) to expected(b) value" );
	errors += fail_not_equal( mb2->sub_id, -1, "realloc payload (clone+nocopy) did not reset sub-id(a) to expected(b) value" );
	errors += fail_not_equal( mb2->len, 0, "realloc payload (clone+nocopy) msg len(a) not expected(b)" );
	errors += fail_not_equal( rmr_payload_size( mb2 ), 2048, "realloc payload (clone+nocopy) alloc len(a) not expected(b)" );
	//errors += fail_if_equal( strncmp( payload_str, mb2->payload, strlen( payload_str )), 0, "realloc payload(clone+nocopy) copied payload when not supposed to" );

	// with a clone, we must verify that original message looks sane too
	errors += fail_not_equal( mbuf->mtype, 99, "realloc payload (clone+nocopy) validation of unchanged mbuf->mtype fails" );
	errors += fail_not_equal( mbuf->sub_id, 100, "realloc payload (clone+nocopy) validation of unchanged mbuf->subid fails" );
	errors += fail_not_equal( mbuf->len, strlen( payload_str ), "realloc payload (clone+nocopy) validation of unchanged payload len fails" );
	errors += fail_not_equal( rmr_payload_size( mbuf ), 1024, "realloc payload (clone+nocopy) validation of unchanged alloc length fails" );
	errors += fail_not_equal( strncmp( payload_str, mbuf->payload, strlen( payload_str )), 0, "realloc payload (clone+nocopy) validation of unchanged payload fails" );


	// ---------------------- misc coverage tests; nothing to verify other than they don't crash -----------------------
	payload_str = strdup( "The Marching 110 will play the OU fightsong after every touchdown or field goal; it is a common sound echoing from Peden Stadium in the fall." );

	dump_n( payload_str, "A dump", strlen( payload_str ) );
	dump_40( payload_str, "another dump" );

	unlink( ".ut_rmr_verbose" );

	return errors;

}

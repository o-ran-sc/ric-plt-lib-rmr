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
	Mmemonic:	sr_nng_static_test.c
	Abstract:	Test the send/receive funcitons. These are meant to be included at compile
				time by the test driver.

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

// ----------- local test support ----------------------------------------------------------
#define CLONE 	1			// convenience constants for payload realloc tests
#define NO_CLONE 0
#define COPY 1
#define NO_COPY 0

/*
	Drive the send and receive functions.  We also drive as much of the route
	table collector as is possible without a real rtg process running somewhere.

	Send and receive functions are indirectly exercised from the rmr_nng_static_test
	module as it tests the user facing send/receive/call/rts functions. These tests
	should exercise specific cases for the internal functions as they will not
	specifically be driven elsewhere.

	This requires the gen_rt funcition that is in the test_gen_rt module and should
	have been included by the test module(s) which include this.
*/
static int sr_nng_test() {
	uta_ctx_t* ctx;				// context needed to test load static rt
	uta_ctx_t*	real_ctx;	// real one to force odd situations for error testing
	int errors = 0;			// number errors found
	rmr_mbuf_t*	mbuf;		// mbuf to send/receive
	rmr_mbuf_t*	mb2;		// second mbuf when needed
	int		whid = -1;
	int		last_whid;
	int 	state;
	nng_socket nn_dummy_sock;					// dummy needed to drive send
	int		size;
	int		i;
	void*	p;
	char*	payload_str;

	//ctx = rmr_init( "tcp:4360", 2048, 0 );				// do NOT call init -- that starts the rtc thread which isn't good here
	ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) );		// alloc the context manually
	memset( ctx, 0, sizeof( uta_ctx_t ) );

	ctx->mring = NULL;		//uta_mk_ring( 128 );
	ctx->max_plen = RMR_MAX_RCV_BYTES + sizeof( uta_mhdr_t );
	ctx->max_mlen = ctx->max_plen + sizeof( uta_mhdr_t );
	ctx->my_name = strdup( "dummy-test" );
	ctx->my_ip = strdup( "30.4.19.86:1111" );
	uta_lookup_rtg( ctx );

	gen_rt( ctx );								// forces a static load with some known info since we don't start the rtc()
	gen_rt( ctx );								// force a second load to test cloning

	p = rt_ensure_ep( NULL, "foo" );				// drive for coverage
	errors += fail_not_nil( p,  "rt_ensure_ep did not return nil when given nil route table" );

	state = rmr_ready( NULL );
	errors += fail_if_true( state, "reported ready when given a nil context" );
	state = rmr_ready( ctx );
	errors += fail_if_false( state, "reported not ready when it should be" );

	mbuf = rcv_msg( ctx, NULL );
	errors += fail_if_nil( mbuf, "no mbuf returned on receive test" );

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
		mbuf = rmr_rcv_msg( ctx, NULL );
	}

	size = 2048 - em_hdr_size();		// emulated nng receive allocates 2K buffers -- subtract off header size
	state = rmr_payload_size( mbuf );
	errors += fail_not_equal( state, size, "payload size didn't return expected value" );	// receive should always give 4k buffer

	rmr_free_msg( mbuf );


	state = xlate_nng_state( NNG_EAGAIN, 99 );
	errors += fail_if( state == 99, "xlate_nng_state returned default for nng_eagain" );
	errors += fail_if( errno != EAGAIN, "xlate_nng_state did not set errno to eagain for nng_eagain" );

	state = xlate_nng_state( NNG_ETIMEDOUT, 99 );
	errors += fail_if( state == 99, "xlate_nng_state returned default for nng_timeout" );
	errors += fail_if( errno != EAGAIN, "xlate_nng_state did not set errno to eagain for nng_timeout" );

	state = xlate_nng_state( NNG_ENOTSUP, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_notsup" );

	state = xlate_nng_state( NNG_ENOTSUP, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_notsup" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (1)" );

	state = xlate_nng_state( NNG_EINVAL, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_inval" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (2)" );

	state = xlate_nng_state( NNG_ENOMEM, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_nomem" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (3)" );

	state = xlate_nng_state( NNG_ESTATE, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_state" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (4)" );

	state = xlate_nng_state( NNG_ECLOSED, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for nng_closed" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (5)" );

	state = xlate_nng_state( 999, 99 );
	errors += fail_if( state != 99, "xlate_nng_state did not return  default for unknown error" );
	errors += fail_if( errno == 0, "xlate_nng_state did not set errno (6)" );

	// ---- drive rtc in a 'static' (not pthreaded) mode to get some coverage; no 'results' to be verified -----
	setenv( ENV_RTG_RAW, "1", 1 );								// rtc should expect raw messages (mostly coverage here)
	setenv( ENV_VERBOSE_FILE, ".ut_rmr_verbose", 1 );			// allow for verbose code in rtc to be driven
	i = open( ".ut_rmr_verbose", O_RDWR | O_CREAT, 0654 );
	if( i >= 0 ) {
		write( i, "2\n", 2 );
		close( i );
	}
	ctx->shutdown = 1;			// should force rtc to quit on first pass
	rtc( NULL );				// coverage test with nil pointer
	rtc( ctx );

	setenv( "RMR_RTG_SVC", "4567", 1 );		// drive for edge case coverage to ensure no nil pointer etc
	rtc( ctx );
	setenv( "RMR_RTG_SVC", "tcp:4567", 1 );
	rtc( ctx );
	setenv( "RMR_RTG_SVC", "tcp:4567:error", 1 );
	rtc( ctx );

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
	errors += fail_if_equal( strncmp( payload_str, mb2->payload, strlen( payload_str )), 0, "realloc payload(clone+nocopy) copied payload when not supposed to" );

	// with a clone, we must verify that original message looks sane too
	errors += fail_not_equal( mbuf->mtype, 99, "realloc payload (clone+nocopy) validation of unchanged mbuf->mtype fails" );
	errors += fail_not_equal( mbuf->sub_id, 100, "realloc payload (clone+nocopy) validation of unchanged mbuf->subid fails" );
	errors += fail_not_equal( mbuf->len, strlen( payload_str ), "realloc payload (clone+nocopy) validation of unchanged payload len fails" );
	errors += fail_not_equal( rmr_payload_size( mbuf ), 1024, "realloc payload (clone+nocopy) validation of unchanged alloc length fails" );
	errors += fail_not_equal( strncmp( payload_str, mbuf->payload, strlen( payload_str )), 0, "realloc payload (clone+nocopy) validation of unchanged payload fails" );

	unlink( ".ut_rmr_verbose" );

	return errors;
}

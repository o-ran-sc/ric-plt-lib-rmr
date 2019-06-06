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
	Mmemonic:	sr_nano_static_test.c
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

/*
	Generate a simple route table (for all but direct route table testing).
	This table contains multiple tables inasmuch as a second update set of
	records follows the initial set. 
*/
static void gen_rt( uta_ctx_t* ctx ) {
	int		fd;
	char* 	rt_stuff;		// strings for the route table

	rt_stuff =
		"\r"										// ensure we are not screwed by broken OSes that insist on using \r
		"newrt|end\n"								// end of table check before start of table found
		"# comment to drive full comment test\n"
		"\n"										// handle blank lines
		"   \n"										// handle blank lines
	    "mse|4|10|localhost:4561\n"					// entry before start message
	    "rte|4|localhost:4561\n"					// entry before start message
		"newrt|start\n"								// false start to drive detection
		"xxx|badentry to drive default case"
		"newrt|start\n"
	    "rte|0|localhost:4560,localhost:4562\n"					// these are legitimate entries for our testing
	    "rte|1|localhost:4562;localhost:4561,localhost:4569\n"
	    "rte|2|localhost:4562| 10\n"								// new subid at end
	    "mse|4|10|localhost:4561\n"									// new msg/subid specifier rec
	    "mse|4|localhost:4561\n"									// new mse entry with less than needed fields
		"   rte|   5   |localhost:4563    #garbage comment\n"		// tests white space cleanup
	    "rte|6|localhost:4562\n"
		"newrt|end\n";

	fd = open( "utesting.rt", O_WRONLY | O_CREAT, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}

	setenv( "RMR_SEED_RT", "utesting.rt", 1 );
	write( fd, rt_stuff, strlen( rt_stuff ) );
	close( fd );
	read_static_rt( ctx, 0 );
	unlink( "utesting.rt" );
}

/*
	Generates a legitimate table but with a missing newline on the last record.
*/
static void gen_mlnl_rt( uta_ctx_t* ctx ) {
	int		fd;
	char* 	rt_stuff;		// strings for the route table

	rt_stuff =
		"newrt|start\n"
	    "rte|0|localhost:4560,localhost:4562\n"					// these are legitimate entries for our testing
	    "rte|1|localhost:4562;localhost:4561,localhost:4569\n"
	    "rte|2|localhost:4562| 10\n"								// new subid at end
	    "mse|4|10|localhost:4561\n"									// new msg/subid specifier rec
	    "mse|4|localhost:4561\n"									// new mse entry with less than needed fields
		"   rte|   5   |localhost:4563    #garbage comment\n"		// tests white space cleanup
	    "rte|6|localhost:4562\n"
		"newrt|end";												// should not affect the loader

	fd = open( "utesting.rt", O_WRONLY | O_CREAT, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}

	setenv( "RMR_SEED_RT", "utesting.rt", 1 );
	write( fd, rt_stuff, strlen( rt_stuff ) );
	close( fd );
	read_static_rt( ctx, 0 );
	unlink( "utesting.rt" );
}

/*
	Generate an empty route table to test edge case.
*/
static void gen_empty_rt( uta_ctx_t* ctx ) {
	int		fd;

	fd = open( "utesting.rt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}

	setenv( "RMR_SEED_RT", "utesting.rt", 1 );
	//write( fd, "", 0 );
	close( fd );
	read_static_rt( ctx, 0 );
	unlink( "utesting.rt" );
}

/*
	Generate an single byte route table to drive an edge handling case.
*/
static void gen_sb_rt( uta_ctx_t* ctx ) {
	int		fd;

	fd = open( "utesting.rt", O_WRONLY | O_CREAT | O_TRUNC, 0600 );
	if( fd < 0 ) {
		fprintf( stderr, "<BUGGERED> unable to open file for testing route table gen\n" );
		return;
	}

	setenv( "RMR_SEED_RT", "utesting.rt", 1 );
	write( fd, " ", 1 );
	close( fd );
	read_static_rt( ctx, 0 );
	unlink( "utesting.rt" );
}


/*
	Drive the send and receive functions.  We also drive as much of the route
	table collector as is possible without a real rtg process running somewhere.

	Send and receive functions are indirectly exercised from the rmr_nano_static_test
	module as it tests the user facing send/receive/call/rts functions. These tests
	should exercise specific cases for the internal functions as they will not
	specifically be driven elsewhere.
*/
static int sr_nano_test() {
	int errors = 0;			// number errors found

	uta_ctx_t* ctx;				// context needed to test load static rt
	uta_ctx_t*	real_ctx;	// real one to force odd situations for error testing
	rmr_mbuf_t*	mbuf;		// mbuf to send/receive
	rmr_mbuf_t*	mb2;		// error capturing msg buf
	int		whid = -1;
	int		last_whid;
	int 	state;
	int nn_dummy_sock;					// dummy needed to drive send
	int		size;
	int		i;
	void*	p;

	//ctx = rmr_init( "tcp:4360", 2048, 0 );				// do NOT call init -- that starts the rtc thread which isn't good here
	ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) );		// alloc the context manually
	memset( ctx, 0, sizeof( uta_ctx_t ) );

	ctx->mring = NULL;		//uta_mk_ring( 128 );
	ctx->max_plen = RMR_MAX_RCV_BYTES + sizeof( uta_mhdr_t );
	ctx->max_mlen = ctx->max_plen + sizeof( uta_mhdr_t );
	ctx->my_name = strdup( "dummy-test" );
	ctx->my_ip = strdup( "30.4.19.86:1111" );
	uta_lookup_rtg( ctx );

	ctx->rtable = NULL;
	gen_sb_rt( ctx );							// generate and read a file with a sinle byte to test edge case
	errors += fail_not_nil( ctx->rtable, "read single byte route table produced a table" );

	ctx->rtable = NULL;
	gen_empty_rt( ctx );						// generate and read an empty rt file to test edge case
	errors += fail_not_nil( ctx->rtable, "read empty route table file produced a table" );

	ctx->rtable = NULL;
	gen_mlnl_rt( ctx );						// ensure that a file with missing last new line does not trip us up
	errors += fail_if_nil( ctx->rtable, "read  route table file with missing last newline did not produce a table" );
	
	ctx->rtable = NULL;
	gen_rt( ctx );								// forces a static load with some known info since we don't start the rtc()
	errors += fail_if_nil( ctx->rtable, "read  multi test route table file did not produce a table" );
	gen_rt( ctx );								// force a second load to test cloning
	errors += fail_if_nil( ctx->rtable, "read  multi test route table file to test clone did not produce a table" );

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
	//errors += fail_not_equal( mbuf->flags, mb2->flags, "clone did not duplicate flags" );
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

	size = 4096;
	state = rmr_payload_size( mbuf );
	errors += fail_not_equal( state, size, "payload size (b) didn't return expected value (a)" );	// receive should always give 4k buffer

	rmr_free_msg( mbuf );


	// ---- direct message read into payload (no rmr header) -------------------------
	mbuf = rcv_payload( ctx, NULL );
	errors += fail_if_nil( mbuf, "rcv_payload did not return a message buffer when given a nil messge" );
	if( mbuf ) {
		errors += fail_if_true( mbuf->len <= 0, "rcv_payload did not return a buffer with payload length set when given a nil messge" );
		errors += fail_not_equal( mbuf->state, 0, "rcv_payload did not return a buffer with good state when given a nil messge" );
	}

	mbuf = rcv_payload( ctx, NULL );
	errors += fail_if_nil( mbuf, "rcv_payload did not return a message buffer" );
	if( mbuf ) {
		errors += fail_if_true( mbuf->len <= 0, "rcv_payload did not return a buffer with payload length set" );
		errors += fail_not_equal( mbuf->state, 0, "rcv_payload did not return a buffer with good state" );
	}

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


	// --- drive the route table things which are nanomsg specific ------

	return !!errors;
}

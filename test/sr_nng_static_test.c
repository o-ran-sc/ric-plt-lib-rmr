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

#include "../src/common/include/rmr.h"
#include "../src/common/include/rmr_agnostic.h"

/*
	Generate a simple route table (for all but direct route table testing).
	This gets tricky inasmuch as we generate two in one; first a whole table 
	and then two update tables. The first is a table with a bad counter in the
	last record to test that we don't load that table and error. The second
	is a good update.
*/
static void gen_rt( uta_ctx_t* ctx ) {
	int		fd;
	char* 	rt_stuff;		// strings for the route table

	rt_stuff =
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
	write( fd, rt_stuff, strlen( rt_stuff ) );		// write in the whole table

	rt_stuff =
		"updatert|start\n"									// this is an update to the table
	    "mse|4|99|fooapp:9999,barapp:9999;logger:9999\n"	// update just one entry
		"updatert|end | 3\n";								// bad count; this update should be rejected
	write( fd, rt_stuff, strlen( rt_stuff ) );

	rt_stuff =
		"updatert|start\n"									// this is an update to the table
	    "mse|4|10|fooapp:4561,barapp:4561;logger:9999\n"	// update just one entry
		"del|2|-1\n"										// delete an entry; not there so no action
		"del|2|10\n"										// delete an entry
		"updatert|end | 3\n";								// end table; updates have a count as last field
	write( fd, rt_stuff, strlen( rt_stuff ) );
	
	close( fd );
	read_static_rt( ctx, 1 );								// force in verbose mode to see stats on tty if failure
	unlink( "utesting.rt" );
}


/*
	Drive the send and receive functions.  We also drive as much of the route
	table collector as is possible without a real rtg process running somewhere.

	Send and receive functions are indirectly exercised from the rmr_nng_static_test
	module as it tests the user facing send/receive/call/rts functions. These tests
	should exercise specific cases for the internal functions as they will not
	specifically be driven elsewhere.
*/
static int sr_nng_test() {
	uta_ctx_t* ctx;				// context needed to test load static rt
	uta_ctx_t*	real_ctx;	// real one to force odd situations for error testing
	int errors = 0;			// number errors found
	rmr_mbuf_t*	mbuf;		// mbuf to send/receive
	rmr_mbuf_t*	mb2;		// error capturing msg buf
	int		whid = -1;
	int		last_whid;
	int 	state;
	nng_socket nn_dummy_sock;					// dummy needed to drive send
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

	size = 2048 - sizeof( uta_mhdr_t );		// emulated nng receive allocates 2K payloads
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


	return !!errors;
}


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
	Mmemonic:	rt_static_test.c
	Abstract:	Test the route table funcitons. These are meant to be included at compile
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

typedef struct entry_info {
	int group;
	char* ep_name;
} ei_t;


/*
	Driven by symtab foreach element of one space.
	We count using the data as a counter.
*/
static void count_things( void* st, void* entry, char const* name, void* thing, void* vdata ) {
	int* counter;

	if( thing ) {
		if( (counter = (int *) vdata) != NULL ) {
			*counter++;
		}
	}
}

/*
	Returns the number of entries in the table for the given class.
*/
static int count_entries( route_table_t* rt, int class ) {
	int counter = 0;

	if( ! rt ) {
		return 0;
	}
	if( !rt->hash ) {
		return 0;
	}

	rmr_sym_foreach_class( rt->hash, class, count_things, &counter );	// run each and update counter

	return counter;
}

/*
	Builds a route table key.
*/
static uint64_t build_key( uint32_t mtype, uint32_t sid ) {
	uint64_t k;

	k = (uint64_t) sid << 32;
	k += mtype;

	fprintf( stderr, "<INFO> build key: %x %x --> %llx\n", (int) mtype, (int) sid, (long long) k );
	return k;
}

/*
	Create a very large set of things to clone and ensure that the colleciton
	buffers are properly resized without errors.
*/
static int lg_clone_test( ) {
	int		errors = 0;
	uta_ctx_t*	ctx;
	char*	old_env;
	route_table_t*	p;

	old_env = getenv( "RMR_SEED_RT" );
	setenv( "RMR_SEED_RT", "./large_meid.rt", 1 );

	ctx = mk_dummy_ctx();

	read_static_rt( ctx, 0 );
	p = uta_rt_clone( ctx, ctx->rtable, NULL, 1 );						// clone to force the copy from the existing table
	errors += fail_if_nil( p, "clone of large table returned nil" );
	if( p != NULL ) {
		errors += fail_not_equal( p->error, 0, "clone of large table had error" );
	}

	setenv( "RMR_SEED_RT", old_env, 1 );

	return errors;
}

/*
	This is the main route table test. It sets up a very specific table
	for testing (not via the generic setup function for other test
	situations).
*/
static int rt_test( ) {
	uta_ctx_t* ctx;			// context needed to test load static rt
	uta_ctx_t* pctx;		// "private" context for route manager communication tests
	route_table_t* rt;		// route table
	route_table_t* crt;		// cloned route table
	rtable_ent_t*	rte;	// route table entries from table
	rtable_ent_t*	rte2;
	endpoint_t*	ep;			// endpoint added
	endpoint_t*	ep2;
	int more = 0;			// more flag from round robin
	int errors = 0;			// number errors found
	int	i;
	int k;
	int	c1;					// general counters
	int c2;
	int mtype;
	int value;
	int alt_value;
	ei_t	entries[50];	// end point information
	int		gcounts[7];		// number of groups in this set
	int		ecounts[7];		// number of elements per group
	uint64_t	mtypes[7];	// mtype/sid 'key' in the modern RMR world
	char*	tok;
	char*	nxt_tok;
	int		enu = 0;
	int		state;
	char	*buf;
	char	*buf2;
	char*	seed_fname;		// seed file
	SOCKET_TYPE	nn_sock;	// differnt in each transport (nng == struct, SI/Nano == int)
	rmr_mbuf_t*	mbuf;		// message for meid route testing
	void*	p;				// generic pointer

	#ifndef NNG_UNDER_TEST
		si_ctx_t* si_ctx = NULL;
	#endif

	setenv( "ENV_VERBOSE_FILE", ".ut_rmr_verbose", 1 );			// allow for verbose code in rtc to be driven
	i = open( ".ut_rmr_verbose", O_RDWR | O_CREAT, 0644 );
	if( i >= 0 ) {
		write( i, "2\n", 2 );
		close( i );
	}


	/*
		The hacky code below calls the necessary rmr functions to create a route table
		as though the following were read and parsed by the rmr functions. (This tests
		the individual funcitons and avoids writing another parser, so it's not pretty.)

		mse | 0 | 0 | yahoo.com:4561,localhost:4562
		mse | 1 | 0 | localhost:4560,localhost:4568,localhost:4569; localhost:4561,localhost:4562
		mse | 2 | 0 | localhost:4563,localhost:4564
		mse | 3 | 0 | localhost:4565
		mse | 3 | 11 | locahost:5511
		mse | 3 | -1 | localhost:5500
	*/
	gcounts[0] = 1;							// first entry has 1 group with 2 endpoints; message type 0, sid 0
	ecounts[0] = 2;
	mtypes[0] = build_key( 0, 0 );			// mtype is now a key of mtype/sid
	entries[enu].group = 0; entries[enu].ep_name = "yahoo.com:4561"; enu++;		// use a dns resolvable name to test that
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4562"; enu++;		// rest can default to some dummy ip

	gcounts[1] = 2;				// 2 groups
	ecounts[1] = 3;				// first has 3 endpoints
	mtypes[1] = build_key( 1, 0 );
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4560"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4568"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4569"; enu++;

	gcounts[2] = 0;					// 0 means use same rte, this is the next group for the entry
	ecounts[2] = 2;					// 2 endpoints
	mtypes[2] = 999;				// ignored when appending to previous entry
	entries[enu].group = 1; entries[enu].ep_name = "localhost:4561"; enu++;
	entries[enu].group = 1; entries[enu].ep_name = "localhost:4562"; enu++;

	gcounts[3] = 1;					// next entry has 1 group
	ecounts[3] = 2;					// with 2 enpoints
	mtypes[3] = build_key( 2, 0 );
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4563"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4564"; enu++;

	gcounts[4] = 1;					// three entries for mt==3 with different sids
	ecounts[4] = 1;
	mtypes[4] = build_key( 3, 0 );
	entries[enu].group = 0; entries[enu].ep_name = "localhost:5500"; enu++;

	gcounts[5] = 1;
	ecounts[5] = 1;
	mtypes[5] = build_key( 3, 11 );
	entries[enu].group = 0; entries[enu].ep_name = "localhost:5511"; enu++;

	gcounts[6] = 1;
	ecounts[6] = 1;
	mtypes[6] = build_key( 3, -1 );
	entries[enu].group = 0; entries[enu].ep_name = "localhost:5512"; enu++;


	rt = uta_rt_init( NULL );
	errors += fail_if_false( rt == NULL, "rt_init given a nil context didn't return nil" );

	ctx = mk_dummy_ctx();		// make a dummy with rtgate mutex
	rt = uta_rt_init( ctx );										// get us a route table
	if( (errors += fail_if_nil( rt, "pointer to route table" )) ) {
		fprintf( stderr, "<FAIL> abort: cannot continue without a route table\n" );
		exit( 1 );
	}

	enu = 0;
	rte = NULL;
	for( i = 0; i < sizeof( gcounts )/sizeof( int ); i++ ) {				// add entries defined above
		if( gcounts[i] ) {
			rte = uta_add_rte( rt, mtypes[i], gcounts[i] );					// get/create entry for message type
			if( (errors += fail_if_nil( rte, "route table entry" )) ) {
				fprintf( stderr, "<FAIL> abort: cannot continue without a route table entry\n" );
				exit( 1 );
			}
		} else {
			if( rte == NULL ) {
				fprintf( stderr, "<SNAFU> internal testing error -- rte was nil for gcount == 0\n" );
				exit( 1 );
			}
		}

		for( k = 0; k < ecounts[i]; k++ ) {
			ep = uta_add_ep( rt, rte, entries[enu].ep_name, entries[enu].group );
			errors += fail_if_nil( ep, "endpoint" );
			enu++;
		}
	}

	// ----- end hacking together a route table ---------------------------------------------------


	crt = uta_rt_clone( ctx, rt, NULL, 0 );								// create a new rt and clone only the me entries
	errors += fail_if_nil( crt, "cloned route table" );
	if( crt ) {
		c1 = count_entries( rt, 1 );
		c2 = count_entries( crt, 1 );
		errors += fail_not_equal( c1, c2, "cloned (endpoints) table entries space 1 count (b) did not match original table count (a)" );

		c2 = count_entries( crt, 0 );
		errors += fail_not_equal( c2, 0, "cloned (endpoints) table entries space 0 count (a) was not zero as expected" );

		errors += fail_if_false( crt->ephash == rt->ephash, "ephash pointer in cloned table is not right" );
		uta_rt_drop( crt );
	}


	crt = uta_rt_clone( ctx, rt, NULL, 1 );							// clone all entries (MT and ME)
	errors += fail_if_nil( crt, "cloned (all) route table" );

	if( crt ) {
		c1 = count_entries( rt, 0 );
		c2 = count_entries( crt, 0 );
		errors += fail_not_equal( c1, c2, "cloned (all) table entries space 0 count (b) did not match original table count (a)" );

		c1 = count_entries( rt, 1 );
		c2 = count_entries( crt, 1 );
		errors += fail_not_equal( c1, c2, "cloned (all) table entries space 1 count (b) did not match original table count (a)" );

		errors += fail_if_false( crt->ephash == rt->ephash, "ephash pointer in cloned table (all) is not right" );
		uta_rt_drop( crt );
	}

	#ifdef NNG_UNDER_TEST
		if( (ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) )) != NULL ) {		// get a "context" needed for si testing
			memset( ctx, 0, sizeof( *ctx ) );
			ctx->rtable = rt;
		} else {
			fprintf( stderr, "<FAIL> cannot acllocate a context, cannot continue rtable tests\n" );
			return errors;
		}
	#else
		ctx = mk_dummy_ctx();
	#endif

	ctx->rtable = rt;

	ep = uta_get_ep( rt, "localhost:4561" );
	errors += fail_if_nil( ep, "end point (fetch by name)" );
	ep = uta_get_ep( rt, "bad_name:4560" );
	errors += fail_not_nil( ep, "end point (fetch by name with bad name)" );

	ep = NULL;
	#ifdef NNG_UNDER_TEST
		state = uta_epsock_byname( rt, "localhost:4561", &nn_sock, &ep );		// this should be found
	#else
		state = uta_epsock_byname( ctx, "localhost:4561", &nn_sock, &ep );		// this should be found
	#endif
	errors += fail_if_equal( state, 0, "socket (by name)" );
	errors += fail_if_nil( ep, "epsock_byname did not populate endpoint pointer when expected to" );
	//alt_value = uta_epsock_byname( rt, "localhost:4562" );			// we might do a memcmp on the two structs, but for now nothing
	//errors += fail_if_equal( value, alt_value, "app1/app2 sockets" );

	#if  NNG_UNDER_TEST
		state = uta_epsock_byname( NULL, "localhost:4561", &nn_sock, &ep );		// test coverage on nil checks
	#else
		state = uta_epsock_byname( NULL, "localhost:4561", &nn_sock, &ep );
		errors += fail_not_equal( state, 0, "socket (by name) nil context check returned true" );

		p = ctx->si_ctx;
		ctx->si_ctx = NULL;		// set to drive second test
		state = uta_epsock_byname( ctx, "localhost:4561", &nn_sock, &ep );
		ctx->si_ctx = p;
	#endif
	errors += fail_not_equal( state, 0, "socket (by name) nil check returned true" );

	if( ep ) {					// if previous test fails, cant run this
		ep->open = 1;
		#if  NNG_UNDER_TEST
			state = uta_epsock_byname( rt, "localhost:4561", &nn_sock, NULL );		// test coverage on nil checks
		#else
			state = uta_epsock_byname( ctx, "localhost:4561", &nn_sock, NULL );
		#endif
		errors += fail_if_equal( state, 0, "socket (by name) open ep check returned false" );
	}


	// --- test that the get_rte function finds expected keys, and retries to find 'bad' sid attempts for valid mtypes with no sid
	rte = uta_get_rte( rt, 0, 1, TRUE );			// s=0 m=1 is defined, so this should return a pointer
	errors += fail_if_nil( rte, "get_rte did not return a pointer when s=0 m=1 true given" );

	rte = uta_get_rte( rt, 0, 1, FALSE );			// the retry shouldn't apply, but ensure it does the righ thing
	errors += fail_if_nil( rte, "get_rte did not return a pointer when s=0 m=1 false given" );

	rte = uta_get_rte( rt, 1000, 1, FALSE );		// s=1000 does not exist for any msg type; should return nil as not allowed to drop sid
	errors += fail_not_nil( rte, "get_rte returned a pointer when s=1000 m=1 false given" );

	rte = uta_get_rte( rt, 1000, 1, TRUE );			// this should also fail as there is no mt==1 sid==-1 defined
	errors += fail_not_nil( rte, "get_rte returned a pointer when s=1000 m=1 true given" );

	rte = uta_get_rte( rt, 0, 3, TRUE );			// mtype sid combo does exist; true/false should not matter
	errors += fail_if_nil( rte, "get_rte did not return a pointer when s=0 m=3 true given" );

	rte2 = uta_get_rte( rt, 11, 3, TRUE );			// same mtype as before, different (valid) group, rte should be different than before
	errors += fail_if_nil( rte2, "get_rte did not return a pointer when s=11 m=3 true given" );
	errors += fail_if_true( rte == rte2, "get_rte for mtype==3 and different sids (0 and 11) returned the same rte pointer" );

	rte2 = uta_get_rte( rt, 0, 3, FALSE );			// since the mtype/sid combo exists, setting false should return the same as before
	errors += fail_if_nil( rte2, "get_rte did not return a pointer when s=0 m=3 false given" );
	errors += fail_if_false( rte == rte2, "get_rte did not return same pointer when mtype/sid combo given with different true/false" );

	rte = uta_get_rte( rt, 12, 3, FALSE );			// this combo does not exist and should fail when alt-key is not allowed (false)
	errors += fail_not_nil( rte, "get_rte returned a pointer for s=12, m=3, false" );

	rte = uta_get_rte( rt, 12, 3, TRUE );			// this should return the entry for the 3/-1 combination
	errors += fail_if_nil( rte, "get_rte did not return a pointer for s=12, m=3, true" );


	alt_value = -1;
	rte = uta_get_rte( rt, 0, 1, FALSE );			// get an rte for the next loop
	if( rte ) {
		for( i = 0; i < 10; i++ ) {									// round robin return value should be different each time
			#ifdef NNG_UNDER_TEST
				value = uta_epsock_rr( rte, 0, &more, &nn_sock, &ep );		// msg type 1, group 1
			#else
				value = uta_epsock_rr( ctx, rte, 0, &more, &nn_sock, &ep );
			#endif

			errors += fail_if_equal( value, alt_value, "round robiin sockets with multiple end points" );
			errors += fail_if_false( more, "more for mtype==1" );
			alt_value = value;
		}
	}

	more = -1;
	rte = uta_get_rte( rt, 0, 3, FALSE );				// get an rte for the next loop
	if( rte ) {
		for( i = 0; i < 10; i++ ) {								// this mtype has only one endpoint, so rr should be same each time
			#ifdef NNG_UNDER_TEST
				value = uta_epsock_rr( rte, 0, NULL, &nn_sock, &ep );		// also test ability to deal properly with nil more pointer
			#else
				value = uta_epsock_rr( ctx, rte, 0, NULL, &nn_sock, &ep );
			#endif

			if( i ) {
				errors += fail_not_equal( value, alt_value, "round robin sockets with one endpoint" );
				errors += fail_not_equal( more, -1, "more value changed in single group instance" );
			}
			alt_value = value;
		}
	}

	rte = uta_get_rte( rt, 11, 3, TRUE );
	#ifdef NNG_UNDER_TEST
		state = uta_epsock_rr( rte, 22, NULL, NULL, &ep );
	#else
		state = uta_epsock_rr( ctx, rte, 22, NULL, NULL, &ep );
	#endif
	errors += fail_if_true( state, "uta_epsock_rr returned bad (non-zero) state when given nil socket pointer" );


	uta_rt_clone( ctx, NULL, NULL, 0 );								// verify null parms don't crash things
	uta_rt_drop( NULL );
	#ifdef NNG_UNDER_TEST
		uta_epsock_rr( NULL, 0,  &more, &nn_sock, &ep );			// drive null case for coverage
		state = uta_epsock_rr( rte, 22, NULL, NULL, &ep );
	#else
		state = uta_epsock_rr( NULL, NULL, 0,  &more, &nn_sock, &ep );			// drive null case for coverage
		errors += fail_not_equal( state, 0, "uta_epsock_rr did not return false when given nil ctx" );

		state = uta_epsock_rr( ctx, NULL, 0,  &more, &nn_sock, &ep );
		errors += fail_not_equal( state, 0, "uta_epsock_rr did not return false when given nil rte" );

		state = uta_epsock_rr( ctx, rte, 10000,  &more, &nn_sock, &ep );
		errors += fail_not_equal( state, 0, "uta_epsock_rr did not return false when given invalid group number" );
	#endif
	uta_add_rte( NULL, 99, 1 );
	uta_get_rte( NULL, 0, 1000, TRUE );

	fprintf( stderr, "[INFO] test: adding end points with nil data; warnings expected\n" );
	uta_add_ep( NULL, NULL, "foo", 1 );
	uta_add_ep( rt, NULL, "foo", 1 );

	buf = uta_fib( ".gitignore" );
	errors += fail_if_nil( buf, "buffer from read file into buffer" );
	if( buf ) {
		free( buf );
	}
	buf = uta_fib( "no-file" );
	errors += fail_if_nil( buf, "buffer from read file into buffer (no file)" );
	if( buf ) {
		free( buf );
	}

	fprintf( stderr, "<INFO> test is overtly dropping rt table at %p\n", rt );
	ctx->rtable = NULL;
	uta_rt_drop( rt );
	rt = NULL;


	// --- force the load of a RT which has some edge case forcing issues
	if( ctx ) {
		char*	rt_stuff =
				"newrt | start | dummy-seed\n"
				"mse | 1  | -1 | localhost:84306\n"
				"mse | 10  | -1 | localhost:84306\n"
				"mse | 10  | 1 | localhost:84306\n"
				"# should cause failure because there aren't 10 entries above\n"
				"newrt | end | 10\n"

				"# this table has no end\n"
				"newrt | start | dummy-seed\n"
				"mse | 1  | -1 | localhost:84306\n"
				"mse | 10  | -1 | localhost:84306\n"
				"mse | 10  | 1 | localhost:84306\n"

				"# this table should be ok\n"
				"newrt | start | dummy-seed\n"
				"mse | 1  | -1 | localhost:84306\n"
				"mse | 10  | -1 | localhost:84306\n"
				"mse | 10  | 1 | localhost:84306\n"
				"newrt | end | 3\n"

				"# for an update to the existing table\n"
				"# not in progress; drive that exception check\n"
				"update | end | 23\n"

				"update | start | dummy-seed\n"
				"mse | 3 | 2 | localhost:2222\n"
				"# short record to drive test\n"
				"del\n"
				"# no table end for exception handling\n"

				"update | start | dummy-seed\n"
				"mse | 2 | 2 | localhost:2222\n"
				"del | 10 | 1\n"
				"update | end | 2\n";

		fprintf( stderr, "<INFO> loading RT from edge case static table\n" );
		fprintf( stderr, "<INFO> %s\n", rt_stuff );
		gen_custom_rt( ctx, rt_stuff );
		fprintf( stderr, "<INFO> edge case load completed\n" );
		errors += fail_if_nil( ctx->rtable, "edge case route table didn't generate a pointer into the context" );

		unsetenv( "RMR_SEED_RT" );			// remove for next read try
		if( ctx && ctx->seed_rt_fname != NULL ) {
			free( ctx->seed_rt_fname );
			ctx->seed_rt_fname = NULL;
		}
		read_static_rt( ctx, 0 );			// drive for not there coverage
	}


	buf = uta_fib( "no-suhch-file" );			// drive some error checking for coverage
	if( buf ) {
		free( buf );
	}


	ep = (endpoint_t *) malloc( sizeof( *ep ) );
	memset( ep, 0, sizeof( ep ) );
	pthread_mutex_init( &ep->gate, NULL );
	ep->name = strdup( "worm" );
	ep->addr = NULL;
	ep->notify = 1;
	#ifdef NNG_UNDER_TEST
		state = uta_link2( ep );
	#else
		state = uta_link2( ctx, ep );
	#endif
	errors += fail_if_true( state, "link2 did not return false when given a bad target name" );

	#ifdef NNG_UNDER_TEST
		state = uta_link2( NULL );
	#else
		state = uta_link2( ctx, NULL );
		errors += fail_if_true( state, "link2 did not return false when given nil ep pointer" );

		state = uta_link2( NULL, ep );
	#endif
	errors += fail_if_true( state, "link2 did not return false when given nil pointer" );

	ep->name = strdup( "localhost:5512" );
	ep->open = 1;
	#ifdef NNG_UNDER_TEST
		state = uta_link2( ep );			// drive for coverage
	#else
		state = uta_link2( ctx, ep );
	#endif
	errors += fail_if_false( state, "link2 did returned false when given open ep" );

	#ifndef NNG_UNDER_TEST
		ep->open = 0;							// context is used only if ep not open, so to check this test close the ep
		ep->notify = 1;
		state = rt_link2_ep( NULL, ep );
		errors += fail_if_true( state, "rt_link2_ep returned true when given bad context" );

		state = rt_link2_ep( ctx, NULL );
		errors += fail_if_true( state, "rt_link2_ep returned true when given bad ep" );

		ep->open = 1;
		state = rt_link2_ep( ctx, ep );
		errors += fail_if_false( state, "rt_link2_ep returned false when given an open ep" );

		ep->open = 0;
		state = rt_link2_ep( ctx, ep );
		errors += fail_if_false( state, "rt_link2_ep returned false when given a closed ep" );

		ep->open = 1;
		uta_ep_failed( ep );
		errors += fail_if_true( ep->open, "uta_ep_failed didn't set open flag to false" );

	#endif


	// ----------------- test the meid support for looking up an endpoint based on the meid in the message -----

	ctx->rtable = NULL;
	ctx->my_name = strdup( "my_host_name" );		// set up to load a rtable
	ctx->my_ip = strdup( "192.168.1.30" );
	if( ctx && ctx->seed_rt_fname != NULL ) {
		free( ctx->seed_rt_fname );
		ctx->seed_rt_fname = NULL;
	}
	gen_rt( ctx );									// generate a route table with meid entries and hang off ctx

	mbuf = rmr_alloc_msg( ctx, 2048 );               //  buffer to play with
	mbuf->len = 100;
	rmr_str2meid( mbuf, "meid1" );					// id that we know is in the map

	#ifdef NNG_UNDER_TEST
		ep = NULL;										// force to nil so we see it go non-nil
		state = epsock_meid( ctx->rtable, mbuf, &nn_sock, &ep );
		errors += fail_if_nil( ep, "ep was nil when looking up ep with known meid in message" );
		errors += fail_not_equal( state, 1, "state was not true when looking up ep with known meid in message" );

		rmr_str2meid( mbuf, "XXXmeid1" );				// id that we know is NOT in the map
		state = epsock_meid( ctx->rtable, mbuf, &nn_sock, &ep );
		// it is NOT a valid check to test ep for nil -- epsock_mied doesn't guarentee ep is set/cleared when state is false
		errors += fail_not_equal( state, 0, "state was not false when looking up ep with unknown meid in message" );
	#else
		ep = NULL;										// force to nil so we see it go non-nil
		state = epsock_meid( ctx, ctx->rtable,  mbuf, &nn_sock, &ep );
		errors += fail_if_nil( ep, "ep was nil when looking up ep with known meid in message" );
		errors += fail_not_equal( state, 1, "state was not true when looking up ep with known meid in message" );

		state = epsock_meid( ctx, ctx->rtable,  mbuf, &nn_sock, &ep );		// a second call to drive open == true check for coverage
		errors += fail_if_nil( ep, "ep was nil when looking up ep with known meid in message; on open ep" );
		errors += fail_not_equal( state, 1, "state was not true when looking up ep with known meid in message; on open ep" );

		rmr_str2meid( mbuf, "XXXmeid1" );				// id that we know is NOT in the map
		state = epsock_meid( ctx, ctx->rtable, mbuf, &nn_sock, &ep );
		// it is NOT a valid check to test ep for nil -- epsock_mied doesn't guarentee ep is set/cleared when state is false
		errors += fail_not_equal( state, 0, "state was not false when looking up ep with unknown meid in message" );

		state = epsock_meid( NULL, ctx->rtable,  mbuf, &nn_sock, &ep );
		errors += fail_not_equal( state, 0, "epsock_meid returned true when given nil context" );

		state = epsock_meid( ctx, ctx->rtable,  mbuf, NULL, &ep );
		errors += fail_not_equal( state, 0, "epsock_meid returned true when given nil socket pointer" );
	#endif

	// ------------  debugging and such; coverage only calls ----------------------------------------------------------
		ep_stats( ctx->rtable, NULL, "name", NULL, NULL );			// ensure no crash when given nil pointer
		rt_epcounts( ctx->rtable, "testing" );
		rt_epcounts( NULL, "testing" );

		buf = ensure_nlterm( NULL );
		errors += fail_if_nil( buf, "ensure nlterm returned null pointer when given nil ptr" );
		if( buf ) {
			errors += fail_not_equal( strlen( buf ), 1, "ensure nlterm returned incorrect length string when given nil pointer" );
			free( buf );
		}

		buf = ensure_nlterm( strdup( "x" ) );			// should return "x\n"
		errors += fail_if_nil( buf, "ensure nlterm returned null pointer when given single char string" );
		if( buf ) {
			errors += fail_not_equal( strlen( buf ), 2, "ensure nlterm returned incorrect length string when given single char string" );
			free( buf );
		}

		buf = strdup( "x\n" );
		buf2 = ensure_nlterm( buf );					// buffer returned should be the same
		if( fail_not_pequal( buf, buf2, "ensure nlterm returned new buffer for one char string with newline" ) ) {
			errors++;
			free( buf2 );
		}
		free( buf );

		buf = strdup( "Missing our trips to Gloria's for papossas.\n" );
		buf2 = ensure_nlterm( buf );											// buffer returned should be the same
		if( fail_not_pequal( buf, buf2, "ensure nlterm returned new buffer for string with newline" ) ) {
			errors++;
			free( buf2 );
		}
		free( buf );

		buf = ensure_nlterm( strdup( "Stand up and cheer!" ) );					// force addition of newline
		if( buf ) {
			errors += fail_not_equal( strcmp( buf, "Stand up and cheer!\n" ), 0, "ensure nlterm didn't add newline" );
			free( buf );
			buf = NULL;
		}


	// ------------- route manager request/response funcitons -------------------------------------------------------
		{
			rmr_mbuf_t*	smsg;

			smsg = rmr_alloc_msg( ctx, 1024 );
			send_rt_ack( ctx, smsg, "123456", 0, "no reason" );

			pctx = mk_dummy_ctx();
			ctx->rtg_whid = -1;
			state = send_update_req( pctx, ctx );
			errors += fail_not_equal( state, 0, "send_update_req did not return 0" );

			ctx->rtg_whid = rmr_wh_open( ctx, "localhost:19289" );
			state = send_update_req( pctx, ctx );
			errors += fail_if_equal( state, 0, "send_update_req to an open whid did not return 0" );
		}


	// ------------- si only; fd to ep conversion functions ---------------------------------------------------------
	#ifndef NNG_UNDER_TEST
		ep2 = (endpoint_t *) malloc( sizeof( *ep ) );

		fd2ep_init( ctx );
		fd2ep_add( ctx, 10, ep2 );

		ep = fd2ep_get( ctx, 10 );
		errors += fail_if_nil( ep, "fd2ep did not return pointer for known mapping" );
		errors += fail_if_false( ep == ep2,  "fd2ep did not return same pointer that was added" );

		ep = fd2ep_get( ctx, 20 );
		errors += fail_not_nil( ep, "fd2ep did returned a pointer for unknown mapping" );

		ep = fd2ep_del( ctx, 10 );
		errors += fail_if_nil( ep, "fd2ep delete did not return pointer for known mapping" );
		errors += fail_if_false( ep == ep2,  "fd2ep delete did not return same pointer that was added" );

		ep = fd2ep_del( ctx, 20 );
		errors += fail_not_nil( ep, "fd2ep delete returned a pointer for unknown mapping" );
	#endif

	// ---------------- misc coverage tests --------------------------------------------------------------------------
	collect_things( NULL, NULL, NULL, NULL, NULL );				// these both return null, these test NP checks
	collect_things( NULL, NULL, NULL, NULL, (void *) 1234 );		// the last is an invalid pointer, but check needed to force check on previous param
	del_rte( NULL, NULL, NULL, NULL, NULL );

	ctx = mk_dummy_ctx();
	roll_tables( ctx );				// drive nil rt check



	// ------ specific edge case tests -------------------------------------------------------------------------------
	errors += lg_clone_test( );

	unlink( ".ut_rmr_verbose" );

	return errors;			// 1 or 0 regardless of count
}

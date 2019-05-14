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

#include "../src/common/include/rmr.h"
#include "../src/common/include/rmr_agnostic.h"

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
	This is the main route table test. It sets up a very specific table
	for testing (not via the generic setup function for other test
	situations).
*/
static int rt_test( ) {
	uta_ctx_t* ctx;			// context needed to test load static rt
	route_table_t* rt;		// route table
	route_table_t* crt;		// cloned route table
	rtable_ent_t*	rte;	// entry in the table
	endpoint_t*	ep;			// endpoint added
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
	int		gcounts[5];		// number of groups in this set
	int		ecounts[5];		// number of elements per group
	int		mtypes[5];		// msg type for each group set
	char*	tok;
	char*	nxt_tok;
	int		enu = 0;
	int		state;
	char	*buf;
	char*	seed_fname;		// seed file
	nng_socket nn_sock;		// this is a struct in nng, so difficult to validate

	setenv( "ENV_VERBOSE_FILE", ".ut_rmr_verbose", 1 );			// allow for verbose code in rtc to be driven
	i = open( ".ut_rmr_verbose", O_RDWR | O_CREAT, 0644 );
	if( i >= 0 ) {
		write( 1, "2\n", 2 );
		close( i );
	}

	gcounts[0] = 1;			// build entry info -- this is hackish, but saves writing another parser
	ecounts[0] = 2;
	mtypes[0] = 0;
	entries[enu].group = 0; entries[enu].ep_name = "yahoo.com:4561"; enu++;		// use a dns resolvable name to test that
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4562"; enu++;	// rest can default to some dummy ip

	gcounts[1] = 2;
	ecounts[1] = 3;
	mtypes[1] = 1;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4561"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4568"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4569"; enu++;

	gcounts[2] = 0;		// 0 groups means use same rte, this is the next gropup
	ecounts[2] = 2;
	mtypes[2] = 1;
	entries[enu].group = 1; entries[enu].ep_name = "localhost:4561"; enu++;
	entries[enu].group = 1; entries[enu].ep_name = "localhost:4562"; enu++;

	gcounts[3] = 1;		// 0 groups means use same rte, this is the next gropup
	ecounts[3] = 2;
	mtypes[3] = 2;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4563"; enu++;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4564"; enu++;

	gcounts[4] = 1;		// 0 groups means use same rte, this is the next gropup
	ecounts[4] = 1;
	mtypes[4] = 3;
	entries[enu].group = 0; entries[enu].ep_name = "localhost:4565"; enu++;



	rt = uta_rt_init( );										// get us a route table
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

	crt = uta_rt_clone( rt );								// clone only the endpoint entries
	errors += fail_if_nil( crt, "cloned route table" );
	if( crt ) {
		c1 = count_entries( rt, 1 );
		c2 = count_entries( crt, 1 );
		errors += fail_not_equal( c1, c2, "cloned (endpoints) table entries space 1 count (b) did not match original table count (a)" );
	
		c2 = count_entries( crt, 0 );
		errors += fail_not_equal( c2, 0, "cloned (endpoints) table entries space 0 count (a) was not zero as expected" );
		uta_rt_drop( crt );
	}


	crt = uta_rt_clone_all( rt );							// clone all entries
	errors += fail_if_nil( crt, "cloned all route table" );

	if( crt ) {
		c1 = count_entries( rt, 0 );
		c2 = count_entries( crt, 0 );
		errors += fail_not_equal( c1, c2, "cloned (all) table entries space 0 count (b) did not match original table count (a)" );
	
		c1 = count_entries( rt, 1 );
		c2 = count_entries( crt, 1 );
		errors += fail_not_equal( c1, c2, "cloned (all) table entries space 1 count (b) did not match original table count (a)" );
		uta_rt_drop( crt );
	}
	

	ep = uta_get_ep( rt, "localhost:4561" );
	errors += fail_if_nil( ep, "end point (fetch by name)" );
	ep = uta_get_ep( rt, "bad_name:4560" );
	errors += fail_not_nil( ep, "end point (fetch by name with bad name)" );

	state = uta_epsock_byname( rt, "localhost:4561", &nn_sock );		// this should be found
	errors += fail_if_equal( state, 0, "socket (by name)" );
	//alt_value = uta_epsock_byname( rt, "localhost:4562" );			// we might do a memcmp on the two structs, but for now nothing
	//errors += fail_if_equal( value, alt_value, "app1/app2 sockets" );

	alt_value = -1;
	for( i = 0; i < 10; i++ ) {										// round robin return value should be different each time
		value = uta_epsock_rr( rt, 1, 0, &more, &nn_sock );			// msg type 1, group 1
		errors += fail_if_equal( value, alt_value, "round robiin sockets with multiple end points" );
		errors += fail_if_false( more, "more for mtype==1" );
		alt_value = value;
	}

	more = -1;
	for( i = 0; i < 10; i++ ) {							// this mtype has only one endpoint, so rr should be same each time
		value = uta_epsock_rr( rt, 3, 0, NULL, &nn_sock );		// also test ability to deal properly with nil more pointer
		if( i ) {
			errors += fail_not_equal( value, alt_value, "round robin sockets with one endpoint" );
			errors += fail_not_equal( more, -1, "more value changed in single group instance" );
		}
		alt_value = value;
	}

	value = uta_epsock_rr( rt, 9, 0, &more, &nn_sock );			// non-existant message type; should return false (0)
	errors += fail_not_equal( value, 0, "socket for bad mtype was valid" );

	uta_rt_clone( NULL );								// verify null parms don't crash things
	uta_rt_drop( NULL );
	uta_epsock_rr( NULL, 1, 0, &more, &nn_sock );		// drive null case for coverage
	uta_add_rte( NULL, 99, 1 );

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

	uta_rt_drop( rt );

	if( (ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) )) != NULL ) {
		memset( ctx, 0, sizeof( *ctx ) );

		if( (seed_fname = getenv( "RMR_SEED_RT" )) != NULL ) {
			if( ! (fail_if_nil( rt, "pointer to rt for load test" )) ) {
				errors++;
				read_static_rt( ctx, 0 );
				unsetenv( "RMR_SEED_RT" );			// unset to test the does not exist condition
				read_static_rt( ctx, 0 );
			} else {
				fprintf( stderr, "<FAIL> cannot gen rt for load test\n" );
			}
		} else {
			read_static_rt( ctx, 0 );		// not defined, just drive for that one case
		}
	}

	uta_fib( "no-suhch-file" );			// drive some error checking for coverage

/*
	if( ctx ) {
		if( ctx->rtg_addr ) {
			free( ctx->rtg_addr );
		}
		free( ctx );
	}
*/

	state = uta_link2( "worm", NULL, NULL );
	errors += fail_if_true( state, "link2 did not return false when given nil pointers" );

	state = uta_epsock_rr( rt, 122, 0, NULL, NULL );
	errors += fail_if_true( state, "uta_epsock_rr returned bad state when given nil socket pointer" );

	rt = uta_rt_init( );										// get us a route table
	state = uta_epsock_rr( rt, 0, -1, NULL, &nn_sock );
	errors += fail_if_true( state, "uta_epsock_rr returned bad state (true) when given negative group number" );

	return !!errors;			// 1 or 0 regardless of count
}

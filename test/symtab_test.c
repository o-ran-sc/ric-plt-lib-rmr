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
	Mnemonic:	symtab_test.c
	Abstract:	This is the unit test module that will drive tests against
				the symbol table portion of RMr.  Run with:
					ksh unit_test.ksh symtab_test.c
	Date:		1 April 2019
	Author:		E. Scott Daniels
*/

#include <pthread.h>

#define NO_DUMMY_RMR 1			// no dummy rmr functions; we don't pull in rmr.h or agnostic.h
#define NO_EMULATION
#define NO_PRIVATE_HEADERS

#include <rmr.h>
#include <rmr_agnostic.h>
#include "test_support.c"
#include "rmr_symtab.h"

#include "symtab.c"							// module under test


int terrors = 0;							// thread errors
int state = GOOD;							// overall pass/fail state 0==fail
int counter;								// global counter for for-each tests



static int fetch( void* st, char* key, int class, int expected ) {
	char* val;
	int error = 0;

	val = rmr_sym_get( st, key, class );
	if( val ) {
		fprintf( stderr, "[%s] get returns key=%s val=%s\n",  !expected ? "FAIL" : "OK", key, val );
		if( !expected ) {
			state = BAD;
			error = 1;
		}

	} else {
		fprintf( stderr, "[%s] string key fetch return nil\n", expected ? "FAIL" : "OK" );
		if( expected ) {
			state = BAD;
			error = 1;
		}
	}

	return error;
}

static int nfetch( void* st, int key, int expected ) {
	char* val;
	int		error = 0;

	val = rmr_sym_pull( st, key );
	if( val ) {
		fprintf( stderr, "[%s] get returns key=%d val=%s\n", !expected ? "FAIL" : "OK", key, val );
		if( !expected )  {
			state = BAD;
			error = 1;
		}
	} else {
		fprintf( stderr, "[%s] get return nil for key=%d\n", expected ? "FAIL" : "OK", key );
		if( expected )  {
			state = BAD;
			error = 1;
		}
	}

	return error;
}

// ----------------- thread based tests -------------------------------------------------------------------
#define NUM_KEYS	512					// number of unique keys
#define NUM_ATTEMPTS	1000000

/*
	This is started in a thread and will attempt 10,000 reads on the symtable
	in an attempt to ensure that there are no concurrent read/write issues.
*/
static void* reader( void* st ) {
	char	key[1024];
	int		i;
	int		ncount = 0;			// number not found
	int		fcount = 0;			// number found

	for( i = 0; i < NUM_ATTEMPTS; i++ ) {
		snprintf( key, sizeof( key ), "key_%d", i % NUM_KEYS );
		if( rmr_sym_get( st, key, 1 ) == NULL ) {
			ncount++;
		} else {
			fcount++;
		}
	}

	fprintf( stderr, "<info> reader finished: n=%d f=%d\n", ncount, fcount );	// there is no right answer
	return NULL;
}

/*
	This is started in a thread and will attempt 10,000 writes on the symtable
	in an attempt to ensure that there are no concurrent read/write issues. Keys are
	written as key_n where n is an integer between 0 and 999 inclusive.
*/
static void* writer( void* st ) {
	char	key[1024];
	int		i;
	int		ncount = 0;			// number first inserts
	int		rcount = 0;			// number replacements
	char*	value = NULL;
	int		num_keys = 256;

	fprintf( stderr, "<INFO> writer now turning\n" );
	for( i = 0; i < NUM_ATTEMPTS; i++ ) {
		value++;
		snprintf( key, sizeof( key ), "key_%d", i % NUM_KEYS );
		rmr_sym_del( st, key, 1 );
		if( rmr_sym_put( st, key, 1, value )  ) {
			ncount++;
		} else {
			rcount++;
		}
	}

	if( ncount != NUM_ATTEMPTS ) {
		fprintf( stderr, "<FAIL> writer finished: n=%d r=%d\n", ncount, rcount );	// there is no right answer
		terrors++;
	} else {
		fprintf( stderr, "<INFO> writer finished: n=%d r=%d\n", ncount, rcount );	// there is no right answer
	}

	return NULL;
}

/*
	Drive a concurrent read/write test to ensure no race issues.
*/
static int thread_test( ) {
	pthread_t	tids[10];
	int			n2start = 3;
	int			i;
	void*		st;

	st = rmr_sym_alloc( 128 );			// should force collisions

	fprintf( stderr, "<INFO> starting writer\n" );
	pthread_create( &tids[0], NULL, writer, st );

	for( i = 1; i <= n2start; i++ ) {
		fprintf( stderr, "<INFO> starting reader %d\n", i );
		pthread_create( &tids[i], NULL, reader, st );
	}

	fprintf( stderr, "<INFO> thread controller is waiting\n" );
	for( i = 0; i <= n2start; i++ ) {
		pthread_join( tids[i], NULL );				// status is unimportant, just hold until all are done
		fprintf( stderr, "<INFO> thread %d  has reported complete\n", i );
	}


	rmr_sym_stats( st, 1 );
	return terrors;
}

// ---------------------------------------------------------------------------------------------------------

/*
	Driven by foreach class -- just incr the counter.
*/
static void each_counter( void* a, void* b, const char* c, void* d, void* e ) {
	counter++;
}

int main( ) {
	void*   st;
	char*   foo = "foo";
	char*   bar = "bar";
	char*	goo = "goo";				// name not in symtab
	int		i;
	int		class = 1;
	int		s;
	void*	p;
	int		errors = 0;

	st = rmr_sym_alloc( 10 );						// alloc with small value to force adjustment inside
	errors += fail_if_nil( st, "symtab pointer" );

	s = rmr_sym_put( st, foo, class, bar );			// add entry with string key; returns 1 if it was inserted
	errors += fail_if_false( s, "insert foo existed" );

	s = rmr_sym_put( st, foo, class+1, bar );		// add to table with a different class
	errors += fail_if_false( s, "insert foo existed" );

	s = rmr_sym_put( st, foo, class, bar );			// inserted above, should return not inserted (0)
	errors += fail_if_true( s, "insert foo existed" );

	errors += fetch( st, foo, class, 1 );
	errors += fetch( st, goo, class, 0 );			// fetch non existant
	rmr_sym_stats( st, 4 );							// early stats at verbose level 4 so chatter is minimised
	rmr_sym_dump( st );

	for( i = 2000; i < 3000; i++ ) {				// bunch of dummy things to force chains in the table
		rmr_sym_map( st, i, foo );					// add entry with unsigned integer key
	}
	rmr_sym_stats( st, 0 );							// just the small facts to verify the 1000 we stuffed in
	rmr_sym_ndel( st, 2001 );						// force a numeric key delete
	rmr_sym_ndel( st, 12001 );						// delete numeric key not there

	s = rmr_sym_map( st, 1234, foo );					// add known entries with unsigned integer key
	errors += fail_if_false( s, "numeric add of key 1234 should not have existed" );
	s = rmr_sym_map( st, 2345, bar );
	fail_if_true( s, "numeric add of key 2345 should have existed" );

	counter = 0;
	rmr_sym_foreach_class( st, 0, each_counter, NULL );
	errors += fail_if_false( counter, "expected counter after foreach to be non-zero" );

	errors += nfetch( st, 1234, 1 );
	errors += nfetch( st, 2345, 1 );

	rmr_sym_del( st, foo, 0 );

	rmr_sym_stats( st, 0 );

	rmr_sym_free( NULL );			// ensure it doesn't barf when given a nil pointer
	rmr_sym_free( st );

	test_summary( errors, "symtab tests" );
	if( state + errors == 0 ) {
		fprintf( stderr, "<PASS> all symtab tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in symtab code\n\n", errors );
	}

	errors += thread_test();

	return !!(state + errors);
}


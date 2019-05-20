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
	Author: 	E. Scott Daniels
*/

#define NO_DUMMY_RMR 1			// no dummy rmr functions; we don't pull in rmr.h or agnostic.h

#include "rmr_symtab.h"
#include "symtab.c"

#include "test_support.c"

int state = GOOD;							// overall pass/fail state 0==fail
int counter;								// global counter for for-each tests



static void fetch( void* st, char* key, int class, int expected ) {
	char* val;

	val = rmr_sym_get( st, key, class );
	if( val ) {
		fprintf( stderr, "[%s] get returns key=%s val=%s\n",  !expected ? "FAIL" : "OK", key, val );
		if( !expected ) {
			state = BAD;
		}

	} else {
		fprintf( stderr, "[%s] string key fetch return nil\n", expected ? "FAIL" : "OK" );
		if( expected ) {
			state = BAD;
		}
	}
}

static void nfetch( void* st, int key, int expected ) {
	char* val;

	val = rmr_sym_pull( st, key );
	if( val ) {
		fprintf( stderr, "[%s] get returns key=%d val=%s\n", !expected ? "FAIL" : "OK", key, val );
		if( !expected )  {
			state = BAD;
		}
	} else {
		fprintf( stderr, "[%s] get return nil for key=%d\n", expected ? "FAIL" : "OK", key );
		if( expected )  {
			state = BAD;
		}
	}
}


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

	st = rmr_sym_alloc( 10 );						// alloc with small value to force adjustment inside
	fail_if_nil( st, "symtab pointer" );

	s = rmr_sym_put( st, foo, class, bar );			// add entry with string key; returns 1 if it was inserted
	fail_if_false( s, "insert foo existed" );

	s = rmr_sym_put( st, foo, class+1, bar );		// add to table with a different class
	fail_if_false( s, "insert foo existed" );

	s = rmr_sym_put( st, foo, class, bar );			// inserted above, should return not inserted (0)
	fail_if_true( s, "insert foo existed" );

	fetch( st, foo, class, 1 );
	fetch( st, goo, class, 0 );					// fetch non existant
	rmr_sym_stats( st, 4 );							// early stats at verbose level 4 so chatter is minimised
	rmr_sym_dump( st );

	for( i = 2000; i < 3000; i++ ) {			// bunch of dummy things to force chains in the table
		rmr_sym_map( st, i, foo );					// add entry with unsigned integer key
	}
	rmr_sym_stats( st, 0 );							// just the small facts to verify the 1000 we stuffed in
	rmr_sym_ndel( st, 2001 );						// force a numeric key delete
	rmr_sym_ndel( st, 12001 );						// delete numeric key not there

	s = rmr_sym_map( st, 1234, foo );					// add known entries with unsigned integer key
	fail_if_false( s, "numeric add of key 1234 should not have existed" );
	s = rmr_sym_map( st, 2345, bar );
	fail_if_true( s, "numeric add of key 2345 should have existed" );

	counter = 0;
	rmr_sym_foreach_class( st, 0, each_counter, NULL );
	fail_if_false( counter, "expected counter after foreach to be non-zero" );

	nfetch( st, 1234, 1 );
	nfetch( st, 2345, 1 );


	rmr_sym_del( st, foo, 0 );

	rmr_sym_stats( st, 0 );

	rmr_sym_free( NULL );			// ensure it doesn't barf when given a nil pointer
	rmr_sym_free( st );

	return state;
}


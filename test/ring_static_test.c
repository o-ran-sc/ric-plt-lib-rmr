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
	Mmemonic:	ring_static_test.c
	Abstract:	Test the ring funcitons. These are meant to be included at compile
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

//#include "rmr.h"
//#include "rmr_agnostic.h"


/*
	Conduct a series of interleaved tests inserting i-factor
	values before beginning to pull values (i-factor must be
	size - 2 smaller than the ring.
	Returns 0 on success, 1 on insert failure and 2 on pull failure.
*/
static int ie_test( void* r, int i_factor, long inserts ) {
	int i;
	int* dp;
	int data[29];

	for( i = 0; i < inserts; i++ ) {
		data[i%29] = i;
		if( ! uta_ring_insert( r, &data[i%29] ) ) {
			fprintf( stderr, "<FAIL> interleaved insert failed on ifactor=%d i=%d\n", i_factor, i );
			return 1;
		}
		if( i > i_factor-1 ) {
			dp = uta_ring_extract( r );
			if( *dp != data[(i-i_factor)%29] ) {
				fprintf( stderr, "<FAIL> interleaved exctract failed on ifactor=%d i=%d expected=%d got=%d\n", i_factor, i, data[(i-i_factor)%29], *dp );
				return 2;
			}
		}
	}
	//fprintf( stderr, "<OK>   interleaved insert/extract test passed for insert factor %d\n", i_factor );

	return 0;
}

static int ring_test( ) {
	void* r;
	int i;
	int j;
	int	data[20];
	int*	dp;
	int size = 18;
	int	pfd = -1;					// pollable file descriptor for the ring
	int	errors = 0;

	r = uta_mk_ring( 0 );			// should return nil
	errors += fail_not_nil( r, "attempt to make a ring with size 0 returned a pointer" );

	r = uta_mk_ring( -1 );			// should also return nil
	errors += fail_not_nil( r, "attempt to make a ring with negative size returned a pointer" );

	r = uta_mk_ring( 18 );
	errors += fail_if_nil( r, "attempt to make a ring with valid size returned a nil pointer" );

	pfd = uta_ring_getpfd( r );		// get pollable file descriptor
	errors += fail_if_true( pfd < 0, "pollable file descriptor returned was bad" );

	pfd = uta_ring_config( r, 0x03 );		// turn on locking for reads and writes
	errors += fail_if_true( pfd != 1, "attempt to enable locking failed" );

	for( i = 0; i < 20; i++ ) {		// test to ensure it reports full when head/tail start at 0
		data[i] = i;
		if( ! uta_ring_insert( r, &data[i] ) ) {
			break;
		}
	}

	errors += fail_if_true( i > size, "ring insert did not report full table" );

	for( i = 0; i < size + 3; i++ ) {								// ensure they all come back in order, and we don't get 'extras'
		if( (dp = uta_ring_extract( r )) == NULL ) {
			errors += fail_if_true( i < size-1, "nil pointer on extract from full table" );
			break;
		}

		if( fail_if_true( *dp != i, "extracted data is incorrect; see details below" )) {
			fprintf( stderr, "<FAIL> data at i=% isnt right; expected %d got %d\n", i, i, *dp );
			errors++;
		}
	}
	fail_if_true( i > size, "got too many values from extract loop" );

	uta_ring_free( NULL );							// ensure this doesn't blow up
	uta_ring_free( r );
	for( i = 2; i < 15; i++ ) {
		r = uta_mk_ring( 16 );
		errors += fail_not_equal( ie_test( r, i, 101 ), 0, "ie test for 101 inserts didn't return 0" );

		uta_ring_free( r );
	}

	size = 5;
	for( j = 0; j < 20; j++ ) {
		for( i = 2; i < size - 2; i++ ) {
			r = uta_mk_ring( size );
			errors += fail_not_equal( ie_test( r, i, 66000 ), 0, "ie test for 66K inserts didn't return 0" );

			uta_ring_free( r );
		}

		size++;
	}

	return errors;
}

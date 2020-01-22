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

#include "rmr.h"
#include "rmr_agnostic.h"


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
	if( r != NULL ) {
		fprintf( stderr, "<FAIL> attempt to make a ring with size 0 returned a pointer\n" );
		return 1;
	}
	r = uta_mk_ring( -1 );			// should also return nil
	if( r != NULL ) {
		fprintf( stderr, "<FAIL> attempt to make a ring with size <0 returned a pointer\n" );
		return 1;
	}

	r = uta_mk_ring( 18 );
	if( r == NULL ) {
		fprintf( stderr, "<FAIL> unable to make ring with 17 entries\n" );
		return 1;
	}

	pfd = uta_ring_getpfd( r );		// get pollable file descriptor
	if( pfd < 0 ) {
		fprintf( stderr, "<FAIL> expected a pollable file descriptor >= 0, but got: %d\n", pfd );
		errors++;
	}

	pfd = uta_ring_config( r, 0x03 );		// turn on locking for reads and writes
	if( pfd != 1 ) {
		fprintf( stderr, "<FAIL> config attempt to enable locking failed\n" );
		errors++;
	}
	

	for( i = 0; i < 20; i++ ) {		// test to ensure it reports full when head/tail start at 0
		data[i] = i;
		if( ! uta_ring_insert( r, &data[i] ) ) {
			break;
		}
	}

	if( i > size ) {
		fprintf( stderr, "<FAIL> didn not report table full: i=%d\n", i );
		return 1;
	}

	fprintf( stderr, "<OK>   reported table full at i=%d as expected\n", i );


	for( i = 0; i < size + 3; i++ ) {								// ensure they all come back in order, and we don't get 'extras'
		if( (dp = uta_ring_extract( r )) == NULL ) {
			if( i < size-1 ) {
				fprintf( stderr, "<FAIL> nil pointer at i=%d\n", i );
				return 1;
			} else {
				break;
			}
		}

		if( *dp != i ) {
			fprintf( stderr, "<FAIL> data at i=% isnt right; expected %d got %d\n", i, i, *dp );
		}
	}
	if( i > size ) {
		fprintf( stderr, "<FAIL> got too many values on extract: %d\n", i );
		return 1;
	}
	fprintf( stderr, "<OK>   extracted values were sane, got: %d\n", i-1 );

	uta_ring_free( NULL );							// ensure this doesn't blow up
	uta_ring_free( r );
	for( i = 2; i < 15; i++ ) {
		r = uta_mk_ring( 16 );
		if( ie_test( r, i, 101 ) != 0 ) {			// modest number of inserts
			fprintf( stderr, "<FAIL> ie test for 101 inserts didn't return 0\n" );
			return 1;
		}

		uta_ring_free( r );
	}
	fprintf( stderr, "<OK>   all modest insert/exctract tests pass\n" );

	size = 5;
	for( j = 0; j < 20; j++ ) {
		for( i = 2; i < size - 2; i++ ) {
			r = uta_mk_ring( size );
			if( ie_test( r, i, 66000 ) != 0 ) {			// should force the 16bit head/tail indexes to roll over
				fprintf( stderr, "<FAIL> ie test for 66K inserts didn't return 0\n" );
				return 1;
			}

			uta_ring_free( r );
		}
		fprintf( stderr, "<OK>   all large insert/exctract tests pass ring size=%d\n", size );

		size++;
	}

	fprintf( stderr, "<INFO> all ring tests pass\n" );
	return errors;
}

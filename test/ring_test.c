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
	Mmemonic:	ring_test.c
	Abstract:	This is a stand alone test driver for the ring module. It
				includes the static tests after setting up the environment
				then invokes it.

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
#include "ring_static.c"

#include "test_support.c"					// things like fail_if()
#include "ring_static_test.c"				// the actual tests

int main( ) {
	int errors = 0;

	errors += ring_test( );

	if( errors ) {
		fprintf( stderr, "<FAIL> ring tests failed\n" );
	} else {
		fprintf( stderr, "<OK>	 ring tests pass\n" );
	}

	return errors;
}

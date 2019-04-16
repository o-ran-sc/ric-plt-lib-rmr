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
	Mnemonic:	mbuf_api_test.c
	Abstract:	Unit tests for the mbuf common API functions.
	Author:		E. Scott Daniels
	Date:		2 April 2019
*/


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>


#include "../src/common/include/rmr.h"
#include "../src/common/include/rmr_agnostic.h"

#include "../src/common/src/mbuf_api.c"			// module under test

#include "test_support.c"						// our private library of test tools
#include "mbuf_api_static_test.c"				// test functions

int main( ) {
	int errors = 0;

	errors += mbuf_api_test( );

	if( errors ) {
		fprintf( stderr, "<FAIL> mbuf_api tests failed\n" );
	} else {
		fprintf( stderr, "<OK>	 mbuf_api tests pass\n" );
	}
}

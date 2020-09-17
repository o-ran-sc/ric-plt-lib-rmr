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
	Mnemonic:	tools_test.c
	Abstract:	Unit tests for the RMr tools module.
	Author:		E. Scott Daniels
	Date:		21 January 2019
*/


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

#include "rmr.h"
#include "rmr_logging.h"
#include "rmr_agnostic.h"

#define NO_EMULATION
#include "test_support.c"		// our private library of test tools

#include "logging.c"		// tools references logging, so pull in too
#include "tools_static.c"

#include "tools_static_test.c"

int main( ) {
	int errors = 0;

	fprintf( stderr, ">>>> starting tools_test\n" );
	errors += tools_test() > 0;

	test_summary( errors, "tool tests" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tool tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in tool code\n\n", errors );
	}

	return !!errors;
}


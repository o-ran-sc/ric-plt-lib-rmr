// :vi sw=4 ts=4 noet:
/*
==================================================================================
	Copyright (c) 2020 Nokia
	Copyright (c) 2020 AT&T Intellectual Property.

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
	Mmemonic:	rmr_si_rcv_test.c
	Abstract:	This drives only the receive tests for the SI API. Because
				of the threaded nature of SI receives it is not possible to
				mix these tests with the other coverage tests which allocate
				various contexes.

	Author:		E. Scott Daniels
	Date:		14 April 2020		(AKD)
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>

#define DEBUG 1
#define PARANOID_CHECKS	1					// must have parinoid testing on to not fail on nil pointer tests

											// specific test tools in this directory
#undef NNG_UNDER_TEST 
#include "test_support.c"					// things like fail_if()
#include "test_msg_support.c"
#include "test_gen_rt.c"


#include "rmr.h"					// things the users see
#include "rmr_symtab.h"
#include "rmr_logging.h"
#include "rmr_agnostic.h"			// transport agnostic header

#include "symtab.c"
#include "logging.c"
#include "rmr_si.c"
#include "mbuf_api.c"

#include "test_ctx_support.c"				// dummy context support (needs rmr headers)


static void gen_rt( uta_ctx_t* ctx );		// defined in sr_si_static_test, but used by a few others (eliminate order requirement below)

											// and finally....
#include "rmr_si_rcv_static_test.c"			// the only test driver


/*
	Drive each of the separate tests and report.
*/
int main() {
	int errors = 0;

	rmr_set_vlevel( 5 );			// enable all debugging

	fprintf( stderr, "\n<INFO> starting receive tests (%d)\n", errors );
	errors += rmr_rcv_test();
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	test_summary( errors, "receive tests" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d modules reported errors\n\n", errors );
	}

	return !!errors;
}

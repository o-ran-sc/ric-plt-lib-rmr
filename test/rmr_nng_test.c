// :vi sw=4 ts=4 noet:
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
	Mmemonic:	rmr_nng_test.c
	Abstract:	This tests the whole rmr nng implementation. This driver
				includes all of the module specific unit test static files
				(e.g. wormhole_static_test.c) and drives the tests contained
				there.   The individual modules allow them to be driven by
				a standalone driver, and to be maintained separately. We must
				test by inclusion because of the static nature of the internal
				functions of the library.

	Author:		E. Scott Daniels
	Date:		18 January 2018		(IMO HRTL)
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

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pipeline0/push.h>
#include <nng/protocol/pipeline0/pull.h>

#define NNG_UNDER_TEST 1
#define EMULATE_NNG
#include "test_nng_em.c"							// nng/nn emulation (before including things under test)


#include "rmr.h"					// things the users see
#include "rmr_symtab.h"
#include "rmr_logging.h"
#include "rmr_agnostic.h"			// transport agnostic header

#include "symtab.c"
#include "logging.c"
#include "rmr_nng.c"
#include "mbuf_api.c"

static void gen_rt( uta_ctx_t* ctx );		// defined in sr_nng_static_test, but used by a few others (eliminate order requirement below)

											// specific test tools in this directory
#include "test_support.c"					// things like fail_if()
#include "test_ctx_support.c"				// specifically geared to creating dummy contex structs
#include "test_gen_rt.c"
											// and finally....
#include "tools_static_test.c"				// local test functions pulled directly because of static nature of things
#include "symtab_static_test.c"
#include "ring_static_test.c"
#include "rt_static_test.c"
#include "sr_nng_static_test.c"
#include "wormhole_static_test.c"
#include "rmr_nng_api_static_test.c"
#include "mbuf_api_static_test.c"


/*
	Drive each of the separate tests and report.
*/
int main() {
	int errors = 0;

	fprintf( stderr, "\n<INFO> starting tool tests\n" );
	errors += tools_test();
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting ring tests (%d)\n", errors );
	errors += ring_test();
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting symtab tests\n" );
	errors += symtab_test( );
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting rtable tests\n" );
	errors += rt_test();				// route table tests
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting RMr API tests\n" );
	errors += rmr_api_test();

	fprintf( stderr, "\n<INFO> run RMr API tests with src name only env var set\n" );
	setenv( "RMR_SRC_NAMEONLY", "1", 1 );
	errors += rmr_api_test();
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting wormhole tests\n" );
	errors += worm_test();				// test wormhole funcitons
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting send/receive tests\n" );
	errors += sr_nng_test();				// test the send/receive static functions
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	fprintf( stderr, "\n<INFO> starting mbuf api tests\n" );
	errors +=  mbuf_api_test( );
	fprintf( stderr, "<INFO> error count: %d\n", errors );

	test_summary( errors, "tool tests" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d modules reported errors\n\n", errors );
	}

	return !!errors;
}

// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2021 Nokia
	    Copyright (c) 2021 AT&T Intellectual Property.

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
	Mmemonic:	alarm_test.c
	Abstract:	Unit test for common/src/alarm.c functions.

	Author:		E. Scott Daniels
	Date:		22 February 2021
*/

#ifdef KEEP

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
//#include <sys/epoll.h>
//#include <pthread.h>
//#include <semaphore.h>

#define DEBUG 1								// must define before pulling in rmr header files
#define PARANOID_CHECKS	1					// must have parinoid testing on to not fail on nil pointer tests


											// specific test tools in this directory
#undef NNG_UNDER_TEST
#include "test_support.c"					// things like fail_if()
#include "test_msg_support.c"
#include "test_gen_rt.c"


#include "rmr.h"					// things the users see
#include "rmr_agnostic.h"			// rmr private things

#include "rmr_symtab.h"				// must pull in for context setup
#include "rmr_agnostic.h"			// transport agnostic header

#include "logging.c"
#include "rt_generic_static.c"
#include "tools_static.c"
#include "symtab.c"
//#include "rmr_si.c"
//#include "mbuf_api.c"

#include "test_ctx_support.c"				// dummy context support (needs symtab defs, so not with others above)


//static void gen_rt( uta_ctx_t* ctx );		// defined in sr_si_static_test, but used by a few others (eliminate order requirement below)

											// and finally.... the things under test
#include "alarm.c"
//#include "tools_static_test.c"				// local test functions pulled directly because of static nature of things
//#include "symtab_static_test.c"
//#include "ring_static_test.c"
//#include "rt_static_test.c"
//#include "wormhole_static_test.c"
//#include "mbuf_api_static_test.c"
//#include "sr_si_static_test.c"
//#include "lg_buf_static_test.c"
// do NOT include the receive test static must be stand alone


#endif

/*
	These tests assume there is a dummy process listening on 127.0.0.1:1986; if it's not there
	the tests still pass, but coverage is reduced because the sends never happen.
*/
static int alarm_test( ) {
	int errors = 0;			// number errors found
	uta_ctx_t* ctx;
	char*	endpt = NULL;

	ctx = mk_dummy_ctx();
	gen_rt( ctx );

	ctx->my_name = strdup( "private" );
	ctx->my_ip = strdup( "30.4.19.86:2750" );

	endpt = uta_alarm_endpt();						// check defaults are generated
	if( fail_if_nil( endpt, "alarm endpoint did not generate a string for defaults" ) ) {
		errors++;
	} else {
		errors += fail_if_false( strcmp( endpt, "service-ricplt-alarmmanager-rmr:4560" ) == 0, "alarm endpoint default string not expected" );
		free( endpt );
	}

	setenv( "ALARM_MGR_SERVICE_NAME", "127.0.0.1", 1 );				// test to ensure digging from env is good too
	setenv( "ALARM_MGR_SERVICE_PORT", "999", 1 );					// connect should fail
	endpt = uta_alarm_endpt();						// check defaults are generated
	uta_alarm( ctx, ALARM_RAISE, 0, "some info for the alarm" );	// this should fail as the service isn't running

	setenv( "ALARM_MGR_SERVICE_NAME", "127.0.0.1", 1 );				// test to ensure digging from env is good too
	setenv( "ALARM_MGR_SERVICE_PORT", "1986", 1 );
	endpt = uta_alarm_endpt();						// check defaults are generated
	if( fail_if_nil( endpt, "alarm endpoint did not generate a string when name/port are set in env" ) ) {
		errors++;
	} else {
		errors += fail_if_false( strcmp( endpt, "127.0.0.1:1986" ) == 0, "alarm endpoint string not expected when values are in env" );
		free( endpt );
	}


	// these functions do not return values; driving for coverage and crash testing
	uta_alarm( ctx, ALARM_RAISE, 0, "some info for the alarm" );
	uta_alarm( ctx, ALARM_CLEAR, 0, NULL );
	uta_alarm_send( ctx, NULL );									// ensure nil message doesn't crash us


	if( ctx ) {
		free( ctx->my_name );
		free( ctx->my_ip );
		free( ctx );
	}

	return !!errors;			// 1 or 0 regardless of count
}
/*

int main( ) {
	int errors = 0;

	errors += alarm_test();
	exit( !!errors );
}
*/

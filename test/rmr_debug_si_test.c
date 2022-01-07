// vim: ts=4 sw=4 noet
/*
==================================================================================
	Copyright (c) 2021 Alexandre Huff (UTFPR).

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
	Mnemonic:	rmr_debug_si_test.c
	Abstract:	This is the main driver to test the si95 debug functions

	Date:		24 December 2021
	Author:		Alexandre Huff
*/
#include <netdb.h>
#include <errno.h>

#define DEBUG 1
#undef NNG_UNDER_TEST						// NNG is NOT under test so undefine if set
#define NO_EMULATION 1						// no emulation of transport functions

#include "rmr.h"
#include "rmr_agnostic.h"

#include "si95/socket_if.h"
#undef uta_ctx_t
#include "rmr_si_private.h"

#include "test_support.c"					// things like fail_if()

#include "rmr_debug_si.c"                   // api under test




// ------------- dummy functions to force edge cases when we can ---------------------------------------

#define SYSTEM_UNDER_TEST	1				// for conditional code


// ----------------------------------------------------------------------------------------

static int get_debug_info_test( uta_ctx_t *ctx ) {
    int errors = 0;
    int ret;
    rmr_rx_debug_t info;

    ctx->acc_dcount = 5;    // dummy info
    ctx->acc_ecount = 10;

    // argument related things
    ret = rmr_get_rx_debug_info( NULL, &info );
    errors += fail_not_equal( EINVAL, errno, "get_debug_info_test: rmr_get_rx_debug_info did not set errno to EINVAL on nil global context" );
    errors += fail_if_equal( 0, ret, "get_debug_info_test: rmr_get_rx_debug_info returned 0 on error" );

    errno = 0;
    ret = rmr_get_rx_debug_info( ctx, NULL );
    errors += fail_not_equal( EINVAL, errno, "get_debug_info_test: rmr_get_rx_debug_info did not set errno to EINVAL on nil info struct" );
    errors += fail_if_equal( 0, ret, "get_debug_info_test: rmr_get_rx_debug_info returned 0 on error" );

    // test rx debug struct values
    ret = rmr_get_rx_debug_info( ctx, &info );
    errors += fail_not_equal( 0, ret, "get_debug_info_test: rmr_get_rx_debug_info did not return 0 on success" );
    errors += fail_not_equal( 5, ctx->acc_dcount, "get_debug_info_test: rmr_get_rx_debug_info unexpected acc_dcount value in info struct" );
    errors += fail_not_equal( 10, ctx->acc_ecount, "get_debug_info_test: rmr_get_rx_debug_info unexpected acc_ecount value in info struct" );

    fprintf( stderr, "<INFO> get_debug_info_test finished with %d errors\n", errors );

    return errors;
}

static int reset_debug_test(uta_ctx_t *ctx) {
    int errors = 0;
    int ret;

    ctx->acc_dcount = 5;    // dummy info
    ctx->acc_ecount = 10;

    ret = rmr_reset_rx_debug_count( NULL ); // expect to fail
    errors += fail_not_equal( EINVAL, errno, "reset_debug_test: rmr_reset_rx_debug_count did not set errno to EINVAL on error" );
    errors += fail_if_equal( 0, ret, "reset_debug_test: rmr_reset_rx_debug_count returned 0 on error" );

    ret = rmr_reset_rx_debug_count( ctx );  // expect to return successfully
    errors += fail_not_equal( 0, ret, "reset_debug_test: reset_debug_rx_count did not return 0 on success" );
    errors += fail_not_equal( 0, ctx->acc_dcount, "reset_debug_test: rmr_reset_rx_debug_count did not reset acc_dcount to 0" );
    errors += fail_not_equal( 0, ctx->acc_ecount, "reset_debug_test: rmr_reset_rx_debug_count did not reset acc_ecount to 0" );

    fprintf( stderr, "<INFO> reset_debug_test finished with %d errors\n", errors );

    return errors;
}

// ----------------------------------------------------------------------------------------

/*
	Drive tests...
*/
int main() {
    uta_ctx_t si_ctx;
    int errors = 0;

	fprintf( stderr, "\n<INFO> starting SI95 debug api tests\n" );

    errors += get_debug_info_test( &si_ctx );
    errors += reset_debug_test( &si_ctx );

	test_summary( errors, "SI95 debug api tests" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in SI95 debug api code\n\n", errors );
	}

	return !!errors;
}

// : vim ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2019-2020 Nokia
	    Copyright (c) 2018-2020 AT&T Intellectual Property.

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
	Mnemonic:	logging_test.c
	Abstract:	This test drives logging related tests.
	Date:		1 April 2019
	Author: 	E. Scott Daniels
*/

#define NO_DUMMY_RMR 1			// no dummy rmr functions; we don't pull in rmr.h or agnostic.h
#define NO_EMULATION

#include "rmr_logging.h"
#include "logging.c"
#include "test_support.c"

/*
	Logging can be difficult to verify as stderr needs to be captured and examined.
	We will verify internally what we can, and drive logging functions for coverage.
*/
int main( ) {
	int llevel = 99;
	int errors = 0;

	setenv( "RMR_HR_LOG", "1", 1 );			// drive for coverage in init
	setenv( "RMR_LOG_VLEVEL", "90", 1 );	// force test for out of range during init

	rmr_vlog( RMR_VL_CRIT, "crit message should be written\n" );		// force coverage; should drive init

	llevel = rmr_vlog_init( );
	errors += fail_if_equal( llevel, 99, "llevel was not reset by vlog init" );
	errors += fail_if_equal( llevel, 90, "vlog init did not catch out of range vlog" );

	llevel = 99;
	setenv( "RMR_LOG_VLEVEL", "-10", 1 );	// force test for out of range during init
	llevel = rmr_vlog_init( );
	errors += fail_if_equal( llevel, 99, "neg llevel was not reset by vlog init" );
	errors += fail_if_equal( llevel, -10, "vlog init did not catch out of range (neg) vlog" );

	rmr_set_vlevel( 2 );

	/*
		The remainder of these tests can be validated only by looking at the stderr
		for the process. If any "should not be written" messages appear, then the
		test should be marked as a failure. In a similar vein, the number of
		expected "should be written" messages should be found.
	*/
	rmr_vlog( RMR_VL_DEBUG, "debug message should not be written\n" );
	rmr_vlog( RMR_VL_INFO, "info message should not be written\n" );
	rmr_vlog( RMR_VL_WARN, "warn message should not be written\n" );
	rmr_vlog( RMR_VL_ERR, "error message should be written\n" );
	rmr_vlog( RMR_VL_CRIT, "crit message should be written\n" );

	rmr_set_vlevel( 5 );
	rmr_vlog( RMR_VL_DEBUG, "debug message should be written\n" );
	rmr_vlog( RMR_VL_INFO, "info message should be written\n" );
	rmr_vlog( RMR_VL_WARN, "warn message should be written\n" );
	rmr_vlog( RMR_VL_ERR, "error message should be written\n" );
	rmr_vlog( RMR_VL_CRIT, "crit message should be written\n" );

	rmr_set_vlevel( 0 );
	rmr_vlog( RMR_VL_DEBUG, "debug message should not be written\n" );
	rmr_vlog( RMR_VL_INFO, "info message should not be written\n" );
	rmr_vlog( RMR_VL_WARN, "warn message should not be written\n" );
	rmr_vlog( RMR_VL_ERR, "error message should not be written\n" );
	rmr_vlog( RMR_VL_CRIT, "crit message should not be written\n" );

	rmr_set_vlevel( 1 );
	rmr_vlog_force( RMR_VL_DEBUG, "debug forced message should be written\n" );
	rmr_vlog_force( RMR_VL_INFO, "info forced message should be written\n" );
	rmr_vlog_force( RMR_VL_WARN, "warn forced message should be written\n" );
	rmr_vlog_force( RMR_VL_ERR, "error forced message should be written\n" );
	rmr_vlog_force( RMR_VL_CRIT, "crit forced message should be written\n" );

	// CAUTION -- this needs to be manually updated when 'should be' messages are added!!
	fprintf( stderr, "<INFO> expeted 'should be' messages count is 13\n" );

	rmr_vlog( -1, "out of range message might be written\n" );			// drive range checks
	rmr_vlog( 10, "out of range message should not be written\n" );

	rmr_vlog_force( -1, "out of range message might be written\n" );			// drive range checks
	rmr_vlog_force( 10, "out of range message should not be written\n" );

	test_summary( errors, "logging tests" );
	return errors > 0;
}

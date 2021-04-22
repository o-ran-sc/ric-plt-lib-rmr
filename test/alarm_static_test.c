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


/*
	These tests assume there is a dummy process listening on 127.0.0.1:1986; if it's not there
	the tests still pass, but coverage is reduced because the sends never happen.
*/
static int alarm_test( ) {
	int errors = 0;				// number errors found
	uta_ctx_t* ctx;
	uta_ctx_t* pctx;			// tests  into rtable functions need a second context
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


	// ------ drive the alarm if dropping function in the route table code --------------------------------

	pctx = mk_dummy_ctx();				// grab a private context for rt to use

	/*
		These tests don't return anything that we can check; driving just to cover the lines and ensure
		we don't segfault or something bad like that.
	*/
	ctx->dcount - 0;
	alarm_if_drops( ctx,  pctx );			// should do nothing; no drops indicated

	ctx->dcount = 1024;						// make it look like we dropped things
	alarm_if_drops( ctx,  pctx );			// should drive the code block to send alarm and put is in dropping mode

	ctx->dcount = 1028;						// make it look like we are still  dropping
	alarm_if_drops( ctx,  pctx );			// drive the just reset time to clear block

	alarm_if_drops( ctx,  pctx );			// drive the check to see if past the clear time (it's not) to reset timer

	fprintf( stderr, "<TEST> pausing 65 seconds before driving last alarm if drops call\n" );
	sleep( 65 );							// we must pause for longer than the timer so we can drive last block
	alarm_if_drops( ctx,  pctx );			// should appear that we're not dropping and reset the alarm


	// -------------------------- tidy the house ---------------------------------------------------------
	if( ctx ) {
		free( ctx->my_name );
		free( ctx->my_ip );
		free( ctx );
	}

	if( pctx ) {
		free( pctx->my_name );
		free( pctx->my_ip );
		free( pctx );
	}

	return errors;
}

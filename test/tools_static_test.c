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
	Mnemonic:	tools_static_test.c
	Abstract:	Unit tests for the RMr tools module. This file is a static include
				that is pulle in at compile time by the test driver.  The driver is
				expected to include necessary rmr*.h and test_support files before
				including this file.  In addition, a context struct, or dummy, must
				be provided based on the type of testing being done.

	Author:		E. Scott Daniels
	Date:		3 April 2019
*/


static int tools_test( ) {
	int i;
	int j;
	int errors = 0;
	char* tokens[127];
	char* buf = "2,Fred,Wilma,Barney,Betty,Dino,Pebbles,Bambam,Mr. Slate,Gazoo";
	char*	dbuf;				// duplicated buf since C marks a const string is unumtable
	char*	hname;
	uta_ctx_t ctx;				// context for uta_lookup test
	void*	if_list;


	// ------------------ tokenise tests -----------------------------------------------------------
	dbuf = strdup( buf );
	i = uta_tokenise( dbuf, tokens, 127, ',' );
	errors += fail_not_equal( i, 10, "unexpected number of tokens returned (comma sep)" );
	for( j = 0; j < i; j++ ) {
		//fprintf( stderr, ">>>> [%d] (%s)\n", j, tokens[j] );
		errors += fail_if_nil( tokens[j], "token from buffer" );
	}
	errors += fail_not_equal( strcmp( tokens[4], "Betty" ), 0, "4th token wasn't 'Betty'" );

	free( dbuf );
	dbuf = strdup( buf );
	i = uta_tokenise( dbuf, tokens, 127, '|' );
	errors += fail_not_equal( i, 1, "unexpected number of tokens returned (bar sep)" );
	free( dbuf );

	// ------------ has str tests -----------------------------------------------------------------
	j = uta_has_str( buf, "Mr. Slate", ',', 1 );			// should fail (-1) because user should use strcmp in this situation
	errors += fail_if_true( j >= 0, "test to ensure has str rejects small max" );

	j = uta_has_str( buf, "Mr. Slate", ',', 27 );
	errors += fail_if_true( j < 0, "has string did not find Mr. Slate" );

	j = uta_has_str( buf, "Mrs. Slate", ',', 27 );
	errors += fail_if_true( j >= 0, "has string not found Mrs. Slate" );

	// ------------ host name 2 ip tests ---------------------------------------------------------
	hname = uta_h2ip( "192.168.1.2" );
	errors += fail_not_equal( strcmp( hname, "192.168.1.2" ), 0, "h2ip did not return IP address when given address" );
	errors += fail_if_nil( hname, "h2ip did not return a pointer" );

	hname = uta_h2ip( "yahoo.com" );
	errors += fail_if_nil( hname, "h2ip did not return a pointer" );

	hname = uta_h2ip( "yahoo.com:1234" );							// should ignore the port
	errors += fail_if_nil( hname, "h2ip did not return a pointer" );

	// ------------ rtg lookup test -------------------------------------------------------------
	ctx.rtg_port = 0;
	ctx.rtg_addr = NULL;

	i = uta_lookup_rtg( NULL );						// ensure it handles a nil context
	errors += fail_if_true( i, "rtg lookup returned that it found something when not expected to (nil context)" );

	setenv( "RMR_RTG_SVC", "localhost:1234", 1);
	i = uta_lookup_rtg( &ctx );
	errors += fail_if_false( i, "rtg lookup returned that it did not find something when expected to" );
	errors += fail_if_nil( ctx.rtg_addr, "rtg lookup did not return a pointer (with port)" );
	errors += fail_not_equal( ctx.rtg_port, 1234, "rtg lookup did not capture the port" );

	setenv( "RMR_RTG_SVC", "localhost", 1);			// test ability to generate default port
	uta_lookup_rtg( &ctx );
	errors += fail_if_nil( ctx.rtg_addr, "rtg lookup did not return a pointer (no port)" );
	errors += fail_not_equal( ctx.rtg_port, 5656, "rtg lookup did not return default port" );

	unsetenv( "RMR_RTG_SVC" );						// this should fail as the default name (rtg) will be unknown during testing
	i = uta_lookup_rtg( &ctx );
	errors += fail_if_true( i, "rtg lookup returned that it found something when not expected to" );

/*
//==== moved out of generic tools ==========
	// -------------- test link2 stuff ----------------------------------------------------------
	i = uta_link2( "bad" );					// should fail
	errors += fail_if_true( i >= 0, "uta_link2 didn't fail when given bad address" );

	i = uta_link2( "nohost:-1234" );
	errors += fail_if_true( i >= 0, "uta_link2 did not failed when given a bad (negative) port " );

	i = uta_link2( "nohost:1234" );					// nn should go off and set things up, but it will never successd, but uta_ call should
	errors += fail_if_true( i < 0, "uta_link2 failed when not expected to" );
*/

	// ------------ my ip stuff -----------------------------------------------------------------

	if_list = mk_ip_list( "1235" );
	errors += fail_if_nil( if_list, "mk_ip_list returned nil pointer" );

	i = has_myip( NULL, NULL, ',', 128 );		// should be false if pointers are nil
	errors += fail_if_true( i, "has_myip returned true when given nil buffer" );

	i = has_myip( "buffer contents not valid", NULL, ',', 128 );		// should be false if pointers are nil
	errors += fail_if_true( i, "has_myip returned true when given nil list" );

	i = has_myip( "buffer contents not valid", NULL, ',', 1 );			// should be false if max < 2
	errors += fail_if_true( i, "has_myip returned true when given small max value" );

	i = has_myip( "buffer.contents.not.valid", if_list, ',', 128 );		// should be false as there is nothing valid in the list
	errors += fail_if_true( i, "has_myip returned true when given a buffer with no valid info" );


	setenv( "RMR_BIND_IF", "192.168.4.30", 1 );			// drive the case where we have a hard set interface; and set known interface in list
	if_list = mk_ip_list( "1235" );
	errors += fail_if_nil( if_list, "mk_ip_list with env set returned nil pointer" );

	i = has_myip( "192.168.1.2:1235,192.168.4.30:1235,192.168.2.19:4567", if_list, ',', 128 );		// should find our ip in middle
	errors += fail_if_false( i, "has_myip did not find IP in middle of list" );

	i = has_myip( "192.168.4.30:1235,192.168.2.19:4567,192.168.2.19:2222", if_list, ',', 128 );		// should find our ip at head
	errors += fail_if_false( i, "has_myip did not find IP at head of list" );

	i = has_myip( "192.168.23.45:4444,192.168.1.2:1235,192.168.4.30:1235", if_list, ',', 128 );		// should find our ip at end
	errors += fail_if_false( i, "has_myip did not find IP at tail of list" );

	i = has_myip( "192.168.4.30:1235", if_list, ',', 128 );											// should find our ip when only in list
	errors += fail_if_false( i, "has_myip did not find IP when only one in list" );

	return !!errors;			// 1 or 0 regardless of count
}

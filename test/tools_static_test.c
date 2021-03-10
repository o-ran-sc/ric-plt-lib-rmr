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

// ------------ zero termianted buffer ---------------------------------------------------------
static int ztbf_test() {
	int errors = 0;
	char buf[128];
	char*	sshort = "Stand up and cheer! Cheer long and loud for old Ohio.";
	char*	slong = "Now is the time for the bobcat in the forest to make its way back to Court St for a round of pints at the Pub.";
	int l1;

	l1 = zt_buf_fill( buf, sshort, 64 );
	errors += fail_not_equal( l1, strlen( sshort ), "zt_buf_fill of short buf returned unexpected len" );
	errors += fail_not_equal( l1, strlen( buf ), "zt_buf_fill of short buf returned len did not match strlen" );

	l1 = zt_buf_fill( buf, slong, 64 );
	errors += fail_if_equal( l1, strlen( slong ), "zt_buf_fill of long buf returned unexpected len" );
	errors += fail_not_equal( l1, strlen( buf ), "zt_buf_fill of long buf returned len did not match strlen" );

	l1 = zt_buf_fill( buf, sshort, strlen( sshort ) );		// edge case of exact size
	errors += fail_not_equal( l1, strlen( sshort )-1, "zt_buf_fill exact length edge case failed" );

	l1 = zt_buf_fill( buf, sshort, 1 );						// unrealistic edge case
	errors += fail_not_equal( l1, 0, "zt_buf_fill dest len == 1 test failed" );

	return errors;
}

/*
	Returns an interface name that is valid in this environment (keeps us from
	having to know/guess a name to test with.
*/
static char* get_ifname( ) {
	if_addrs_t* l;
	struct	ifaddrs *ifs;		// pointer to head
	struct	ifaddrs *ele;		// pointer into the list
	char*	rstr = NULL;		// return string
	char    octs[NI_MAXHOST+1];


	if( (l = (if_addrs_t *) malloc( sizeof( if_addrs_t ) )) == NULL ) {
		return NULL;
	}
	memset( l, 0, sizeof( if_addrs_t ) );
	l->addrs = (char **) malloc( sizeof( char* ) * 128 );
	if( l->addrs == NULL ) {
		free( l );
		return NULL;
	}

	getifaddrs( &ifs );
	for( ele = ifs; ele; ele = ele->ifa_next ) {
		if( ele && strcmp( ele->ifa_name, "lo" ) ) {
			memset( octs, 0, sizeof( octs ) );
			getnameinfo( ele->ifa_addr, sizeof( struct sockaddr_in6 ),  octs, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
			if( *octs ) {
				rstr = strdup( ele->ifa_name );
				fprintf( stderr, "<INFO> found interface with address: %s\n", rstr );
				break;
			}
		}
	}

	free( l );
	if( rstr == NULL ) {
		fprintf( stderr, "<ERROR> no interface with an address was found!\n" );
	}
	return rstr;
}

static int tools_test( ) {
	int i;
	int j;
	int errors = 0;
	char* tokens[127];
	char* buf = "2,Fred,Wilma,Barney,Betty,Dino,Pebbles,Bambam,Mr. Slate,Gazoo";
	char*	dbuf;				// duplicated buf since C marks a const string is unumtable
	char*	hname;
	char*	ip;					// ip address string
	uta_ctx_t ctx;				// context for uta_lookup test
	void*	if_list;


	uta_dump_env();

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
	free( hname );

	hname = uta_h2ip( "yahoo.com" );
	errors += fail_if_nil( hname, "h2ip did not return a pointer" );
	free( hname );

	hname = uta_h2ip( "yahoo.com:1234" );							// should ignore the port
	errors += fail_if_nil( hname, "h2ip did not return a pointer" );
	free( hname );

	// ------------ rtg lookup test -------------------------------------------------------------
#ifdef KEEP
	// pub/sub route table generator is deprecated and should be removed at this point
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
#endif

	// ------------ my_ip stuff -----------------------------------------------------------------

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

	ip = get_default_ip( NULL );
	errors += fail_not_nil( ip, "get_default_ip returned non-nil pointer when given nil information" );

	ip = get_default_ip( if_list );
	if( ip ) {
		free( ip );
	} else {
		errors += fail_if_nil( ip, "get_defaul_ip returned nil pointer when valid pointer expected" );
	}

	ip = get_ifname();							// suss out a valid interface name (not lo)
	if( ip ) {
		setenv( "RMR_BIND_IF", ip, 1 );			// drive the case where we have a hard set interface; and set known interface in list
		free( ip );
		if_list = mk_ip_list( "1235" );
		if( if_list ) {
			ip = get_default_ip( if_list );
			errors += fail_if_nil( ip, "get_default_ip did not return valid pointer when list created from interface name" );
		} else {
			errors += fail_if_nil( if_list, "mk_ip_list with a specific interface name returned a nil list" );
		}

		free( ip );
	} else {
		fprintf( stderr, "<SKIP> test skipped because no interface with address could be found on system" );
	}

	errors += ztbf_test();					// test the zero term buffer fill function

// -------------------------------------------------------------------------------------------------

	return !!errors;			// 1 or 0 regardless of count
}

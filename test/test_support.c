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
	Mnemonic:	test_tools.c
	Abstract:	Functions for test applications to make their life a bit easier.
				This file is probably compiled to a .o, and then included on
				the cc command for the test.
	Author:		E. Scott Daniels
	Date:		6 January 2019
*/

#ifndef _test_support_c
#define _test_support_c

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef BAD
#define BAD 1			// these are exit codes unless user overrides
#define GOOD 0
#endif

/*
	Snag the optional positional parameter at pp, return defval if not there.
*/
static char* snag_pp( int pp, int argc, char** argv, char* defval ) {

	if( pp < argc ) {
		return argv[pp];
	}

	return defval;
}

/*
	Signal handler -- inside of the tests we will exit cleanly for hup/temp/intr
	signals so that the coverage stuff will generate the needed data files. If
	we inter/term the process they don't drive.
*/

void sig_clean_exit( int sign ) {
	fprintf( stderr, "signal trapped for clean exit: %d\n", sign );
	exit( 0 );
}

/*	
	Setup all of the signal handling for signals that we want to force a clean exit:
	term, intr, hup, quit, usr1/2 alarm, etc.  All others we'll let default.
*/
static void set_signals( void ) {
	struct sigaction sa;
	int	sig_list[] = { SIGINT, SIGQUIT, SIGILL, SIGALRM, SIGTERM, SIGUSR1 , SIGUSR2 };
	int i;
	int nele;		// number of elements in the list
	
	nele = (int) ( sizeof( sig_list )/sizeof( int ) );		// convert raw size to the number of elements
	for( i = 0; i < nele; i ++ ) {
		memset( &sa, 0, sizeof( sa ) );
		sa.sa_handler = sig_clean_exit;
		sigaction( sig_list[i], &sa, NULL );
	}
}


static int fail_if_nil( void* p, char* what ) {
	if( !p ) {
		fprintf( stderr, "<FAIL> %s: pointer was nil\n", what );
	}
	return p ? GOOD : BAD;
}

static int fail_not_nil( void* p, char* what ) {
	if( p ) {
		fprintf( stderr, "<FAIL> %s: pointer was not nil\n", what );
	}
	return !p ? GOOD : BAD;
}

static int fail_if_false( int bv, char* what ) {
	if( !bv ) {
		fprintf( stderr, "<FAIL> %s: expected true, boolean test was false (%d)\n", what, bv );
	}

	return bv ? GOOD : BAD;
}

static int fail_if_true( int bv, char* what ) {
	if( bv ) {
		fprintf( stderr, "<FAIL> %s: expected false, boolean test was true (%d)\n", what, bv );
	}
	return bv ? BAD : GOOD;
}

/*
	Same as fail_if_true(), but reads easier in the test code.
*/
static int fail_if( int bv, char* what ) {

	if( bv ) {
		fprintf( stderr, "<FAIL> %s: expected false, boolean test was true (%d)\n", what, bv );
	}
	return bv ? BAD : GOOD;
}

static int fail_not_equal( int a, int b, char* what ) {
	if( a != b ) {
		fprintf( stderr, "<FAIL> %s: values were not equal a=%d b=%d\n", what, a, b );
	}
	return a == b ? GOOD : BAD;			// user may override good/bad so do NOT return a==b directly!
}

static int fail_if_equal( int a, int b, char* what ) {
	if( a == b ) {
		fprintf( stderr, "<FAIL> %s values were equal a=%d b=%d\n", what, a, b );
	}
	return a != b ? GOOD : BAD;			// user may override good/bad so do NOT return a==b directly!
}


#ifndef NO_DUMMY_RMR
/*
	Dummy message allocator for testing without sr_static functions
*/
static rmr_mbuf_t* test_mk_msg( int len, int tr_len ) {
	rmr_mbuf_t*	new_msg;
	uta_mhdr_t* hdr;
	size_t	alen;

	alen = sizeof( *hdr ) + tr_len + len;

	new_msg = (rmr_mbuf_t *) malloc( sizeof *new_msg );
	new_msg->tp_buf = (void *) malloc( alen );
	memset( new_msg->tp_buf, 0, alen );

	hdr = (uta_mhdr_t*) new_msg->tp_buf;
	SET_HDR_LEN( hdr );
	SET_HDR_TR_LEN( hdr, tr_len );

	new_msg->header = new_msg->tp_buf;
	new_msg->payload =  new_msg->header + PAYLOAD_OFFSET( hdr );
	new_msg->alloc_len = alen;
	new_msg->len = 0;
	
	return new_msg;
}
#endif

#endif

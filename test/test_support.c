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

/*
	This is ugly, but needed to allow for component testing.

	The test code (e.g. foo_test.c and not foo_static_test.c) can include these
	constants to turn off the import of test support files:
		NO_EMULATION		-- the transport emulation will not be included
		NO_PRIVATE_HEADERS	-- the private headers for the transport component of RMR
								(e.g. si) will not be included.
*/
#ifndef NO_EMULATION				// assume emulation unless specifically put off (love double negatives)
	#ifdef NNG_UNDER_TEST
		#define TP_HDR_LEN 0				// needed for support functions but nonexistant in nng world
		#include "test_nng_em.c"			// nano/ngg emulation functions
	#else
		#include "test_si95_em.c"			// si emulation functions
	#endif
#endif

#ifndef NO_PRIVATE_HEADERS					// include transport headers unless specifically turned off
	#ifdef NNG_UNDER_TEST
		#include <rmr_nng_private.h>		// context things are type sensitive
	#else
		#include "si95/socket_if.h"			// need to have the si context more than anything else
		#include <rmr_si_private.h>
	#endif
#endif

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

static int fail_not_equalp( void* a, void* b, char* what ) {
	if( a != b ) {
		fprintf( stderr, "<FAIL> %s: pointers were not equal a=%p b=%p\n", what, a, b );
	}
	return a == b ? GOOD : BAD;			// user may override good/bad so do NOT return a==b directly!
}

static int fail_if_equalp( void* a, void* b, char* what ) {
	if( a == b ) {
		fprintf( stderr, "<FAIL> %s pointers were equal a=%p b=%p\n", what, a, b );
	}
	return a != b ? GOOD : BAD;			// user may override good/bad so do NOT return a==b directly!
}


// for symtab and other non-message things this allows them to exclude by setting
#ifndef NO_DUMMY_RMR
/*
	Dummy message allocator for testing without sr_static functions
*/
#ifndef MSG_VER
#define MSG_VER 3
#endif

//#ifndef TP_HDR_LEN
	//#define TP_HDR_LEN	0				// for nng there is no such beast
//#endif

static rmr_mbuf_t* test_mk_msg( int len, int tr_len ) {
	rmr_mbuf_t*	new_msg;
	uta_mhdr_t* hdr;
	size_t	alen;

	alen = sizeof( *hdr ) + tr_len + len + TP_HDR_LEN;	// this does no support allocating len2 and len3 data fields

	new_msg = (rmr_mbuf_t *) malloc( sizeof *new_msg );
	new_msg->tp_buf = (void *) malloc( alen );
	memset( new_msg->tp_buf, 0, alen );

	hdr = (uta_mhdr_t*) new_msg->tp_buf;
	SET_HDR_LEN( hdr );
	SET_HDR_TR_LEN( hdr, tr_len );
	hdr->rmr_ver = htonl( MSG_VER );
	strcpy( hdr->src, "dummyhost:1111" );
	strcpy( hdr->srcip, "30.4.19.86:1111" );

	new_msg->header = new_msg->tp_buf;
	new_msg->payload =  new_msg->header + PAYLOAD_OFFSET( hdr );
	new_msg->alloc_len = alen;
	new_msg->len = 0;

	return new_msg;
}

static void test_set_ver( rmr_mbuf_t* msg, int ver ) {
	uta_mhdr_t* hdr;

	hdr = (uta_mhdr_t*) msg->tp_buf;
	hdr->rmr_ver = htonl( ver );
	strcpy( hdr->src, "dummyhost-v2:1111" );
	strcpy( hdr->srcip, "30.4.19.86:2222" );

	return;
}

/*
	These allow values to be pushed deep into the real RMR header allocated
	at the front of the transport buffer. These are needed to simulate
	the actions of rmr_send() which pushes the values from the message buffer
	just before putting them on the wire.
*/
static void test_set_mtype( rmr_mbuf_t* msg, int mtype ) {
	uta_mhdr_t* hdr;

	msg->mtype = mtype;
	hdr = (uta_mhdr_t*) msg->tp_buf;
	hdr->mtype = htonl( mtype );
}

static void test_set_sid( rmr_mbuf_t* msg, int sid ) {
	uta_mhdr_t* hdr;

	msg->sub_id = sid;
	hdr = (uta_mhdr_t*) msg->tp_buf;
	hdr->sub_id = htonl( sid );
}

static void test_set_plen( rmr_mbuf_t* msg, int plen ) {
	uta_mhdr_t* hdr;

	msg->len = plen;
	hdr = (uta_mhdr_t*) msg->tp_buf;
	hdr->plen = htonl( plen );
}

/*
	Build a message and populate both the msg buffer and the tranport header
	with mid, sid, and payload len. Tr_len causes that much space in the 
	header for trace info to be reserved.
*/
static rmr_mbuf_t* mk_populated_msg( int alloc_len, int tr_len, int mtype, int sid, int plen ) {
	uta_mhdr_t* hdr;
	rmr_mbuf_t* mbuf;

	mbuf = test_mk_msg( alloc_len, tr_len );
	test_set_mtype( mbuf, mtype );
	test_set_sid( mbuf, sid );
	test_set_plen( mbuf, plen );

	return mbuf;
}


#endif

#endif

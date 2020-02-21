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
	Mnemonic:	mbuf_api_test.c
	Abstract:	Unit tests for the mbuf common API functions.
				To allow the mbuf functions to be tested without the bulk of the
				RMr mechanics, we dummy up a couple of functions that are in
				rmr[_nng].c.

	Author:		E. Scott Daniels
	Date:		2 April 2019
*/


#define NO_EMULATION

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>


#include "rmr.h"
#include "rmr_agnostic.h"

#include "mbuf_api.c"

#define MSG_VER 3
#include "test_support.c"						// our private library of test tools
#include "mbuf_api_static_test.c"				// test functions

// --- dummies -------------------------------------------------------------------

/*
	This will leak, but we're not here to test free.
*/
extern void rmr_free_msg( rmr_mbuf_t* mbuf ) {
	return;
}

/*
	Minimal buffer realloc to allow api to be tested without coverage hit if
	we actually pulled in the sr static set.

	WARNING:  this is NOT a complete realloc.  We assume that we are testing
			just the trace length adjustment portion of the set_trace()
			API and are not striving to test the real realloc function. That
			will be tested when the mbuf_api_static_test code is used by the
			more generic RMr test.  So, not all fields in the realloc'd buffer
			can be used.
*/
extern rmr_mbuf_t* rmr_realloc_msg( rmr_mbuf_t* msg, int new_tr_size ) {
	rmr_mbuf_t*	new_msg;
	uta_mhdr_t* hdr;

	new_msg = (rmr_mbuf_t *) malloc( sizeof *new_msg );
	new_msg->tp_buf = (void *) malloc( 2048 );
	memset( new_msg->tp_buf, 0, 2048 );
	hdr = (uta_mhdr_t*) new_msg->tp_buf;
	SET_HDR_LEN( hdr );
	SET_HDR_TR_LEN( hdr, new_tr_size );

	new_msg->payload = new_msg->tp_buf + new_tr_size;
	new_msg->header = new_msg->tp_buf;
	new_msg->alloc_len = 2048;
	new_msg->len = msg->len;

	return new_msg;
}

// --------------------------------------------------------------------------------

int main( ) {
	int errors = 0;

	errors += mbuf_api_test( );

	if( errors ) {
		fprintf( stderr, "<FAIL> mbuf_api tests failed\n" );
	} else {
		fprintf( stderr, "<OK>	 mbuf_api tests pass\n" );
	}

	return errors;
}

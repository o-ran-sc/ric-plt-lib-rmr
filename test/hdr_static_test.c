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
	Mmemonic:	hdr_static_test.c
	Abstract:	This tests specific properties of the message header

	Author:		E. Scott Daniels
	Date:		12 April 2019
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pipeline0/push.h>
#include <nng/protocol/pipeline0/pull.h>

#include "rmr.h"
#include "rmr_agnostic.h"
#include "rmr_nng_private.h"

#define EMULATE_NNG
#include "test_nng_em.c"

#include "symtab.c"
#include "mbuf_api.c"
#include <rtable_nng_static.c>
#include <sr_nng_static.c>
#include "rmr_nng.c"

#include "test_support.c"

static int hdr_test( ) {
	int errors = 0;
	uta_ctx_t*	ctx;
	rmr_mbuf_t*	msg;
	uta_mhdr_t*	hdr;
	int hlen;
	int len;
	int	payload_len = 2049;
	int trace_len = 37;

	ctx = (uta_ctx_t *) malloc( sizeof( *ctx ) );
	ctx->trace_data_len = 0;
	ctx->my_name = strdup( "my-dummy-host-name-and-port:xxxx" );
	ctx->my_ip = strdup( "192.168.98.99" );

	msg = alloc_zcmsg( ctx, NULL, payload_len, 0, 0 );				// header len here should just be len of our struct
	hdr = (uta_mhdr_t *) msg->header;
	hlen = RMR_HDR_LEN( hdr );

	fprintf( stderr, "<INFO> struct len= %d  msg len= %d %d\n", (int) sizeof( uta_mhdr_t ), hlen, htonl( hlen ) );
	errors += fail_not_equal( hlen, (int) sizeof( uta_mhdr_t ), "header len (a) not size of struct when no trace data is present" );

	len = (int) sizeof( uta_mhdr_t ) + payload_len;					// expected size of transport buffer allocated
	errors += fail_not_equal( len, msg->alloc_len, "alloc len (a)  not expected size" );


	ctx->trace_data_len = trace_len;		// alloc messages with tracing buffer in place
	msg = alloc_zcmsg( ctx, NULL, payload_len, 0, 0 );				// header len here should just be len of our struct
	hdr = (uta_mhdr_t *) msg->header;
	hlen = RMR_HDR_LEN( hdr );
	fprintf( stderr, "<INFO> with trace data: struct+trace len= %d  msg len= %d %d\n", (int) sizeof( uta_mhdr_t )+trace_len, hlen, htonl( hlen ) );
	errors += fail_not_equal( hlen, (int) sizeof( uta_mhdr_t ) + trace_len, "header len (a) was not header + trace data size (b)" );

	len = RMR_TR_LEN( hdr );
	errors += fail_not_equal( len, trace_len, "trace len in header (a) not expected value (b)" );

	len = RMR_D1_LEN( hdr );
	errors += fail_not_equal( len, 0, "d1 len in header (a) not expected value (b)" );

	len = RMR_D2_LEN( hdr );
	errors += fail_not_equal( len, 0, "d2 len in header (a) not expected value (b)" );


	// -------------------------------------------------------------------------------------------

	if( ! errors ) {
		fprintf( stderr, "<INFO> all msg header tests pass\n" );
	}
	return !! errors;
}

int main() {
	return hdr_test();
}

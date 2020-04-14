// vi: ts=4 sw=4 noet :
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
	Mnemonic:	test_msg_support.c
	Abstract:	Support for testing with messages; requires RMR defs, so not
				to be included for things like ring tests etc.
	Author:		E. Scott Daniels
	Date:		14 April 2020	(AKD)
*/

#ifndef _test_msg_support_c
#define _test_msg_support_c

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int test_support_xact_count = 0;
/*
	Send a 'burst' of messages to drive some send retry failures to increase RMr coverage
	by handling the retry caee.
*/
static void send_n_msgs( void* ctx, int n ) {
	rmr_mbuf_t*	msg;			// message buffers
	int i;

	msg = rmr_alloc_msg( ctx,  1024 );
	if( ! msg ) {
		fprintf( stderr, "<FAIL> mass send of %d messages couldn't allocate message!\n", n );
		return;
	}

	fprintf( stderr, "<INFO> mass send of %d messages\n", n );
	for( i = 0; i < n; i++ ) {
		msg->len = 100;
		msg->mtype = 1;
		msg->state = 999;
		
		snprintf( msg->xaction, 32, "%015d", test_support_xact_count++ );		// simple transaction id so we can test receive specific and ring stuff

		errno = 999;
		msg = rmr_send_msg( ctx, msg );
		if( msg && msg->state != 0 ) {
			fprintf( stderr, "<WARN> mass send failed: state=%d type=%d\n", msg->state, msg->mtype );
		}
	}
}


/*
	Allow test to reset the transaction id counter.
*/
static void reset_xact_count() {
	test_support_xact_count = 0;
}

/*
	Refresh or allocate a message with some default values
*/
static rmr_mbuf_t* fresh_msg( void* ctx, rmr_mbuf_t* msg ) {
	if( ! msg )  {
		msg = rmr_alloc_msg( ctx, 2048 );
	}

	msg->mtype = 0;
	msg->sub_id = -1;
	msg->state = 0;
	msg->len = 100;

	return msg;
}

#endif

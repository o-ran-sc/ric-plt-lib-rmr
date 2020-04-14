// : vi ts=4 sw=4 noet :
/*
==================================================================================
	    Copyright (c) 2020 Nokia
	    Copyright (c) 2020 AT&T Intellectual Property.

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
	Mmemonic:	rmr_si_lgb_static_test.c
	Abstract:	These are specific tests to exercise the ability to receive
				a buffer larger than what was specified as the normal max
				on the init call.

	Author:		E. Scott Daniels
	Date:		9 April 2020
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "rmr.h"
#include "rmr_agnostic.h"

static int rmr_lgbuf_test( ) {
	int		errors = 0;
	void*	rmc;				// route manager context
	rmr_mbuf_t*	msg;			// message buffers

	if( (rmc = rmr_init( "4560", 128, FL_NOTHREAD )) == NULL ) {		// use a short "normal max" value
		fail_if_nil( rmc, "rmr_init returned a nil pointer when trying to initialise a short normal max"  );
		return 1;
	}

	gen_rt( rmc );							// dummy route table so the send works
	msg = rmr_alloc_msg( rmc, 4024 );		// payload size, and the msg len must be larger than 128
	msg->len = 4000;
	msg->mtype = 1;
	msg->state = 999;
	snprintf( msg->payload, msg->len, "Rambling Wreck from Georgia Tech!\n\n" );
	msg = rmr_send_msg( rmc, msg );
	if( msg && msg->state != RMR_OK ) {
		fprintf( stderr, "<FAIL> lg buf test: send failed? %d\n", msg->state );
	}
	rmr_free_msg( msg );

	msg = rmr_torcv_msg( rmc, NULL, 2000 );
	errors += fail_if( msg == NULL, "rmr_rcv_msg returned nil msg when given nil msg "  );
	if( msg ) {
		errors += fail_not_equal( msg->state, RMR_OK, "receive given nil message did not return msg with good state"  );

		errors += fail_if_false( msg->len > 128, "expected larger message than the small buffer" );
	}

	return errors;
}

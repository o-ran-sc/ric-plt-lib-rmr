// : vi ts=4 sw=4 noet :
/*
==================================================================================
	Copyright (c) 2021 Nokia
	Copyright (c) 2021	AT&T Intellectual Property.

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
------------------------------------------------------------------------------
Mnemonic:	alarm.c
Abstract:	The RMR housekeeping thread (route table collection) is also responsible
			for sending a small number of alarm messages to the alarm system.
			To avoid extra overhead, the alarm library is NOT included; this
			module provides the alarm generation support for the housekeeping
			thread.

Date:		19 February 2021
			HBD-EKD
Author:		E. Scott Daniels

------------------------------------------------------------------------------
*/

#ifndef alarm_static_c
#define alarm_static_c

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>

#include <rmr.h>
#include <RIC_message_types.h>
#include <rmr_logging.h>


extern const char* __progname;			// runtime supplies (we have no acccess to argv[0]

/*
	Generate the endpoint for the arlarm system. We assume that it is defined
	by two environment variables: ALARM_MGR_SERVICE_NAME and ALARM_MGR_SERVICE_PORT.
	If the name is not given, we assume 127.0.0.1 (we don't use localhost for
	to avoid the double meaning within some flavours' hosts files). If the port
	is not given, we assume the default RMR port of 4560.

	The string returned must be freed by the caller.
*/
static char* uta_alarm_endpt( ) {
	char	wbuf[300];											// enough room for max name of 256 + :port
	const char* et;												// environment token
	const char* addr = "service-ricplt-alarmmanager-rmr";		// defaults
	const char* port = "4560";

    if( (et = getenv( ENV_AM_NAME )) != NULL ) {
        addr = et;
    }

    if( (et = getenv( ENV_AM_PORT )) != NULL ) {
        port = et;
    }

	wbuf[sizeof(wbuf)-1] = 0;				// guarentee it is nil terminated if too long
	snprintf( wbuf, sizeof( wbuf ) -1, "%s:%s", addr, port );
    return strdup( wbuf );
}

/*
	Send the message as an alarm. We will create a wormhole as we cannot depend
	on having a routing table that is valid. This send does NOT allocate a new
	message to return to the caller.
*/
static void uta_alarm_send( void* vctx, rmr_mbuf_t* msg ) {
	static int	whid = -1;
	static int	ok2log = 0;		// if we must log to stderr, do it seldomly; this is the next spot in time where logging is ok
	char*	ept;
	int		now;

	if( whid < 0 ) {
		ept = uta_alarm_endpt();
		if( (whid = rmr_wh_open( vctx, ept )) < 0 ) {
			if( (now = time( NULL )) >	ok2log ) {
				ok2log = now + 60;
			}
			rmr_vlog( RMR_VL_WARN, "unable to open wormhole for alarms: %s\n", ept );
			free( ept );
			return;
		}
		rmr_vlog( RMR_VL_INFO, "alarm wormhole opened to: %s\n", ept );
		free( ept );
	}

	if( msg != NULL ) {
		msg = rmr_wh_send_msg( vctx, whid, msg );
		rmr_free_msg( msg );
	}
}


/*
	Builds an alarm message for the kind of action. Alarm message is palced in
	the given buffer using the buffer length (blen) to truncate. A convenience
	pointer to the buffer is returned and will be nil on error.

	Kind is an ALARM_* constant ANDed with ALARM_RAISE or ALARM_CLEAR.
	Problem ID (prob_id) is an integer assigned by the caller and should
	match when calling to clear the alarm.

	Info is the message text that is added to the alarm; nil terminated string.
*/
static void uta_alarm( void* vctx, int kind, int prob_id, char* info ) {
	rmr_mbuf_t*	msg;
	int used;
	char*	maction = "CLEAR";

	if( (msg = rmr_alloc_msg( vctx, 2048 )) == NULL ) {
		return;
	}

	if( kind & ALARM_RAISE ) {
		maction = "RAISE";
	}

	if( info == NULL ) {
		info = "unspecific RMR issue";
	}

	used = snprintf( msg->payload, 2047,
		"{  "
		"\"managedObjectId\": null, "					// there is no object for rmr alarms
		"\"applicationId\": \"%s\", "
		"\"specificProblem\": %d, "						// problem id
		"\"perceivedSeverity\": \"WARNING\", "			// all RMR messages are warnings
		"\"identifyingInfo\": \"%s\", "
		"\"additionalInfo\": null, "
		"\"AlarmAction\": \"%s\", "
		"\"AlarmTime\": %lld"
		" }",

		__progname,										// see comment above
		prob_id,
		info,
		maction,										// raise/clear
		mstime()										// use logging module's timestamp generator
	);

	msg->len = strlen( msg->payload );
	msg->state = RMR_OK;
	msg->sub_id = -1;
	msg->mtype = RIC_ALARM;
	uta_alarm_send( vctx, msg );
}



#endif

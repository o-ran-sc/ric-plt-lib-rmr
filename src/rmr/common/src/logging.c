// :vi sw=4 ts=4 noet:
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
	Mnemonic:	logging.c
	Abstract:	Implements a common logging interface.
		
				Some thoughts and theory....
				Libraries should write human readable messages to standard error
				when errors, warnings, or general information messages are called
				for.  These messages should NEVER be written to standard output,
				and should never be mixed with an application's log messages which
				are assumed to contain potentially sensitive information, such as
				user names, and should not be written to standard error.

				For libraries such as RMR, where speed and latency are absolutely
				critical, logging should only be done from non-critical path code,
				or when an error has already compromised the ability to be performant.
				Thus, it is expected that most calls to the functions here are of
				the form:
					if( DEBUG ) rmr_logger( vlevel, fmt, parms);

				such that the call to rmr_logger() is removed from the code unless
				compiled with the DEBUG flag set.  Code such as
					if( vlevel >= WARNING ) {
						rmr_logger( fmt, parms )
					}

				is unacceptable because it adds unnecessary checking for the current
				verbosity level setting in line.

	
				With respect to formatting messages written to stderr: they should NOT
				be written with json, or anyother form of mark up.  Idealy, they should
				use a syntax similar to system log:
					<timestamp> <pid> <situation> <message>

				where timestamp is UNIX time, pid is the process id, situation is error,
				warning, etc, and message is obviously the text.

				Thus, the default messages geneated are "plain."  Because some might feel
				that messages need to be formatted, the code here provides for the gross
				encapsulation of standard error text into json (best wishes for the poor
				sap who is left having to read the output on some dark and stormy night).
				To enable this, the environment veriable RMR_HRLOG=0 must be set to
				turn off human readable logs.
				
	Author:		E. Scott Daniels
	Date:		27 January 2020
*/

#ifndef _logger_static_c
#define _logger_static_c

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#include <rmr.h>
#include <rmr_agnostic.h>
#include <rmr_logging.h>

static int log_initialised = 0;
static int log_vlevel = RMR_VL_ERR;
static int log_hrlogging = 1;
static int log_pid = 0;
static char* log_situations[RMR_VL_DEBUG+1];

/*
	Initialise logging. Returns the current log level.
*/
extern int rmr_vlog_init( ) {
	char*	data;

	if( (data = getenv( ENV_LOG_HR )) != NULL ) {
		log_hrlogging = atoi( data );
	}

	if( (data = getenv( ENV_LOG_VLEVEL )) != NULL ) {
		log_vlevel = atoi( data );
		if( log_vlevel < -1 ) {							// allow housekeeping stats to be squelched with -1
			log_vlevel = -1;
		} else {
			if( log_vlevel > RMR_VL_DEBUG ) {
				log_vlevel = RMR_VL_DEBUG;
			}
		}
	}

	log_pid = getpid();
	log_situations[RMR_VL_DEBUG] = LOG_DEBUG;
	log_situations[RMR_VL_INFO] = LOG_INFO;
	log_situations[RMR_VL_WARN] = LOG_WARN;
	log_situations[RMR_VL_ERR] = LOG_ERROR;
	log_situations[RMR_VL_CRIT] = LOG_CRIT;

	log_initialised = 1;

	return log_vlevel;
}

/*
	Write a variable message formatted in the same vein as *printf.
	We add a header to each log message with time, pid and message
	situation (error, warning, etc.).
*/
extern void rmr_vlog( int write_level, char* fmt, ... ) {
	va_list	argp;		// first arg in our parms which is variable
	char	msg[4096];
	int		hlen;
	char*	body;		// pointer into msg, past header

	if( write_level > log_vlevel ) { 			// write too big, skip
		return;
	}

	if( ! log_initialised ) {
		rmr_vlog_init();
	}

	if( write_level > RMR_VL_DEBUG || write_level < 0 ) {
		write_level = RMR_VL_DEBUG;
	}

	memset( msg, 0, sizeof( msg ) );								// logging is slow; this ensures 0 term if msg is too large
	hlen = snprintf( msg, sizeof( msg ), "%ld %d/RMR [%s] ", (long) time( NULL ), log_pid, log_situations[write_level] );
	if( hlen > sizeof( msg ) - 1024 ) {								// should never happen, but be parinoid
		return;
	}
	body = msg + hlen;

	va_start( argp, fmt );		// suss out parm past fmt

	vsnprintf( body, sizeof( msg ) - (hlen+2), fmt, argp );			// add in user message formatting it along the way
	fprintf( stderr, "%s", msg );									// we grew from printfs so all existing msg have \n; assume there

	va_end( argp );
}

/*
	This ensures that the message is written regardless of the current log level
	setting. This allows for route table collection verbose levels to be taken
	into consideration separately from the err/warn/debug messages.  Component
	verbosity would be better and should be implemented.
*/

extern void rmr_vlog_force( int write_level, char* fmt, ... ) {
	va_list	argp;		// first arg in our parms which is variable
	char	msg[4096];
	int		hlen;
	char*	body;		// pointer into msg, past header

	if( ! log_initialised ) {
		rmr_vlog_init();
	}

	if( log_vlevel <= 0 ) {			// can force if off completely to allow for total silience
		return;
	}

	if( write_level > RMR_VL_DEBUG || write_level < 0 ) {
		write_level = RMR_VL_DEBUG;
	}

	hlen = snprintf( msg, sizeof( msg ), "%ld %d/RMR [%s] ", (long) time( NULL ), log_pid, log_situations[write_level] );
	body = msg + hlen;

	va_start( argp, fmt );		// suss out parm past fmt

	vsnprintf( body, sizeof( msg ) - (hlen+2), fmt, argp );			// add in user message formatting it along the way
	fprintf( stderr, "%s", msg );									// we grew from printfs so all existing msg have \n; assume there

	va_end( argp );
}

// -------------------- public functions that are needed -----------------

/*
	Allow user control to logging level control. Accepts a new log level 
	from the user programme.  Messages which have a write level setting
	which is >= to the new level will be written.  Setting the new value
	to RMR_VL_OFF disables all logging (not advised).
*/
extern void rmr_set_vlevel( int new_level ) {
	if( new_level >= 0 ) {
		log_vlevel = new_level;
	}
}

#endif

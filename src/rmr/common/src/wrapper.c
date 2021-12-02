// :vi sw=4 ts=4 noet:
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
	Mnemonic:	wrapper.c
	Abstract:	Functions that may make life easier for wrappers written on top
				of the library which don't have access to header files.
	Author:		E. Scott Daniels
	Date:		3 May 2019
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/rmr.h"

#define ADD_SEP		1
#define	NO_SEP		0

/*
	Build a "name": value  sequence with an optional trailing
	seperator. Caller must free the buffer after use.
*/
static char* build_ival( char* name, int val, int add_sep ) {
	char	wbuf[512];

	snprintf( wbuf, sizeof( wbuf ), "\"%s\": %d%s", name, val, add_sep ? "," : "" );
	return strdup( wbuf );
}

/*
	Builds a "name": "string" sequence with an optional trailing
	separator. Caller must free resulting string.
*/
static char* build_sval( char* name, char* val, int add_sep ) {
	char	wbuf[512];

	snprintf( wbuf, sizeof( wbuf ), "\"%s\": \"%s\"%s", name, val, add_sep ? "," : "" );
	return strdup( wbuf );
}


/*
	Similar to strcat, bangs src onto the end of target, but UNLIKE
	strcat src is freed as a convenience. Max is the max amount
	that target can accept; we don't bang on if src len is
	larger than max.  Return is the size of src; 0 if the
	target was not modified.

	Source is ALWAYS freed!
*/
static int bang_on( char* target, char* src, int max ) {
	int		len;
	int		rc = 0;		// return code, assume error

	if( src && target ) {
		len = strlen( src );
		if( (rc = len <= max ? len : 0 ) > 0 ) {	// if it fits, add it.
			strncat( target, src, len );
		}
	}

	if( src ) {
		free( src );
	}
	return rc;
}

/*
	Frees the string that was allocated and returned using rmr_get_consts()
*/
extern void rmr_free_consts( char* p) {
	free(p);
}

/*
	Returns a set of json with the constants which are set in the header.
	Caller must free the returned string using rmr_free_consts()
*/
extern char* rmr_get_consts( ) {
	int		remain;				// bytes remaining in wbuf
	int		slen = 0;			// length added
	char	wbuf[2048];
	char*	phrase;

	snprintf( wbuf, sizeof( wbuf ), "{ " );
	remain = sizeof( wbuf ) - strlen( wbuf ) -10;	// reserve some bytes allowing us to close the json

	phrase = build_ival( "RMR_MAX_XID",       RMR_MAX_XID, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_MAX_SID",       RMR_MAX_SID, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_MAX_MEID",      RMR_MAX_MEID, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_MAX_SRC",       RMR_MAX_SRC, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_MAX_RCV_BYTES", RMR_MAX_RCV_BYTES, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );

	phrase = build_ival( "RMRFL_NONE",        RMRFL_NONE, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMRFL_AUTO_ALLOC",  RMRFL_AUTO_ALLOC, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMRFL_MTCALL",	  RMRFL_MTCALL, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );

	phrase = build_ival( "RMR_DEF_SIZE",      RMR_DEF_SIZE, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );

	phrase = build_ival( "RMR_VOID_MSGTYPE",  RMR_VOID_MSGTYPE, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_VOID_SUBID",    RMR_VOID_SUBID , ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );

	phrase = build_ival( "RMR_OK",            RMR_OK, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_BADARG",    RMR_ERR_BADARG, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_NOENDPT",   RMR_ERR_NOENDPT, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_EMPTY",     RMR_ERR_EMPTY, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_NOHDR",     RMR_ERR_NOHDR, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_SENDFAILED", RMR_ERR_SENDFAILED, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_CALLFAILED", RMR_ERR_CALLFAILED, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_NOWHOPEN",  RMR_ERR_NOWHOPEN, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_WHID",      RMR_ERR_WHID, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_OVERFLOW",  RMR_ERR_OVERFLOW, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_RETRY",     RMR_ERR_RETRY, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_RCVFAILED", RMR_ERR_RCVFAILED, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_TIMEOUT",   RMR_ERR_TIMEOUT, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_UNSET",     RMR_ERR_UNSET, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_TRUNC",     RMR_ERR_TRUNC, ADD_SEP );
	remain -= bang_on( wbuf, phrase, remain );
	phrase = build_ival( "RMR_ERR_INITFAILED", RMR_ERR_INITFAILED, NO_SEP );
	remain -= bang_on( wbuf, phrase, remain );

	strncat( wbuf, " }", remain );
	return strdup( wbuf );			// chop unused space and return
}

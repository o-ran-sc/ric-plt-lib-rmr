// : vi ts=4 sw=4 noet :
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
	Mnemonic:	rtc_si_static.c
	Abstract:	This is a test module to allow the route table to be read
				from a static spot and NOT to attempt to listen for updates
				from some outside source.

	Author:		E. Scott Daniels
	Date:		18 October 2019
*/

#ifndef _rtc_si_staic_c
#define _rtc_si_staic_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
	Loop forever (assuming we're running in a pthread reading the static table
	every minute or so.
*/
static void* rtc_file( void* vctx ) {
	uta_ctx_t*	ctx;					// context user has -- where we pin the route table
	char*	eptr;
	int		vfd = -1;					// verbose file des if we have one
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char	wbuf[256];


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		fprintf( stderr, "[CRI] rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	if( (eptr = getenv( ENV_VERBOSE_FILE )) != NULL ) {
		vfd = open( eptr, O_RDONLY );
	}

	while( 1 ) {
		if( vfd >= 0 ) {
			wbuf[0] = 0;
			lseek( vfd, 0, 0 );
			read( vfd, wbuf, 10 );
			vlevel = atoi( wbuf );
		}                
	
		read_static_rt( ctx, vlevel );						// seed the route table if one provided

		sleep( 60 );
	}

}
#endif

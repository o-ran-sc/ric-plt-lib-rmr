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
	Mnemonic:	mt_call_static.c
	Abstract:	Static multi-threaded call support.

	Author:		E. Scott Daniels
	Date:		17  May 2019
*/

#ifndef mtcall_static_c
#define mtcall_static_c

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
#include <netdb.h>
#include <time.h>

#include <semaphore.h>


/*
	Initialises the chutes that are then hung off of the context.

	Returns > 0 on success; 0 on failure with errno set.
*/
static int init_mtcall( uta_ctx_t*	ctx ) {
	int			rc = 1;			// return code 1== good.
	int			i;
	chute_t*	chutes;

	if( ctx == NULL ) {
		errno = EINVAL;
		return 0;
	}

	chutes = ctx->chutes = (chute_t *) malloc( sizeof( chute_t ) * (MAX_CALL_ID+1) );
	if( chutes == NULL ) {
		return 0;
	}

	for( i = 0; i < MAX_CALL_ID; i++ ) {				// initialise all of the semaphores
		chutes[i].mbuf = NULL;
		if( sem_init( &chutes[i].barrier, 0, 0 ) < 0 ) {
			rmr_vlog( RMR_VL_ERR, "rmr: unable to initialise mt call chute [%d]: %s\n", i, strerror( errno ) );
			rc = -1;
		}
	}	


	return rc;
}

#endif

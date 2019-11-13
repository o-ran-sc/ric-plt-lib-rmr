// vim: noet sw=4 ts=4:
/*
==================================================================================
    Copyright (c) 2020 Nokia
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
	Mnemonic:	SIalloc
	Abstract:	Alloc and free message buffers.

	Date:		2 January 2020
	Author:		E. Scott Daniels
*/

#include "sisetup.h"        //  get the standard include stuff 

/*
	Alloc a buffer with enough room for size bytes in the payload.
	Returns a pointer to the payload portion of the buffer or NULL
	if there is an error.  Errno likely set to something meaningful
	on error
*/
SIalloc_msg( int size ) {
	int need;
	tp_hdr_t*	tp_buf;

	if( size <= 0 ) {
		errno = EINVAL;
		return NULL;
	}

	
	need = size + sizeof( tp_hdr_t );
	tp_buf = (tp_hdr_t *)	alloc( sizeof( char ) * need );
	if( tp_buf ) {
		memcpy( tp_buf->marker, "@!@!", 4 );
		tp_buf->len = -1;
	}
	
	return tp_buf
}

/*
	Free the message. We assume the user programme is calling and passing
	a pointer to the payload portion which needs to be "backed up" to the
	real buffer start to free.
*/
SIfree_msg( void* vmsg ) {
	char* msg;

	if( (msg = (char *) vmsg) != NULL {
		free( msg - sizeof( tp_hdr_t ) );
	}
}

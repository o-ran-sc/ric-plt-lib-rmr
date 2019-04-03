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
	Mnemonic:	mbuf_api.c
	Abstract:	These are common functions which work only on the mbuf and 
				thus (because they do not touch an endpoint or context) 
				can be agnostic to the underlying transport.

	Author:		E. Scott Daniels
	Date:		8 February 2019
*/

#include <stdlib.h>
#include <netdb.h>			// uint* types
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "rmr.h"				// things the users see
#include "rmr_agnostic.h"		// agnostic things (must be included before private)


// ---------- some wrappers need explicit copy-in functions, also header field setters -----

/*
	Allow the user programme to set the meid in the header.  The meid is a fixed 
	length buffer in the header and thus we must ensure it is not overrun. If
	the  user gives a source buffer that is too large, we truncate. The return
	value is the number of bytes copied, or -1 for absolute failure (bad pointer
	etc.).  Errno is set:
		EINVAL id poitner, buf or buf header are bad.
		EOVERFLOW if the bytes given would have overrun
	
*/
extern int rmr_bytes2meid( rmr_mbuf_t* mbuf, unsigned char const* src, int len ) {
	uta_mhdr_t* hdr;

	if( src == NULL  ||  mbuf == NULL || mbuf->header == NULL ) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if( len > RMR_MAX_MEID ) {
		len = RMR_MAX_MEID;
		errno = EOVERFLOW;
	}

	hdr = (uta_mhdr_t *) mbuf->header;
	memcpy( hdr->meid, src, len );

	return len;
}

/*
	Allows the user programme to set the meid from a string. The end of string
	(nil) will be included UNLESS the total length including the end of string
	would exceed the size of the space in the header for the meid.  The return
	value is RMR_OK for success and !RMR_OK on failure. Errno will be set
	on error.
*/
extern int rmr_str2meid( rmr_mbuf_t* mbuf, unsigned char const* str ) {
	int len;		// len moved -- we do validate

	if( str == NULL  ||  mbuf == NULL || mbuf->header == NULL ) {
		errno = EINVAL;
		return RMR_ERR_BADARG;
	}

	errno = 0;
	if( (len = strlen( (char *) str )) > RMR_MAX_MEID-1 ) {
		errno = EOVERFLOW;
		return RMR_ERR_OVERFLOW;
	}
	
	rmr_bytes2meid( mbuf, str, len+1 );
	return RMR_OK;
}



/*
	This will copy n bytes from source into the payload. If len is larger than
	the payload only the bytes which will fit are copied, The user should 
	check errno on return to determine success or failure.
*/
extern void rmr_bytes2payload( rmr_mbuf_t* mbuf, unsigned char const* src, int len ) {
	if( src == NULL  ||  mbuf == NULL || mbuf->payload == NULL ) {
		errno = EINVAL;
		return;
	}

	errno = 0;
	mbuf->state = RMR_OK;
	if( len > mbuf->alloc_len - sizeof( uta_mhdr_t ) ) {
		mbuf->state = RMR_ERR_OVERFLOW;
		errno = EMSGSIZE;
		len = mbuf->alloc_len - sizeof( uta_mhdr_t );
	}

	mbuf->len = len;
	memcpy( mbuf->payload, src, len );
}

/*
	This will copy a nil terminated string to the mbuf payload. The buffer length
	is set to the string length.
*/
extern void rmr_str2payload( rmr_mbuf_t* mbuf, unsigned char const* str ) {
	rmr_bytes2payload( mbuf, str, strlen( (char *) str ) + 1 );
}


/*
	Allow the user programme to set the xaction field in the header.  The xaction
	is a fixed length buffer in the header and thus we must ensure it is not overrun.
	If the  user gives a source buffer that is too large, we truncate. The return
	value is the number of bytes copied, or -1 for absolute failure (bad pointer
	etc.).  Errno is set:
		EINVAL id poitner, buf or buf header are bad.
		EOVERFLOW if the bytes given would have overrun
	
*/
extern int rmr_bytes2xact( rmr_mbuf_t* mbuf, unsigned char const* src, int len ) {
	uta_mhdr_t* hdr;

	if( src == NULL  ||  mbuf == NULL || mbuf->header == NULL ) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if( len > RMR_MAX_XID ) {
		len = RMR_MAX_XID;
		errno = EOVERFLOW;
	}

	hdr = (uta_mhdr_t *) mbuf->header;
	memcpy( hdr->xid, src, len );

	return len;
}



/*
	Allows the user programme to set the xaction (xid) field from a string. The end
	of string (nil) will be included UNLESS the total length including the end of string
	would exceed the size of the space in the header for the xaction.  The return
	value is RMR_OK for success and !RMR_OK on failure. Errno will be set
	on error.
*/
extern int rmr_str2xact( rmr_mbuf_t* mbuf, unsigned char const* str ) {
	int len;		// len moved -- we do validate

	if( str == NULL  ||  mbuf == NULL || mbuf->header == NULL ) {
		errno = EINVAL;
		return RMR_ERR_BADARG;
	}

	errno = 0;
	if( (len = strlen( (char *) str )) > RMR_MAX_XID-1 ) {
		errno = EOVERFLOW;
		return RMR_ERR_OVERFLOW;
	}
	
	rmr_bytes2xact( mbuf, str, len+1 );
	return RMR_OK;
}

/*
	Extracts the meid (managed equipment) from the header and copies the bytes
	to the user supplied area. If the user supplied pointer is nil, then
	a buffer will be allocated and it is the user's responsibilty to free.
	A pointer is returned to the destination memory (allocated or not)
	for consistency. If the user programme supplies a destination it is
	the responsibility of the programme to ensure that the space is large
	enough.
*/
extern unsigned char*  rmr_get_meid( rmr_mbuf_t* mbuf, unsigned char* dest ) {
	uta_mhdr_t* hdr;

	if( mbuf == NULL || mbuf->header == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	if( ! dest ) {
		if( (dest = (unsigned char *) malloc( sizeof( unsigned char ) * RMR_MAX_MEID )) == NULL ) {
			errno = ENOMEM;
			return NULL;
		}
	}

	hdr = (uta_mhdr_t *) mbuf->header;
	memcpy( dest, hdr->meid, RMR_MAX_XID );

	return dest;
}

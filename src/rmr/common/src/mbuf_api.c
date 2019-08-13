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
#include <stdio.h>
#include <semaphore.h>

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
	Extracts the transaction bytes from the header and places into a
	user supplied buffer (dest). The buffer must be at least RMR_MAX_XID
	bytes in length as this function will copy the entire field (even if
	the sender saved a shorter "string." If the user supplies a nil poniter
	as the destination, a buffer is allocated; the caller must free when
	finished.  The buffer will be exactly RMR_MAX_XID bytes long.

	The return value is a pointer to the buffer that was filled (to
	the dest buffer provided, or the buffer we allocated). If there is an
	error, a nil pointer is returned, and errno should suggest the root
	cause of the issue (invalid message or no memory).
*/
extern unsigned char*  rmr_get_xact( rmr_mbuf_t* mbuf, unsigned char* dest ) {
	errno = 0;

	if( mbuf == NULL || mbuf->xaction == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	if( ! dest ) {
		if( (dest = (unsigned char *) malloc( sizeof( unsigned char ) * RMR_MAX_XID )) == NULL ) {
			errno = ENOMEM;
			return NULL;
		}
	}

	memcpy( dest, mbuf->xaction, RMR_MAX_XID );

	return dest;
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

	errno = 0;

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
	memcpy( dest, hdr->meid, RMR_MAX_MEID );

	return dest;
}

// ------------------- trace related access functions --------------------------------------
/*
	The set_trace function will copy the supplied data for size bytes into the
	header.  If the header trace area is not large enough, a new one will be allocated
	which will cause a payload copy based on the msg->len value (if 0 no payload
	data is copied).

	The return value is the number of bytes actually coppied. If 0 bytes are coppied
	errno should indicate the reason. If 0 is returned and errno is 0, then size
	passed was 0.  The state in the message is left UNCHANGED.
*/
extern int rmr_set_trace( rmr_mbuf_t* msg, unsigned const char* data, int size ) {
	uta_mhdr_t*	hdr;
	rmr_mbuf_t* nm;			// new message if larger is needed
	int		len;
	void*	old_tp_buf;		// if we need to realloc, must hold old to free
	void*	old_hdr;

	if( msg == NULL ) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;
	if( size <= 0 ) {
		return 0;
	}

	hdr = (uta_mhdr_t *) msg->header;
	if( !hdr ) {
		errno = EINVAL;
		return 0;
	}

	len = RMR_TR_LEN( hdr );

	if( len != size ) {							// different sized trace data, must realloc the buffer
		nm = rmr_realloc_msg( msg, size );		// realloc with changed trace size
		old_tp_buf = msg->tp_buf;
		old_hdr = msg->header;

		msg->tp_buf = nm->tp_buf;				// reference the reallocated buffer
		msg->header = nm->header;
		msg->id = NULL;							// currently unused
		msg->xaction = nm->xaction;
		msg->payload = nm->payload;

		nm->tp_buf = old_tp_buf;				// set to free
		nm->header = old_hdr;					// nano frees on hdr, so must set both
		rmr_free_msg( nm );

		hdr = (uta_mhdr_t *) msg->header;		// header WILL be different
		len = RMR_TR_LEN( hdr );
	}

	memcpy( TRACE_ADDR( hdr ), data, size );

	return size;
}


/*
	Returns a pointer (reference) to the trace data in the message. If sizeptr
	is supplied, then the trace data size is assigned to the referenced
	integer so that the caller doesn't need to make a second call to get the
	value.

	CAUTION:  user programmes should avoid writing to the referenced trace
			area (use rmr_set_trace() to avoid possible data overruns. This
			function is a convenience, and it is expected that users will
			use the trace data as read-only so as a copy is not necessary.
*/
extern void* rmr_trace_ref( rmr_mbuf_t* msg, int* sizeptr ) {
	uta_mhdr_t*	hdr = NULL;
	int size = 0;

	if( sizeptr != NULL ) {
		*sizeptr = 0;					// ensure reset if we bail
	}

	if( msg == NULL ) {
		return NULL;
	}

	hdr = msg->header;
	if( (size = RMR_TR_LEN( hdr )) <= 0 ) {
		return NULL;
	}

	if( sizeptr != NULL ) {
		*sizeptr = size;
	}

	return (void *) TRACE_ADDR( hdr );
}

/*
	Copies the trace bytes from the message header into the buffer provided by
	the user. If the trace data in the header is less than size, then only
	that number of bytes are copied, else exactly size bytes are copied. The
	number actually copied is returned.
*/
extern int rmr_get_trace( rmr_mbuf_t* msg, unsigned char* dest, int size ) {
	uta_mhdr_t*	hdr = NULL;
	int n2copy = 0;

	if( msg == NULL ) {
		return 0;
	}

	if( size <= 0 || dest == NULL ) {
		return 0;
	}

	hdr = msg->header;
	if( (n2copy = size < RMR_TR_LEN( hdr ) ? size : RMR_TR_LEN( hdr )) <= 0  ) {
		return 0;
	}

	memcpy( dest, TRACE_ADDR( hdr ), n2copy );

	return n2copy;
}

/*
	Returns the number of bytes currently allocated for trace data in the message
	buffer.
*/
extern int rmr_get_trlen( rmr_mbuf_t* msg ) {
	uta_mhdr_t*	hdr;

	if( msg == NULL ) {
		return 0;
	}

	hdr = msg->header;

	return RMR_TR_LEN( hdr );
}

/*
	Returns the string in the source portion of the header. This is assumed to be
	something that can be used for direct sends (hostname:port). Regardless, it
	will be a nil terminated, ascii string with max of 64 characters including
	the final nil. So, the user must ensure that dest is at least 64 bytes.

	As a convenience, the pointer to dest is returned on success; nil on failure
	with errno set.
*/
extern unsigned char* rmr_get_src( rmr_mbuf_t* msg, unsigned char* dest ) {
	uta_mhdr_t*	hdr = NULL;

	if( msg == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	if( dest != NULL ) {
		hdr = msg->header;
		strcpy( dest, hdr->src );
	}

	return dest;
}

/*
	Returns the string with the IP address as reported by the sender. This is
	the IP address that the sender has sussed off of one of the interfaces
	and cannot be guarenteed to be the acutal IP address which was used to
	establish the connection.   The caller must provide a buffer of at least
	64 bytes; the string will be nil terminated. A pointer to the user's buffer
	is returned on success, nil on failure.
*/
extern unsigned char* rmr_get_srcip( rmr_mbuf_t* msg, unsigned char* dest ) {
	uta_mhdr_t*	hdr = NULL;
	char*	rstr = NULL;

	errno = EINVAL;

	if( dest != NULL && msg != NULL ) {
		hdr = msg->header;
		if( HDR_VERSION( msg->header ) > 2 ) {		// src ip was not present in hdr until ver 3
			errno = 0;
			strcpy( dest, hdr->srcip );
			rstr = dest;
		} else  {
			errno = 0;
			strcpy( dest, hdr->src );				// reutrn the name:port for old messages
			rstr = dest;
		}
	}

	return rstr;
}

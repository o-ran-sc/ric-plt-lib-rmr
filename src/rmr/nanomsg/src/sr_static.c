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
	Mnemonic:	sr_static.c
	Abstract:	These are static send/receive related functions.

				(broken out of rmr.c)
	Author:		E. Scott Daniels
	Date:		13 February 2019
*/

#ifndef _sr_static_c
#define _sr_static_c

/*
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>

#include <nanomsg/nn.h>
#include <nanomsg/tcp.h>
#include <nanomsg/pair.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/pubsub.h>

#include "rmr.h"				// things the users see
#include "rmr_private.h"		// things that we need too
#include "rmr_symtab.h"

#include "ring_static.c"		// message ring support
#include "rt_generic_static.c"	// generic route table (not nng/nano specific)
#include "rtable_static.c"		// route table things	(nano specific)
#include "tools_static.c"
*/


/*
	Alloc a new nano zero copy buffer and put into msg. If msg is nil, then we will alloc
	a new message struct as well. Size is the size of the zc buffer to allocate (not
	including our header). If size is 0, then the buffer allocated is the size previously
	allocated (if msg is !nil) or the default size given at initialisation).


	The trlo parm is the trace length override which will be used if not 0. If 0, then the
	length in the context is used (default).
*/
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state, int trlo ) {
	int	mlen;
	uta_mhdr_t*	hdr;
	int tr_len;				// length to allocate for trace info

	tr_len = trlo > 0 ? trlo : ctx->trace_data_len;

	mlen = sizeof( uta_mhdr_t ) + tr_len + ctx->d1_len + ctx->d2_len;	// start with header and trace/data lengths
	mlen += (size > 0 ? size  : ctx->max_plen);							// add user requested size or size set during init

	if( msg == NULL ) {
		msg = (rmr_mbuf_t *) malloc( sizeof *msg );
		if( msg == NULL ) {
			fprintf( stderr, "[CRIT] rmr_alloc_zc: cannot get memory for message\n" );
			exit( 1 );
		}
	} else {
		mlen = msg->alloc_len;							// msg given, allocate the same size as before
	}

	memset( msg, 0, sizeof( *msg ) );

	if( (msg->header = (uta_mhdr_t *) nn_allocmsg( mlen, 0 )) == NULL ) {				// this will be released on send, so DO NOT free
		fprintf( stderr, "[CRIT] rmr_alloc_zc: cannot get memory for zero copy buffer: %d\n", errno );
		exit( 1 );
	}

	memset( msg->header, 0, sizeof( uta_mhdr_t ) );			// must ensure that header portion of tpbuf is 0
	msg->tp_buf = msg->header;
	hdr = (uta_mhdr_t *) msg->header;
	hdr->rmr_ver = htonl( RMR_MSG_VER );								// current version
	hdr->sub_id = htonl( UNSET_SUBID );
	SET_HDR_LEN( hdr );
	SET_HDR_TR_LEN( hdr, tr_len );							// set the actual length used
	//SET_HDR_D1_LEN( hdr, ctx->d1_len );					// moot until we actually need these data areas
	//SET_HDR_D2_LEN( hdr, ctx->d1_len );

	msg->len = 0;											// length of data in the payload
	msg->alloc_len = mlen;									// length of allocated payload
	msg->sub_id = UNSET_SUBID;
	msg->mtype = UNSET_MSGTYPE;
	msg->payload = PAYLOAD_ADDR( hdr );						// point at the payload in transport
	msg->xaction = ((uta_mhdr_t *)msg->header)->xid;		// point at transaction id in header area
	msg->state = state;										// fill in caller's state (likely the state of the last operation)
	msg->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SRC );
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->srcip, ctx->my_ip, RMR_MAX_SRC );

	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] alloc_zcmsg mlen = %d size=%d mpl=%d flags=%02x %p m=%p @%p\n", mlen, size, ctx->max_plen, msg->flags, &msg->flags, msg, msg->header );

	return msg;
}

/*
	This will clone a message into a new zero copy buffer and return the cloned message.
*/
static inline rmr_mbuf_t* clone_msg( rmr_mbuf_t* old_msg  ) {
	rmr_mbuf_t* nm;			// new message buffer
	int	mlen;

	if( old_msg == NULL ) {
		return NULL;
	}

	nm = (rmr_mbuf_t *) malloc( sizeof *nm );
	if( nm == NULL ) {
		fprintf( stderr, "[CRIT] rmr_clone: cannot get memory for message buffer\n" );
		exit( 1 );
	}
	memset( nm, 0, sizeof( *nm ) );

	mlen = old_msg->alloc_len;							// length allocated before
	if( (nm->header = (uta_mhdr_t *) nn_allocmsg( mlen, 0 )) == NULL ) {				// this will be released on send, so DO NOT free
		fprintf( stderr, "[CRIT] rmr_clone: cannot get memory for zero copy buffer: %d\n", errno );
		exit( 1 );
	}

	memcpy( nm->header, old_msg->header, RMR_HDR_LEN( old_msg->header ) );     // copy complete header, trace and other data

	nm->mtype = old_msg->mtype;
	nm->sub_id = old_msg->sub_id;
	nm->len = old_msg->len;									// length of data in the payload
	nm->alloc_len = mlen;									// length of allocated payload
	nm->payload = PAYLOAD_ADDR( nm->header );				// reference the payload
	nm->xaction = ((uta_mhdr_t *)nm->header)->xid;			// point at transaction id in header area
	nm->state = old_msg->state;								// fill in caller's state (likely the state of the last operation)
	nm->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	memcpy( nm->payload, old_msg->payload, old_msg->len );

	return nm;
}

static inline rmr_mbuf_t* realloc_msg( rmr_mbuf_t* old_msg, int tr_len  ) {
	rmr_mbuf_t* nm;			// new message buffer
	size_t	mlen;
	int state;
	uta_mhdr_t* hdr;
	uta_v1mhdr_t* v1hdr;
	int	tr_old_len;			// tr size in new buffer


	nm = (rmr_mbuf_t *) malloc( sizeof *nm );
	if( nm == NULL ) {
		fprintf( stderr, "[CRI] rmr_clone: cannot get memory for message buffer\n" );
		exit( 1 );
	}
	memset( nm, 0, sizeof( *nm ) );

	hdr = old_msg->header;
	tr_old_len = RMR_TR_LEN( hdr );				// bytes in old header for trace

	mlen = old_msg->alloc_len + (tr_len - tr_old_len);							// new length with trace adjustment
	if( DEBUG ) fprintf( stderr, "tr_realloc old size=%d new size=%d new tr_len=%d\n", (int) old_msg->alloc_len, (int) mlen, (int) tr_len );
	if( (nm->header = (uta_mhdr_t *) nn_allocmsg( mlen, 0 )) == NULL ) {				// this will be released on send, so DO NOT free
		fprintf( stderr, "[CRIT] rmr_realloc: cannot get memory for zero copy buffer: %d\n", errno );
		exit( 1 );
	}

	nm->tp_buf = nm->header;								// in nano both are the same
	v1hdr = (uta_v1mhdr_t *) old_msg->header;				// v1 will work to dig header out of any version
	switch( ntohl( v1hdr->rmr_ver ) ) {
		case 1:
			memcpy( v1hdr, old_msg->header, sizeof( *v1hdr ) );	 	// copy complete header
			nm->payload = (void *) v1hdr + sizeof( *v1hdr );
			break;

		default:											// current message always caught  here
			hdr = nm->header;
			memcpy( hdr, old_msg->header, sizeof( uta_mhdr_t ) );	 	// ONLY copy the header portion; trace and data offsets might have changed
			if( RMR_D1_LEN( hdr )  ) {
				memcpy( DATA1_ADDR( hdr ), DATA1_ADDR( old_msg->header ), RMR_D1_LEN( hdr ) );	 	// copy data1 and data2 if necessary

			}
			if( RMR_D2_LEN( hdr )  ) {
				memcpy( DATA2_ADDR( hdr ), DATA2_ADDR( old_msg->header ), RMR_D2_LEN( hdr ) );	 	// copy data1 and data2 if necessary
			}

			SET_HDR_TR_LEN( hdr, tr_len );										// len MUST be set before pointing payload
			nm->payload = PAYLOAD_ADDR( hdr );									// reference user payload
			break;
	}

	// --- these are all version agnostic -----------------------------------
	nm->mtype = old_msg->mtype;
	nm->sub_id = old_msg->sub_id;
	nm->len = old_msg->len;									// length of data in the payload
	nm->alloc_len = mlen;									// length of allocated payload

	nm->xaction = hdr->xid;									// reference xaction
	nm->state = old_msg->state;								// fill in caller's state (likely the state of the last operation)
	nm->flags = old_msg->flags | MFL_ZEROCOPY;				// this is a zerocopy sendable message
	memcpy( nm->payload, old_msg->payload, old_msg->len );

	return nm;
}

/*
	This is the receive work horse used by the outer layer receive functions.
	It waits for a message to be received on our listen socket. If old msg
	is passed in, the we assume we can use it instead of allocating a new
	one, else a new block of memory is allocated.

	This allocates a zero copy message so that if the user wishes to call
	uta_rts_msg() the send is zero copy.
*/
static rmr_mbuf_t* rcv_msg( uta_ctx_t* ctx, rmr_mbuf_t* old_msg ) {
	int nn_sock;				// endpoint socket for send
	int state;
	rmr_mbuf_t*	msg = NULL;		// msg received
	uta_mhdr_t* hdr;

	if( old_msg ) {
		msg = old_msg;
	} else {
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK, DEF_TR_LEN );			// will abort on failure, no need to check
	}

	msg->state = nn_recv( ctx->nn_sock, msg->header, msg->alloc_len, NO_FLAGS );		// total space (header + payload len) allocated
	if( msg->state > (int) sizeof( uta_mhdr_t ) ) {						// we need more than just a header here
		hdr = (uta_mhdr_t *) msg->header;
		msg->len = ntohl( hdr->plen );						// length of data in the payload (likely < payload size)
		if( msg->len > msg->state - RMR_HDR_LEN( hdr ) ) {
			msg->state = RMR_ERR_TRUNC;
			msg->len = msg->state - RMR_HDR_LEN( hdr );
		}
		msg->mtype = ntohl( hdr->mtype );								// capture and convert from network order to local order
		msg->sub_id = ntohl( hdr->sub_id );								// capture and convert from network order to local order
		msg->state = RMR_OK;
		msg->flags |= MFL_ADDSRC;										// turn on so if user app tries to send this buffer we reset src
		msg->payload = PAYLOAD_ADDR( msg->header );
		msg->xaction = &hdr->xid[0];							// provide user with ref to fixed space xaction id
		if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rcv_msg: got something: type=%d state=%d len=%d diff=%ld\n",
				msg->mtype, msg->state, msg->len,  msg->payload - (unsigned char *) msg->header );
	} else {
		msg->len = 0;
		msg->state = RMR_ERR_EMPTY;
	}

	return msg;
}


/*
	Receives a 'raw' message from a non-RMr sender (no header expected). The returned
	message buffer cannot be used to send, and the length information may or may
	not be correct (it is set to the length received which might be more than the
	bytes actually in the payload).
*/
static void* rcv_payload( uta_ctx_t* ctx, rmr_mbuf_t* old_msg ) {
	int nn_sock;				// endpoint socket for send
	int state;
	rmr_mbuf_t*	msg = NULL;		// msg received

	if( old_msg ) {
		msg = old_msg;
	} else {
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK, DEF_TR_LEN );			// will abort on failure, no need to check
	}

	msg->state = nn_recv( ctx->nn_sock, msg->header, msg->alloc_len, NO_FLAGS );		// read and state will be length
	if( msg->state >= 0 ) {
		msg->xaction = NULL;
		msg->mtype = UNSET_MSGTYPE;
		msg->sub_id = UNSET_SUBID;
		msg->len = msg->state;										// no header; len is the entire thing received
		msg->state = RMR_OK;
		msg->flags = MFL_RAW;										// prevent any sending of this headerless buffer
		msg->payload = msg->header;
		if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rcv_payload: got something: type=%d state=%d len=%d\n", msg->mtype, msg->state, msg->len );
	} else {
		msg->len = 0;
		msg->state = RMR_ERR_EMPTY;
		msg->payload = NULL;
		msg->xaction = NULL;
		msg->mtype = UNSET_MSGTYPE;
		msg->sub_id = UNSET_SUBID;
	}

	return msg;
}

/*
	This does the hard work of actually sending the message to the given socket. On success,
	a new message struct is returned. On error, the original msg is returned with the state
	set to a reasonable value. If the message being sent as MFL_NOALLOC set, then a new
	buffer will not be allocated and returned (mostly for call() interal processing since
	the return message from call() is a received buffer, not a new one).

	Called by rmr_send_msg() and rmr_rts_msg().
*/
static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int nn_sock ) {
	int state;
	uta_mhdr_t*	hdr;
	int	tr_len;					// length from the message being sent (must snarf before send to use after send)

	// future: ensure that application did not overrun the XID buffer; last byte must be 0

	//fprintf( stderr, ">>>>>> sending to %d %d\n", nn_sock, msg->mtype );
	hdr = (uta_mhdr_t *) msg->header;
	hdr->mtype = htonl( msg->mtype );								// stash type/len/sub-id in network byte order for transport
	hdr->sub_id = htonl( msg->sub_id );
	hdr->plen = htonl( msg->len );

	if( msg->flags & MFL_ADDSRC ) {									// buffer was allocated as a receive buffer; must add our source
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SRC );					// must overlay the source to be ours
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->srcip, ctx->my_ip, RMR_MAX_SRC );
	}

	tr_len = RMR_TR_LEN( hdr );
	if( msg->flags & MFL_ZEROCOPY ) {									// faster sending with zcopy buffer
		if( (state = nn_send( nn_sock, &msg->header, NN_MSG, NN_DONTWAIT )) < 0 ) {
			msg->state = state;
		} else {
			msg->header = NULL;											// nano frees; don't risk accessing later by mistake
		}
	} else {
		if( (state = nn_send( nn_sock, msg->header, sizeof( uta_mhdr_t ) + msg->len, NN_DONTWAIT )) < 0 ) {
			msg->state = state;
		}
	}

	// future:  if nano sends bytes, but less than mlen, then what to do?
	if( msg->state >= 0 ) {										// successful send
		if( !(msg->flags & MFL_NOALLOC) ) {						// if noalloc is set, then caller doesn't want a new buffer
			return alloc_zcmsg( ctx, msg, 0, RMR_OK, tr_len );	// preallocate a zero-copy buffer and return msg (with same trace len as sent buffer)
		} else {
			rmr_free_msg( msg );						// not wanting a meessage back, trash this one
			return NULL;
		}
	} else {											// send failed -- return original message
		if( errno == EAGAIN ) {
			msg->state = RMR_ERR_RETRY;					// some wrappers can't see errno, make this obvious
		} else {
			msg->state = RMR_ERR_SENDFAILED;					// errno will have nano reason
		}
		if( DEBUG ) fprintf( stderr, "[DBUG] send failed: %s\n", strerror( errno ) );
	}

	return msg;
}


/*
	A generic wrapper to the real send to keep wormhole stuff agnostic.
	We assume the wormhole function vetted the buffer so we don't have to.
*/
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg ) {
	return send_msg( ctx, msg, ep->nn_sock );
}

#endif

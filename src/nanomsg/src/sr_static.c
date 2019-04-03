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
*/
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state ) {
	int	mlen;

	mlen = sizeof( uta_mhdr_t );						// figure size should we not have a msg buffer
	mlen += (size > 0 ? size  : ctx->max_plen);			// add user requested size or size set during init

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

	((uta_mhdr_t *) msg->header)->rmr_ver = RMR_MSG_VER;	// version info should we need to recognised old style messages someday
	msg->len = 0;											// length of data in the payload
	msg->alloc_len = mlen;									// length of allocated payload
	msg->payload = msg->header + sizeof( uta_mhdr_t );		// point past header to payload (single buffer allocation above)
	msg->xaction = ((uta_mhdr_t *)msg->header)->xid;						// point at transaction id in header area
	msg->state = state;										// fill in caller's state (likely the state of the last operation)
	msg->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SID );

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

	nm->mtype = old_msg->mtype;
	nm->len = old_msg->len;									// length of data in the payload
	nm->alloc_len = mlen;									// length of allocated payload
	nm->payload = nm->header + sizeof( uta_mhdr_t );		// point past header to payload (single buffer allocation above)
	nm->xaction = ((uta_mhdr_t *)nm->header)->xid;			// point at transaction id in header area
	nm->state = old_msg->state;								// fill in caller's state (likely the state of the last operation)
	nm->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	memcpy( ((uta_mhdr_t *)nm->header)->src, ((uta_mhdr_t *)old_msg->header)->src, RMR_MAX_SID );
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
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK );			// will abort on failure, no need to check
	}

	msg->state = nn_recv( ctx->nn_sock, msg->header, msg->alloc_len, NO_FLAGS );		// total space (header + payload len) allocated
	if( msg->state > (int) sizeof( uta_mhdr_t ) ) {						// we need more than just a header here
		hdr = (uta_mhdr_t *) msg->header;
		msg->len = ntohl( hdr->plen );						// length of data in the payload (likely < payload size)
		if( msg->len > msg->state - sizeof( uta_mhdr_t ) ) {
			fprintf( stderr, "[WARN] rmr_rcv indicated payload length < rcvd payload: expected %d got %ld\n", 
				msg->len, msg->state - sizeof( uta_mhdr_t ) );
		}
		msg->mtype = ntohl( hdr->mtype );								// capture and convert from network order to local order
		msg->state = RMR_OK;
		msg->flags |= MFL_ADDSRC;										// turn on so if user app tries to send this buffer we reset src
		msg->payload = msg->header + sizeof( uta_mhdr_t );
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
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK );			// will abort on failure, no need to check
	}

	msg->state = nn_recv( ctx->nn_sock, msg->header, msg->alloc_len, NO_FLAGS );		// read and state will be length
	if( msg->state >= 0 ) {
		msg->xaction = NULL;
		msg->mtype = -1;
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
		msg->mtype = -1;
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

	// future: ensure that application did not overrun the XID buffer; last byte must be 0

	hdr = (uta_mhdr_t *) msg->header;
	hdr->mtype = htonl( msg->mtype );								// stash type/len in network byte order for transport
	hdr->plen = htonl( msg->len );

	if( msg->flags & MFL_ADDSRC ) {									// buffer was allocated as a receive buffer; must add our source
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SID );					// must overlay the source to be ours
	}

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
	if( msg->state >= 0 ) {								// successful send
		if( !(msg->flags & MFL_NOALLOC) ) {				// if noalloc is set, then caller doesn't want a new buffer
			return alloc_zcmsg( ctx, msg, 0, RMR_OK );	// preallocate a zero-copy buffer and return msg
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

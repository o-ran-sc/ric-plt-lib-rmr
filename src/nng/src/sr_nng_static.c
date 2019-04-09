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
	Mnemonic:	sr_nng_static.c
	Abstract:	These are static send/receive primatives which (sadly) 
				differ based on the underlying protocol (nng vs nanomsg).
				Split from rmr_nng.c  for easier wormhole support.

	Author:		E. Scott Daniels
	Date:		13 February 2019
*/

#ifndef _sr_nng_static_c
#define _sr_nng_static_c

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pipeline0/push.h>
#include <nng/protocol/pipeline0/pull.h>


/*
	Translates the nng state passed in to one of ours that is suitable to put
	into the message, and sets errno to something that might be useful.
	If we don't have a specific RMr state, then we return the default (e.g.
	receive failed).
*/
static inline int xlate_nng_state( int state, int def_state ) {

	switch( state ) {
		case 0:
			errno = 0;
			state = RMR_OK;
			break;

		case NNG_EAGAIN:				// soft errors get retry as the RMr error
			state = RMR_ERR_RETRY;
			errno = EAGAIN;
			break;
			
		case NNG_ETIMEDOUT:
			state = RMR_ERR_RETRY;
			errno = EAGAIN;
			break;

		case NNG_ENOTSUP:
			errno  = ENOTSUP;
			state = def_state;
			break;

		case NNG_EINVAL:
			errno  = EINVAL;
			state = def_state;
			break;

		case NNG_ENOMEM:
			errno  = ENOMEM;
			state = def_state;
			break;

		case NNG_ESTATE:
			errno  = EBADFD;				// file des not in a good state for the operation
			state = def_state;
			break;

		case NNG_ECLOSED:
			errno  = EBADFD;				// file des not in a good state for the operation
			state = def_state;
			break;
		
		default:
			errno = EBADE;
			state = def_state;
			break;
	}

	return state;
}

/*
	Alloc a new nano zero copy buffer and put into msg. If msg is nil, then we will alloc
	a new message struct as well. Size is the size of the zc buffer to allocate (not
	including our header). If size is 0, then the buffer allocated is the size previously
	allocated (if msg is !nil) or the default size given at initialisation).

	NOTE:  while accurate, the nng doc implies that both the msg buffer and data buffer
		are zero copy, however ONLY the message is zero copy. We now allocate and use
		nng messages.
*/
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state ) {
	size_t		mlen;
	uta_mhdr_t*	hdr;			// convenience pointer

	mlen = sizeof( uta_mhdr_t );						// figure size should we not have a msg buffer
	mlen += (size > 0 ? size  : ctx->max_plen);			// add user requested size or size set during init

	if( msg == NULL ) {
		msg = (rmr_mbuf_t *) malloc( sizeof *msg );
		if( msg == NULL ) {
			fprintf( stderr, "[CRI] rmr_alloc_zc: cannot get memory for message\n" );
			exit( 1 );
		}
	} else {
		mlen = msg->alloc_len;							// msg given, allocate the same size as before
	}

	memset( msg, 0, sizeof( *msg ) );

	if( (state = nng_msg_alloc( (nng_msg **) &msg->tp_buf, mlen )) != 0 ) {
		fprintf( stderr, "[CRI] rmr_alloc_zc: cannot get memory for zero copy buffer: %d\n", ENOMEM );
		abort( );											// toss out a core file for this
	}

	msg->header = nng_msg_body( msg->tp_buf );
	if( (hdr = (uta_mhdr_t *) msg->header) != NULL ) {
		hdr->rmr_ver = RMR_MSG_VER;								// version info should we need to recognised old style messages someday
	}
	msg->len = 0;											// length of data in the payload
	msg->alloc_len = mlen;									// length of allocated payload
	msg->payload = msg->header + sizeof( uta_mhdr_t );		// point past header to payload (single buffer allocation above)
	msg->xaction = ((uta_mhdr_t *)msg->header)->xid;		// point at transaction id in header area
	msg->state = state;										// fill in caller's state (likely the state of the last operation)
	msg->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SID );

	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] alloc_zcmsg mlen=%ld size=%d mpl=%d flags=%02x\n", (long) mlen, size, ctx->max_plen, msg->flags );

	return msg;
}

/*
	Allocates only the mbuf and does NOT allocate an underlying transport buffer since 
	NNG receive must allocate that on its own.
*/
static rmr_mbuf_t* alloc_mbuf( uta_ctx_t* ctx, int state ) {
	size_t	mlen;
	uta_mhdr_t* hdr;			// convenience pointer
	rmr_mbuf_t* msg;

	msg = (rmr_mbuf_t *) malloc( sizeof *msg );
	if( msg == NULL ) {
		fprintf( stderr, "[CRI] rmr_alloc_zc: cannot get memory for message\n" );
		exit( 1 );
	}

	memset( msg, 0, sizeof( *msg ) );

	msg->tp_buf = NULL;
	msg->header = NULL;
	msg->len = -1;											// no payload; invalid len
	msg->alloc_len = -1;
	msg->payload = NULL;
	msg->xaction = NULL;
	msg->state = RMR_ERR_UNSET;
	msg->flags = 0;

	return msg;
}

/*
	This accepts a message with the assumption that only the tp_buf pointer is valid. It
	sets all of the various header/payload/xaction pointers in the mbuf to the proper
	spot in the transport layer buffer.  The len in the header is assumed to be the 
	allocated len (a receive buffer that nng created);

	The alen parm is the assumed allocated length; assumed because it's a value likely
	to have come from nng receive and the actual alloc len might be larger, but we
	can only assume this is the total usable space.
*/
static void ref_tpbuf( rmr_mbuf_t* msg, size_t alen )  {
	uta_mhdr_t* hdr;

	msg->header = nng_msg_body( msg->tp_buf );				// header is the start of the transport buffer

	hdr = (uta_mhdr_t *) msg->header;
	hdr->rmr_ver = RMR_MSG_VER;								// version info should we need to recognised old style messages someday
	msg->len = ntohl( hdr->plen );							// length sender says is in the payload (received length could be larger)
	msg->alloc_len = alen;									// length of whole tp buffer (including header)
	msg->payload = msg->header + sizeof( uta_mhdr_t );		// point past header to payload (single buffer allocation above)
	msg->xaction = ((uta_mhdr_t *)msg->header)->xid;		// point at transaction id in header area
	msg->flags |= MFL_ZEROCOPY;								// this is a zerocopy sendable message
	msg->mtype = ntohl( hdr->mtype );								// capture and convert from network order to local order
	msg->state = RMR_OK;
}

/*
	This will clone a message into a new zero copy buffer and return the cloned message.
*/
static inline rmr_mbuf_t* clone_msg( rmr_mbuf_t* old_msg  ) {
	rmr_mbuf_t* nm;			// new message buffer
	size_t	mlen;
	int state;

	nm = (rmr_mbuf_t *) malloc( sizeof *nm );
	if( nm == NULL ) {
		fprintf( stderr, "[CRI] rmr_clone: cannot get memory for message buffer\n" );
		exit( 1 );
	}
	memset( nm, 0, sizeof( *nm ) );

	mlen = old_msg->alloc_len;										// length allocated before
	if( (state = nng_msg_alloc( (nng_msg **) &nm->tp_buf, mlen )) != 0 ) {
		fprintf( stderr, "[CRI] rmr_clone: cannot get memory for zero copy buffer: %d\n", ENOMEM );
		exit( 1 );
	}

	nm->header = nng_msg_body( nm->tp_buf );
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
	rmr_rts_msg() the send is zero copy.

	The nng timeout on send is at the ms level which is a tad too long for
	our needs.  So, if NNG returns eagain or timedout (we don't set one)
	we will loop up to 5 times with a 10 microsecond delay between each
	attempt.  If at the end of this set of retries NNG is still saying
	eagain/timeout we'll return to the caller with that set in errno.
	Right now this is only for zero-copy buffers (they should all be zc
	buffers now).


	In the NNG msg world it must allocate the receive buffer rather
	than accepting one that we allocated from their space and could 
	reuse.  They have their reasons I guess.  Thus, we will free
	the old transport buffer if user passes the message in; at least
	our mbuf will be reused. 
*/
static rmr_mbuf_t* rcv_msg( uta_ctx_t* ctx, rmr_mbuf_t* old_msg ) {
	int state;
	rmr_mbuf_t*	msg = NULL;		// msg received
	uta_mhdr_t* hdr;
	size_t	rsize;				// nng needs to write back the size received... grrr

	if( old_msg ) {
		msg = old_msg;
		if( msg->tp_buf != NULL ) {
			nng_msg_free( msg->tp_buf );
		}

		msg->tp_buf = NULL;
	} else {
		//msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK );			// will abort on failure, no need to check
		msg = alloc_mbuf( ctx, RMR_OK );				// msg without a transport buffer
	}

	msg->len = 0;
	msg->payload = NULL;
	msg->xaction = NULL;

	//rsize = msg->alloc_len;														// set to max, and we'll get len back here too
	//msg->state = nng_recv( ctx->nn_sock, msg->header, &rsize, NO_FLAGS );		// total space (header + payload len) allocated
	msg->state = nng_recvmsg( ctx->nn_sock, (nng_msg **) &msg->tp_buf, NO_FLAGS );			// blocks hard until received
	if( (msg->state = xlate_nng_state( msg->state, RMR_ERR_RCVFAILED )) != RMR_OK ) {
		return msg;
	}

	if( msg->tp_buf == NULL ) {		// if state is good this _should_ not be nil, but parninoia says check anyway
		msg->state = RMR_ERR_EMPTY;
		return msg;
	}

	rsize = nng_msg_len( msg->tp_buf );
	if( rsize >= sizeof( uta_mhdr_t ) ) {								// we need at least a full header here

		ref_tpbuf( msg, rsize );						// point payload, header etc to the just received tp buffer
		hdr = (uta_mhdr_t *) msg->header;
		msg->flags |= MFL_ADDSRC;				// turn on so if user app tries to send this buffer we reset src
		if( msg->len > (msg->alloc_len - sizeof( uta_mhdr_t )) ) {		// way more than we should have had room for; error
			msg->state = RMR_ERR_TRUNC;
		}

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

	Mostly this supports the route table collector, but could be extended with an 
	API external function.
*/
static void* rcv_payload( uta_ctx_t* ctx, rmr_mbuf_t* old_msg ) {
	int state;
	rmr_mbuf_t*	msg = NULL;		// msg received
	size_t	rsize;				// nng needs to write back the size received... grrr

	if( old_msg ) {
		msg = old_msg;
	} else {
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK );			// will abort on failure, no need to check
	}

	msg->state = nng_recvmsg( ctx->nn_sock, (nng_msg **) &msg->tp_buf, NO_FLAGS );			// blocks hard until received
	if( (msg->state = xlate_nng_state( msg->state, RMR_ERR_RCVFAILED )) != RMR_OK ) {
		return msg;
	}
	rsize = nng_msg_len( msg->tp_buf );

	// do NOT use ref_tpbuf() here! Must fill these in manually.
	msg->header = nng_msg_body( msg->tp_buf );
	msg->len = rsize;							// len is the number of bytes received
	msg->alloc_len = rsize;
	msg->mtype = -1;							// raw message has no type
	msg->state = RMR_OK;
	msg->flags = MFL_RAW;
	msg->payload = msg->header;					// payload is the whole thing; no header
	msg->xaction = NULL;

	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rcv_payload: got something: type=%d state=%d len=%d\n", msg->mtype, msg->state, msg->len );

	return msg;
}

/*
	This does the hard work of actually sending the message to the given socket. On success,
	a new message struct is returned. On error, the original msg is returned with the state 
	set to a reasonable value. If the message being sent as MFL_NOALLOC set, then a new 
	buffer will not be allocated and returned (mostly for call() interal processing since
	the return message from call() is a received buffer, not a new one).

	Called by rmr_send_msg() and rmr_rts_msg(), etc. and thus we assume that all pointer
	validation has been done prior.
*/
static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, nng_socket nn_sock, int retries ) {
	int state;
	uta_mhdr_t*	hdr;
	int nng_flags = NNG_FLAG_NONBLOCK;		// if we need to set any nng flags (zc buffer) add it to this
	int spin_retries = 1000;				// if eagain/timeout we'll spin this many times before giving up the CPU

	// future: ensure that application did not overrun the XID buffer; last byte must be 0

	hdr = (uta_mhdr_t *) msg->header;
	hdr->mtype = htonl( msg->mtype );								// stash type/len in network byte order for transport
	hdr->plen = htonl( msg->len );

	if( msg->flags & MFL_ADDSRC ) {									// buffer was allocated as a receive buffer; must add our source
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SID );					// must overlay the source to be ours
	}

	errno = 0;
	msg->state = RMR_OK;
	if( msg->flags & MFL_ZEROCOPY ) {									// faster sending with zcopy buffer
		//nng_flags |= NNG_FLAG_ALLOC;									// indicate a zc buffer that nng is expected to free

		do {
			if( (state = nng_sendmsg( nn_sock, (nng_msg *) msg->tp_buf, nng_flags )) != 0 ) {		// must check and retry some if transient failure
				msg->state = state;
				if( retries > 0 && (state == NNG_EAGAIN || state == NNG_ETIMEDOUT) ) {
					if( --spin_retries <= 0 ) {			// don't give up the processor if we don't have to
						retries--;
						usleep( 1 );					// sigh, give up the cpu and hope it's just 1 miscrosec
						spin_retries = 1000;
					}
				} else {
					state = 0;			// don't loop
					//if( DEBUG ) fprintf( stderr, ">>>>> send failed: %s\n", nng_strerror( state ) );
				}
			} else {
				state = 0;
				msg->state = RMR_OK;
				msg->header = NULL;											// nano frees; don't risk accessing later by mistake
				msg->tp_buf = NULL;
			}
		} while( state && retries > 0 );
	} else {
		msg->state = RMR_ERR_SENDFAILED;
		errno = ENOTSUP;
		return msg;
		/*
		NOT SUPPORTED
		if( (state = nng_send( nn_sock, msg->header, sizeof( uta_mhdr_t ) + msg->len, nng_flags )) != 0 ) {
			msg->state = state;
			//if( DEBUG ) fprintf( stderr, ">>>>> copy buffer send failed: %s\n", nng_strerror( state ) );
		} 
		*/
	}

	if( msg->state == RMR_OK ) {								// successful send
		if( !(msg->flags & MFL_NOALLOC) ) {				// allocate another sendable zc buffer unless told otherwise
			return alloc_zcmsg( ctx, msg, 0, RMR_OK );	// preallocate a zero-copy buffer and return msg
		} else {
			rmr_free_msg( msg );						// not wanting a meessage back, trash this one
			return NULL;
		}
	} else {											// send failed -- return original message
		if( msg->state == NNG_EAGAIN || msg->state == NNG_ETIMEDOUT ) {
			errno = EAGAIN;
			msg->state = RMR_ERR_RETRY;					// errno will have nano reason
		} else {
			msg->state = xlate_nng_state( msg->state, RMR_ERR_SENDFAILED );		// xlate to our state and set errno
			//errno = -msg->state;
			//msg->state = RMR_ERR_SENDFAILED;					// errno will have nano reason
		}

		if( DEBUG ) fprintf( stderr, "[DBUG] send failed: %d %s\n", (int) msg->state, strerror( msg->state ) );
	}

	return msg;
}

/*
	A generic wrapper to the real send to keep wormhole stuff agnostic.
	We assume the wormhole function vetted the buffer so we don't have to.
*/
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg ) {
	return send_msg( ctx, msg, ep->nn_sock, -1 );
}

#endif

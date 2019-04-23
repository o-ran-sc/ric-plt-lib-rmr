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
	Mnemonic:	rmr.c
	Abstract:	The bulk of the ric message routing library which is built upon
				the older nanomsg messaging transport mehhanism.

				To "hide" internal functions the choice was made to implement them
				all as static functions. This means that we include nearly
				all of our modules here as 90% of the library is not visible to
				the outside world.

	Author:		E. Scott Daniels
	Date:		28 November 2018
*/

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
#include "rmr_agnostic.h"		// headers agnostic to the underlying transport mechanism
#include "rmr_private.h"		// things that we need too
#include "rmr_symtab.h"

#include "ring_static.c"		// message ring support
#include "rt_generic_static.c"	// generic route table (not nng/nano specific)
#include "rtable_static.c"		// route table things	(nano specific)
#include "rtc_static.c"			// common rt collector
#include "tools_static.c"
#include "sr_static.c"			// send/receive static functions
#include "wormholes.c"			// external wormhole api, and it's static functions (must be LAST)

// ------------------------------------------------------------------------------------------------------

/*
	Clean up a context.
*/
static void free_ctx( uta_ctx_t* ctx ) {
	if( ctx ) {
		if( ctx->rtg_addr ) {
			free( ctx->rtg_addr );
		}
	}
}

// --------------- public functions --------------------------------------------------------------------------

/*
	Set the receive timeout to time (ms). A value of 0 is the same as a non-blocking
	receive and -1 is block for ever.
	Returns the nn value (0 on success <0 on error).
*/
extern int rmr_set_rtimeout( void* vctx, int time ) {
	uta_ctx_t* ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return -1;
	}

	if( ctx->last_rto == time ) {
		return 0;
	}

	ctx->last_rto = time;

	return nn_setsockopt( ctx->nn_sock, NN_SOL_SOCKET, NN_RCVTIMEO, &time, sizeof( time ) );
}

/*
	Deprecated -- use rmr_set_rtimeout()
*/
extern int rmr_rcv_to( void* vctx, int time ) {
	return rmr_rcv_to( vctx, time );
}

/*
	Set the send timeout to time. If time >1000 we assume the time is milliseconds,
	else we assume seconds. Setting -1 is always block.
	Returns the nn value (0 on success <0 on error).
*/
extern int rmr_set_stimeout( void* vctx, int time ) {
	uta_ctx_t* ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return -1;
	}

	if( time > 0 ) {
		if( time < 1000 ) {
			time = time * 1000;			// assume seconds, nn wants ms
		}
	}

	return nn_setsockopt( ctx->nn_sock, NN_SOL_SOCKET, NN_SNDTIMEO, &time, sizeof( time ) );
}

/*
	Deprecated -- use rmr_set_stimeout()
*/
extern int rmr_send_to( void* vctx, int time ) {
	return rmr_send_to( vctx, time );
}

/*
	Returns the size of the payload (bytes) that the msg buffer references.
	Len in a message is the number of bytes which were received, or should
	be transmitted, however, it is possible that the mbuf was allocated
	with a larger payload space than the payload length indicates; this
	function returns the absolute maximum space that the user has available
	in the payload. On error (bad msg buffer) -1 is returned and errno should
	indicate the rason.
*/
extern int rmr_payload_size( rmr_mbuf_t* msg ) {
	if( msg == NULL || msg->header == NULL ) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	return msg->alloc_len - RMR_HDR_LEN( msg->header );			// transport buffer less header and other data bits
}

/*
	Allocates a send message as a zerocopy message allowing the underlying message protocol
	to send the buffer without copy.
*/
extern rmr_mbuf_t* rmr_alloc_msg( void* vctx, int size ) {
	uta_ctx_t*	ctx;
	rmr_mbuf_t*	m;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return NULL;
	}

	m = alloc_zcmsg( ctx, NULL, size, 0, DEF_TR_LEN );
	return  m;
}

/*
	Allocates a send message as a zerocopy message allowing the underlying message protocol
	to send the buffer without copy. In addition, a trace data field of tr_size will be
	added and the supplied data coppied to the buffer before returning the message to
	the caller.
*/
extern rmr_mbuf_t* rmr_tralloc_msg( void* vctx, int size, int tr_size, unsigned const char* data ) {
	uta_ctx_t*	ctx;
	rmr_mbuf_t*	m;
	int state;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return NULL;
	}

	m = alloc_zcmsg( ctx, NULL, size, 0, tr_size );				// alloc with specific tr size
	if( m != NULL ) {
		state = rmr_set_trace( m, data, tr_size );				// roll their data in
		if( state != tr_size ) {
			m->state = RMR_ERR_INITFAILED;
		}
	}

	return  m;
}

/*
	Need an external path to the realloc static function as it's called by an
	outward facing mbuf api function.
*/
extern rmr_mbuf_t* rmr_realloc_msg( rmr_mbuf_t* msg, int new_tr_size ) {
	return realloc_msg( msg, new_tr_size );
}

/*
	Return the message to the available pool, or free it outright.
*/
extern void rmr_free_msg( rmr_mbuf_t* mbuf ) {
	if( mbuf == NULL ) {
		return;
	}

	if( mbuf->header ) {
		if( mbuf->flags & MFL_ZEROCOPY ) {
			nn_freemsg( mbuf->header );				// must let nano free it
		} else {
			free( mbuf->header );
		}
	}

	free( mbuf );
}

/*
	Accept a message and send it to an endpoint based on message type.
	Allocates a new message buffer for the next send. If a message type has
	more than one group of endpoints defined, then the message will be sent
	in round robin fashion to one endpoint in each group.

	CAUTION: this is a non-blocking send.  If the message cannot be sent, then
		it will return with an error and errno set to eagain. If the send is
		a limited fanout, then the returned status is the status of the last
		send attempt.
*/
extern rmr_mbuf_t* rmr_send_msg( void* vctx, rmr_mbuf_t* msg ) {
	int nn_sock;				// endpoint socket for send
	uta_ctx_t*	ctx;
	int	group;					// selected group to get socket for
	int send_again;				// true if the message must be sent again
	rmr_mbuf_t*	clone_m;		// cloned message for an nth send

	if( (ctx = (uta_ctx_t *) vctx) == NULL || msg == NULL ) {		// bad stuff, bail fast
		errno = EINVAL;												// if msg is null, this is their clue
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
			errno = EINVAL;											// must ensure it's not eagain
		}
		return msg;
	}

	errno = 0;													// clear; nano might set, but ensure it's not left over if it doesn't
	if( msg->header == NULL ) {
		fprintf( stderr, "[ERR] rmr_send_msg: message had no header\n" );
		msg->state = RMR_ERR_NOHDR;
		errno = EBADMSG;										// must ensure it's not eagain
		return msg;
	}

	send_again = 1;											// force loop entry
	group = 0;												// always start with group 0

	while( send_again ) {
		nn_sock = uta_epsock_rr( ctx->rtable, msg->mtype, group, &send_again );		// round robin select endpoint; again set if mult groups
		if( DEBUG ) fprintf( stderr, "[DBUG] send msg: type=%d again=%d group=%d socket=%d len=%d\n",
				msg->mtype, send_again, group, nn_sock, msg->len );
		group++;

		if( nn_sock < 0 ) {
			msg->state = RMR_ERR_NOENDPT;
			errno = ENXIO;											// must ensure it's not eagain
			return msg;												// caller can resend (maybe) or free
		}

		if( send_again ) {
			clone_m = clone_msg( msg );								// must make a copy as once we send this message is not available
			if( DEBUG ) fprintf( stderr, "[DBUG] msg cloned: type=%d len=%d\n", msg->mtype, msg->len );
			msg->flags |= MFL_NOALLOC;								// send should not allocate a new buffer
			msg = send_msg( ctx, msg, nn_sock );					// do the hard work, msg should be nil on success
			/*
			if( msg ) {
				// error do we need to count successes/errors, how to report some success, esp if last fails?
			}
			*/

			msg = clone_m;											// clone will be the next to send
		} else {
			msg = send_msg( ctx, msg, nn_sock );					// send the last, and allocate a new buffer; drops the clone if it was
		}
	}

	return msg;									// last message caries the status of last/only send attempt
}

/*
	Return to sender allows a message to be sent back to the endpoint where it originated.
	The source information in the message is used to select the socket on which to write
	the message rather than using the message type and round-robin selection. This
	should return a message buffer with the state of the send operation set. On success
	(state is RMR_OK, the caller may use the buffer for another receive operation), and on
	error it can be passed back to this function to retry the send if desired. On error,
	errno will liklely have the failure reason set by the nanomsg send processing.
	The following are possible values for the state in the message buffer:

	Message states returned:
		RMR_ERR_BADARG - argument (context or msg) was nil or invalid
		RMR_ERR_NOHDR  - message did not have a header
		RMR_ERR_NOENDPT- an endpoint to send the message to could not be determined
		RMR_ERR_SENDFAILED - send failed; errno has nano error code
		RMR_ERR_RETRY	- operation failed, but caller should retry

	A nil message as the return value is rare, and generally indicates some kind of horrible
	failure. The value of errno might give a clue as to what is wrong.

	CAUTION:
		Like send_msg(), this is non-blocking and will return the msg if there is an errror.
		The caller must check for this and handle.
*/
extern rmr_mbuf_t*  rmr_rts_msg( void* vctx, rmr_mbuf_t* msg ) {
	int nn_sock;				// endpoint socket for send
	uta_ctx_t*	ctx;
	int state;
	uta_mhdr_t*	hdr;
	char*	hold_src;			// we need the original source if send fails

	if( (ctx = (uta_ctx_t *) vctx) == NULL || msg == NULL ) {		// bad stuff, bail fast
		errno = EINVAL;												// if msg is null, this is their clue
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
		}
		return msg;
	}

	errno = 0;														// at this point any bad state is in msg returned
	if( msg->header == NULL ) {
		fprintf( stderr, "rmr_send_msg: ERROR: message had no header\n" );
		msg->state = RMR_ERR_NOHDR;
		return msg;
	}

	nn_sock = uta_epsock_byname( ctx->rtable, (char *) ((uta_mhdr_t *)msg->header)->src );			// socket of specific endpoint
	if( nn_sock < 0 ) {
		msg->state = RMR_ERR_NOENDPT;
		return msg;							// preallocated msg can be reused since not given back to nn
	}

	hold_src = strdup( (char *) ((uta_mhdr_t *)msg->header)->src );							// the dest where we're returning the message to
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SID );		// must overlay the source to be ours
	msg = send_msg( ctx, msg, nn_sock );
	if( msg ) {
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, hold_src, RMR_MAX_SID );		// always return original source so rts can be called again
		msg->flags |= MFL_ADDSRC;													// if msg given to send() it must add source
	}

	free( hold_src );
	return msg;
}

/*
	Call sends the message based on message routing using the message type, and waits for a
	response message to arrive with the same transaction id that was in the outgoing message.
	If, while wiating for the expected response,  messages are received which do not have the
	desired transaction ID, they are queued. Calls to uta_rcv_msg() will dequeue them in the
	order that they were received.

	Normally, a message struct pointer is returned and msg->state must be checked for RMR_OK
	to ensure that no error was encountered. If the state is UTA_BADARG, then the message
	may be resent (likely the context pointer was nil).  If the message is sent, but no
	response is received, a nil message is returned with errno set to indicate the likley
	issue:
		ETIMEDOUT -- too many messages were queued before reciving the expected response
		ENOBUFS -- the queued message ring is full, messages were dropped
		EINVAL  -- A parameter was not valid
		EAGAIN	-- the underlying message system wsa interrupted or the device was busy;
					user should call this function with the message again.


	QUESTION:  should user specify the number of messages to allow to queue?
*/
extern rmr_mbuf_t* rmr_call( void* vctx, rmr_mbuf_t* msg ) {
	uta_ctx_t*		ctx;
	unsigned char	expected_id[RMR_MAX_XID+1];		// the transaction id in the message; we wait for response with same ID

	if( (ctx = (uta_ctx_t *) vctx) == NULL || msg == NULL ) {		// bad stuff, bail fast
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
		}
		return msg;
	}

	memcpy( expected_id, msg->xaction, RMR_MAX_XID );
	expected_id[RMR_MAX_XID] = 0;					// ensure it's a string
	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rmr_call is making call, waiting for (%s)\n", expected_id );
	errno = 0;
	msg->flags |= MFL_NOALLOC;						// we don't need a new buffer from send

	msg = rmr_send_msg( ctx, msg );
	if( msg ) {										// msg should be nil, if not there was a problem; return buffer to user
		if( msg->state != RMR_ERR_RETRY ) {
			msg->state = RMR_ERR_CALLFAILED;		// don't stomp if send_msg set retry
		}
		return msg;
	}

	return rmr_rcv_specific( ctx, NULL, (char *) expected_id, 20 ); 		// wait for msg allowing 20 to queue ahead
}

/*
	The outward facing receive function. When invoked it will pop the oldest message
	from the receive ring, if any are queued, and return it. If the ring is empty
	then the receive function is invoked to wait for the next message to arrive (blocking).

	If old_msg is provided, it will be populated (avoiding lots of free/alloc cycles). If
	nil, a new one will be allocated. However, the caller should NOT expect to get the same
	struct back (if a queued message is returned the message struct will be different).
*/
extern rmr_mbuf_t* rmr_rcv_msg( void* vctx, rmr_mbuf_t* old_msg ) {
	uta_ctx_t*	ctx;
	rmr_mbuf_t*	qm;				// message that was queued on the ring

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		if( old_msg != NULL ) {
			old_msg->state = RMR_ERR_BADARG;
		}
		errno = EINVAL;
		return old_msg;
	}
	errno = 0;

	qm = (rmr_mbuf_t *) uta_ring_extract( ctx->mring );			// pop if queued
	if( qm != NULL ) {
		if( old_msg ) {
			rmr_free_msg( old_msg ); 							// future:  push onto a free list???
		}

		return qm;
	}

	return rcv_msg( ctx, old_msg );								// nothing queued, wait for one
}

/*
	Receive with a timeout.  This is a convenience function when sitting on top of
	nanomsg as it just sets the rcv timeout and calls rmr_rcv_msg().
*/
extern rmr_mbuf_t* rmr_torcv_msg( void* vctx, rmr_mbuf_t* old_msg, int ms_to ) {
	uta_ctx_t*	ctx;

	if( (ctx = (uta_ctx_t *) vctx) != NULL ) {
		if( ctx->last_rto != ms_to ) {							// avoid call overhead
			rmr_set_rtimeout( vctx, ms_to );
		}
	}

	return rmr_rcv_msg( vctx, old_msg );
}


/*
	This blocks until the message with the 'expect' ID is received. Messages which are received
	before the expected message are queued onto the message ring.  The function will return
	a nil message and set errno to ETIMEDOUT if allow2queue messages are received before the
	expected message is received. If the queued message ring fills a nil pointer is returned
	and errno is set to ENOBUFS.

	Generally this will be invoked only by the call() function as it waits for a response, but
	it is exposed to the user application as three is no reason not to.
*/
extern rmr_mbuf_t* rmr_rcv_specific( void* vctx, rmr_mbuf_t* msg, char* expect, int allow2queue ) {
	uta_ctx_t*	ctx;
	int	queued = 0;				// number we pushed into the ring
	int	exp_len = 0;			// length of expected ID

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
		}
		errno = EINVAL;
		return msg;
	}

	errno = 0;

	if( expect == NULL || ! *expect ) {				// nothing expected if nil or empty string, just receive
		return rmr_rcv_msg( ctx, msg );
	}

	exp_len = strlen( expect );
	if( exp_len > RMR_MAX_XID ) {
		exp_len = RMR_MAX_XID;
	}
	if( DEBUG ) fprintf( stderr, "[DBUG] rcv_specific waiting for id=%s\n",  expect );

	while( queued < allow2queue ) {
		msg = rcv_msg( ctx, msg );					// hard wait for next
		if( msg->state == RMR_OK ) {
			if( memcmp( msg->xaction, expect, exp_len ) == 0 ) {			// got it -- return it
				if( DEBUG ) fprintf( stderr, "[DBUG] rcv-specific matched (%s); %d messages were queued\n", msg->xaction, queued );
				return msg;
			}

			if( ! uta_ring_insert( ctx->mring, msg ) ) {					// just queue, error if ring is full
				if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rcv_specific ring is full\n" );
				errno = ENOBUFS;
				return NULL;
			}

			if( DEBUG ) fprintf( stderr, "[DBUG] rcv_specific queued message type=%d\n", msg->mtype );
			queued++;
			msg = NULL;
		}
	}

	if( DEBUG ) fprintf( stderr, "[DBUG] rcv_specific timeout waiting for %s\n", expect );
	errno = ETIMEDOUT;
	return NULL;
}


/*
	Initialise the message routing environment. Flags are one of the UTAFL_
	constants. Proto_port is a protocol:port string (e.g. tcp:1234). If default protocol
	(tcp) to be used, then :port is all that is needed.

	At the moment it seems that TCP really is the only viable protocol, but
	we'll allow flexibility.

	The return value is a void pointer which must be passed to most uta functions. On
	error, a nil pointer is returned and errno should be set.
*/
static void* init( char* uproto_port, int max_msg_size, int flags ) {
	uta_ctx_t*	ctx = NULL;
	char	bind_info[NN_SOCKADDR_MAX];	// bind info
	char*	proto = "tcp";				// pointer into the proto/port string user supplied
	char*	port;
	char*	proto_port;
	char	wbuf[1024];					// work buffer
	char*	tok;						// pointer at token in a buffer
	int		state;
	char*	interface = NULL;			// interface to bind to pulled from RMR_BIND_IF if set

	fprintf( stderr, "[INFO] ric message routing library on nanomsg (%s %s.%s.%s built: %s)\n",
			QUOTE_DEF(GIT_ID), QUOTE_DEF(MAJOR_VER), QUOTE_DEF(MINOR_VER), QUOTE_DEF(PATCH_VER), __DATE__ );

	errno = 0;
	if( uproto_port == NULL ) {
		proto_port = strdup( "tcp:4567" );
	} else {
		proto_port = strdup( uproto_port );		// so we can modify it
	}

	if( (ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) )) == NULL ) {
		errno = ENOMEM;
		return NULL;
	}
	memset( ctx, 0, sizeof( uta_ctx_t ) );


	ctx->mring = uta_mk_ring( 128 );				// message ring to hold asynch msgs received while waiting for call response
	ctx->last_rto = -2;								// last receive timeout that was set; invalid value to force first to set

	ctx->max_plen = RMR_MAX_RCV_BYTES + sizeof( uta_mhdr_t );		// default max buffer size
	if( max_msg_size > 0 ) {
		if( max_msg_size <= ctx->max_plen ) {						// user defined len can be smaller
			ctx->max_plen = max_msg_size;
		} else {
			fprintf( stderr, "[WARN] rmr_init: attempt to set max payload len > than allowed maximum; capped at %d bytes\n", ctx->max_plen );
		}
	}

	ctx->max_mlen = ctx->max_plen + sizeof( uta_mhdr_t );

	uta_lookup_rtg( ctx );							// attempt to fill in rtg info; rtc will handle missing values/errors

	ctx->nn_sock = nn_socket( AF_SP, NN_PULL );		// our 'listen' socket should allow multiple senders to connect
	if( ctx->nn_sock < 0 ) {
		fprintf( stderr, "[CRIT] rmr_init: unable to initialise nanomsg listen socket: %d\n", errno );
		free_ctx( ctx );
		return NULL;
	}

	if( (port = strchr( proto_port, ':' )) != NULL ) {
		if( port == proto_port ) {		// ":1234" supplied; leave proto to default and point port correctly
			port++;
		} else {
			*(port++) = 0;			// term proto string and point at port string
			proto = proto_port;		// user supplied proto so point at it rather than default
		}
	} else {
		port = proto_port;			// assume something like "1234" was passed
	}

	if( (gethostname( wbuf, sizeof( wbuf ) )) < 0 ) {
		fprintf( stderr, "[CRIT] rmr_init: cannot determine localhost name: %s\n", strerror( errno ) );
		return NULL;
	}
	if( (tok = strchr( wbuf, '.' )) != NULL ) {
		*tok = 0;									// we don't keep domain portion
	}
	ctx->my_name = (char *) malloc( sizeof( char ) * RMR_MAX_SID );
	if( snprintf( ctx->my_name, RMR_MAX_SID, "%s:%s", wbuf, port ) >= RMR_MAX_SID ) {			// our registered name is host:port
		fprintf( stderr, "[CRIT] rmr_init: hostname + port must be less than %d characters; %s:%s is not\n", RMR_MAX_SID, wbuf, port );
		return NULL;
	}

	if( (interface = getenv( ENV_BIND_IF )) == NULL ) {
		interface = "0.0.0.0";
	}
	snprintf( bind_info, sizeof( bind_info ), "%s://%s:%s", proto, interface, port );
	if( nn_bind( ctx->nn_sock, bind_info ) < 0) {			// bind and automatically accept client sessions
		fprintf( stderr, "[CRIT] rmr_init: unable to bind nanomsg listen socket for %s: %s\n", bind_info, strerror( errno ) );
		nn_close( ctx->nn_sock );
		free_ctx( ctx );
		return NULL;
	}

	if( ! (flags & FL_NOTHREAD) ) {			// skip if internal context that does not need rout table thread
		if( pthread_create( &ctx->rtc_th,  NULL, rtc, (void *) ctx ) ) { 		// kick the rt collector thread
			fprintf( stderr, "[WARN] rmr_init: unable to start route table collector thread: %s", strerror( errno ) );
		}
	}

	free( proto_port );
	return (void *) ctx;
}


/*
	Publicly facing initialisation function. Wrapper for the init() funcion above
	as it needs to ensure internal flags are masked off before calling the
	real workhorse.
*/
extern void* rmr_init( char* uproto_port, int max_msg_size, int flags ) {
	return init( uproto_port, max_msg_size, flags & UFL_MASK  );
}

/*
	Return true if routing table is initialised etc. and app can send/receive.
*/
extern int rmr_ready( void* vctx ) {
	uta_ctx_t *ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return FALSE;
	}

	if( ctx->rtable != NULL ) {
		return TRUE;
	}

	return FALSE;
}

/*
	Provides a non-fatal (compile) interface for the nng only function.
	Not supported on top of nano, so this always returns -1.
*/
extern int rmr_get_rcvfd( void* vctx ) {
	errno = ENOTSUP;
	return -1;
}

/*
	Compatability (mostly) with NNG.
*/
extern void rmr_close( void* vctx ) {
	uta_ctx_t *ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return;
	}

	nn_close( ctx->nn_sock );
}

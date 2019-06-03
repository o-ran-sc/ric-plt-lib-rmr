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
	Mnemonic:	rmr_nng.c
	Abstract:	This is the compile point for the nng version of the rmr
				library (formarly known as uta, so internal function names
				are likely still uta_*)

				With the exception of the symtab portion of the library,
				RMr is built with a single compile so as to "hide" the
				internal functions as statics.  Because they interdepend
				on each other, and CMake has issues with generating two
				different wormhole objects from a single source, we just
				pull it all together with a centralised comple using
				includes.

				Future:  the API functions at this point can be separated
				into a common source module.

	Author:		E. Scott Daniels
	Date:		1 February 2019
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
#include <time.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>

#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pipeline0/push.h>
#include <nng/protocol/pipeline0/pull.h>


#include "rmr.h"				// things the users see
#include "rmr_agnostic.h"		// agnostic things (must be included before private)
#include "rmr_nng_private.h"	// things that we need too
#include "rmr_symtab.h"

#include "ring_static.c"			// message ring support
#include "rt_generic_static.c"		// route table things not transport specific
#include "rtable_nng_static.c"		// route table things -- transport specific
#include "rtc_static.c"				// route table collector
#include "tools_static.c"
#include "sr_nng_static.c"			// send/receive static functions
#include "wormholes.c"				// wormhole api externals and related static functions (must be LAST!)
#include "mt_call_static.c"
#include "mt_call_nng_static.c"


//------------------------------------------------------------------------------


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
	return msg->alloc_len - RMR_HDR_LEN( msg->header );				// allocated transport size less the header and other data bits
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

	m = alloc_zcmsg( ctx, NULL, size, 0, DEF_TR_LEN );				// alloc with default trace data
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
	This provides an external path to the realloc static function as it's called by an
	outward facing mbuf api function. Used to reallocate a message with a different
	trace data size.
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
			//nng_free( (void *) mbuf->header, mbuf->alloc_len );
			if( mbuf->tp_buf ) {
				nng_msg_free(  mbuf->tp_buf );
			}
		}
	}

	free( mbuf );
}

/*
	This is a wrapper to the real timeout send. We must wrap it now to ensure that
	the call flag and call-id are reset
*/
extern rmr_mbuf_t* rmr_mtosend_msg( void* vctx, rmr_mbuf_t* msg, int max_to ) {
	char* d1;															// point at the call-id in the header

	if( msg != NULL ) {
		((uta_mhdr_t *) msg->header)->flags &= ~HFL_CALL_MSG;			// must ensure call flag is off

		d1 = DATA1_ADDR( msg->header );
		d1[D1_CALLID_IDX] = NO_CALL_ID;										// must blot out so it doesn't queue on a chute at the other end
	}	

	return mtosend_msg( vctx, msg, max_to );
}

/*
	Send with default max timeout as is set in the context.
	See rmr_mtosend_msg() for more details on the parameters.
	See rmr_stimeout() for info on setting the default timeout.
*/
extern rmr_mbuf_t* rmr_send_msg( void* vctx, rmr_mbuf_t* msg ) {
	char* d1;														// point at the call-id in the header

	if( msg != NULL ) {
		((uta_mhdr_t *) msg->header)->flags &= ~HFL_CALL_MSG;			// must ensure call flag is off

		d1 = DATA1_ADDR( msg->header );
		d1[D1_CALLID_IDX] = NO_CALL_ID;										// must blot out so it doesn't queue on a chute at the other end
	}	

	return rmr_mtosend_msg( vctx, msg,  -1 );							// retries < 0  uses default from ctx
}

/*
	Return to sender allows a message to be sent back to the endpoint where it originated.
	The source information in the message is used to select the socket on which to write
	the message rather than using the message type and round-robin selection. This
	should return a message buffer with the state of the send operation set. On success
	(state is RMR_OK, the caller may use the buffer for another receive operation), and on
	error it can be passed back to this function to retry the send if desired. On error,
	errno will liklely have the failure reason set by the nng send processing.
	The following are possible values for the state in the message buffer:

	Message states returned:
		RMR_ERR_BADARG - argument (context or msg) was nil or invalid
		RMR_ERR_NOHDR  - message did not have a header
		RMR_ERR_NOENDPT- an endpoint to send the message to could not be determined
		RMR_ERR_SENDFAILED - send failed; errno has nano error code
		RMR_ERR_RETRY	- the reqest failed but should be retried (EAGAIN)

	A nil message as the return value is rare, and generally indicates some kind of horrible
	failure. The value of errno might give a clue as to what is wrong.

	CAUTION:
		Like send_msg(), this is non-blocking and will return the msg if there is an errror.
		The caller must check for this and handle.
*/
extern rmr_mbuf_t*  rmr_rts_msg( void* vctx, rmr_mbuf_t* msg ) {
	nng_socket	nn_sock;			// endpoint socket for send
	uta_ctx_t*	ctx;
	int			state;
	char*		hold_src;			// we need the original source if send fails
	int			sock_ok = 0;		// true if we found a valid endpoint socket

	if( (ctx = (uta_ctx_t *) vctx) == NULL || msg == NULL ) {		// bad stuff, bail fast
		errno = EINVAL;												// if msg is null, this is their clue
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
		}
		return msg;
	}

	errno = 0;														// at this point any bad state is in msg returned
	if( msg->header == NULL ) {
		fprintf( stderr, "[ERR] rmr_send_msg: message had no header\n" );
		msg->state = RMR_ERR_NOHDR;
		return msg;
	}

	((uta_mhdr_t *) msg->header)->flags &= ~HFL_CALL_MSG;			// must ensure call flag is off
	if( HDR_VERSION( msg->header ) > 2 ) {							// new version uses sender's ip address for rts
		sock_ok = uta_epsock_byname( ctx->rtable, (char *) ((uta_mhdr_t *)msg->header)->srcip, &nn_sock );			// default to IP based rts
	}
	if( ! sock_ok ) {
		sock_ok = uta_epsock_byname( ctx->rtable, (char *) ((uta_mhdr_t *)msg->header)->src, &nn_sock );		// IP  not in rt, try name
		if( ! sock_ok ) {
			msg->state = RMR_ERR_NOENDPT;
			return msg;																// preallocated msg can be reused since not given back to nn
		}
	}

	msg->state = RMR_OK;																// ensure it is clear before send
	hold_src = strdup( (char *) ((uta_mhdr_t *)msg->header)->src );						// the dest where we're returning the message to
	strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, ctx->my_name, RMR_MAX_SRC );	// must overlay the source to be ours
	msg = send_msg( ctx, msg, nn_sock, -1 );
	if( msg ) {
		strncpy( (char *) ((uta_mhdr_t *)msg->header)->src, hold_src, RMR_MAX_SRC );	// always return original source so rts can be called again
		msg->flags |= MFL_ADDSRC;														// if msg given to send() it must add source
	}

	free( hold_src );
	return msg;
}

/*
	If multi-threading call is turned on, this invokes that mechanism with the special call
	id of 1 and a max wait of 1 second.  If multi threaded call is not on, then the original
	behavour (described below) is carried out.  This is safe to use when mt is enabled, but
	the user app is invoking rmr_call() from only one thread, and the caller doesn't need 
	a flexible timeout.

	On timeout this function will return a nil pointer. If the original message could not
	be sent without blocking, it will be returned with the RMR_ERR_RETRY set as the status.

	Original behavour:
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

	if( ctx->flags & CFL_MTC_ENABLED ) {				// if multi threaded call is on, use that
		return rmr_mt_call( vctx, msg, 1, 1000 );		// use the reserved call-id of 1 and wait up to 1 sec
	}

	memcpy( expected_id, msg->xaction, RMR_MAX_XID );
	expected_id[RMR_MAX_XID] = 0;					// ensure it's a string
	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] rmr_call is making call, waiting for (%s)\n", expected_id );
	errno = 0;
	msg->flags |= MFL_NOALLOC;						// we don't need a new buffer from send

	msg = rmr_send_msg( ctx, msg );
	if( msg ) {										// msg should be nil, if not there was a problem; return buffer to user
		if( msg->state != RMR_ERR_RETRY ) {
			msg->state = RMR_ERR_CALLFAILED;		// errno not available to all wrappers; don't stomp if marked retry
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

	if( ctx->flags & CFL_MTC_ENABLED ) {						// must pop from ring with a semaphore dec first
		return rmr_mt_rcv( ctx, old_msg, -1 );
	}

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
	This implements a receive with a timeout via epoll. Mostly this is for
	wrappers as native C applications can use epoll directly and will not have
	to depend on this.
*/
extern rmr_mbuf_t* rmr_torcv_msg( void* vctx, rmr_mbuf_t* old_msg, int ms_to ) {
	struct epoll_stuff* eps;	// convience pointer
	uta_ctx_t*	ctx;
	rmr_mbuf_t*	qm;				// message that was queued on the ring
	int nready;
	rmr_mbuf_t* msg;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		if( old_msg != NULL ) {
			old_msg->state = RMR_ERR_BADARG;
		}
		errno = EINVAL;
		return old_msg;
	}

	if( ctx->flags & CFL_MTC_ENABLED ) {						// must pop from ring with a semaphore dec first
		return rmr_mt_rcv( ctx, old_msg, ms_to );
	}

	qm = (rmr_mbuf_t *) uta_ring_extract( ctx->mring );			// pop if queued
	if( qm != NULL ) {
		if( old_msg ) {
			rmr_free_msg( old_msg ); 							// future:  push onto a free list???
		}

		return qm;
	}

	if( (eps = ctx->eps)  == NULL ) {					// set up epoll on first call
		eps = malloc( sizeof *eps );

		if( (eps->ep_fd = epoll_create1( 0 )) < 0 ) {
	    	fprintf( stderr, "[FAIL] unable to create epoll fd: %d\n", errno );
			free( eps );
			return NULL;
		}

		eps->nng_fd = rmr_get_rcvfd( ctx );
   		eps->epe.events = EPOLLIN;
		eps->epe.data.fd = eps->nng_fd;

		if( epoll_ctl( eps->ep_fd, EPOLL_CTL_ADD, eps->nng_fd, &eps->epe ) != 0 )  {
	    	fprintf( stderr, "[FAIL] epoll_ctl status not 0 : %s\n", strerror( errno ) );
			free( eps );
			return NULL;
		}

		ctx->eps = eps;
	}

	if( old_msg ) {
		msg = old_msg;
	} else {
		msg = alloc_zcmsg( ctx, NULL, RMR_MAX_RCV_BYTES, RMR_OK, DEF_TR_LEN );			// will abort on failure, no need to check
	}

	if( ms_to < 0 ) {
		ms_to = 0;
	}

	nready = epoll_wait( eps->ep_fd, eps->events, 1, ms_to );     // block until something or timedout
	if( nready <= 0 ) {						// we only wait on ours, so we assume ready means it's ours
		msg->state = RMR_ERR_TIMEOUT;
	} else {
		return rcv_msg( ctx, msg );								// receive it and return it
	}

	return msg;				// return empty message with state set
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

//  CAUTION:  these are not supported as they must be set differently (between create and open) in NNG.
//				until those details are worked out, these generate a warning.
/*
	Set send timeout. The value time is assumed to be microseconds.  The timeout is the
	rough maximum amount of time that RMr will block on a send attempt when the underlying
	mechnism indicates eagain or etimeedout.  All other error conditions are reported
	without this delay. Setting a timeout of 0 causes no retries to be attempted in
	RMr code. Setting a timeout of 1 causes RMr to spin up to 10K retries before returning,
	but without issuing a sleep.  If timeout is > 1, then RMr will issue a sleep (1us)
	after every 10K send attempts until the time value is reached. Retries are abandoned
	if NNG returns anything other than NNG_AGAIN or NNG_TIMEDOUT.

	The default, if this function is not used, is 1; meaning that RMr will retry, but will
	not enter a sleep.  In all cases the caller should check the status in the message returned
	after a send call.

	Returns -1 if the context was invalid; RMR_OK otherwise.
*/
extern int rmr_set_stimeout( void* vctx, int time ) {
	uta_ctx_t*	ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return -1;
	}

	if( time < 0 ) {
		time = 0;
	}

	ctx->send_retries = time;
	return RMR_OK;
}

/*
	Set receive timeout -- not supported in nng implementation
*/
extern int rmr_set_rtimeout( void* vctx, int time ) {
	fprintf( stderr, "[WRN] Current implementation of RMR ontop of NNG does not support setting a receive timeout\n" );
	return 0;
}


/*
	This is the actual init workhorse. The user visible function meerly ensures that the
	calling programme does NOT set any internal flags that are supported, and then
	invokes this.  Internal functions (the route table collector) which need additional
	open ports without starting additional route table collectors, will invoke this
	directly with the proper flag.
*/
static void* init(  char* uproto_port, int max_msg_size, int flags ) {
	static	int announced = 0;
	uta_ctx_t*	ctx = NULL;
	char	bind_info[NNG_MAXADDRLEN];	// bind info
	char*	proto = "tcp";				// pointer into the proto/port string user supplied
	char*	port;
	char*	interface = NULL;			// interface to bind to (from RMR_BIND_IF, 0.0.0.0 if not defined)
	char*	proto_port;
	char	wbuf[1024];					// work buffer
	char*	tok;						// pointer at token in a buffer
	int		state;

	if( ! announced ) {
		fprintf( stderr, "[INFO] ric message routing library on NNG mv=%d (%s %s.%s.%s built: %s)\n",
			RMR_MSG_VER, QUOTE_DEF(GIT_ID), QUOTE_DEF(MAJOR_VER), QUOTE_DEF(MINOR_VER), QUOTE_DEF(PATCH_VER), __DATE__ );
		announced = 1;
	}

	errno = 0;
	if( uproto_port == NULL ) {
		proto_port = strdup( DEF_COMM_PORT );
	} else {
		proto_port = strdup( uproto_port );		// so we can modify it
	}

	if( (ctx = (uta_ctx_t *) malloc( sizeof( uta_ctx_t ) )) == NULL ) {
		errno = ENOMEM;
		return NULL;
	}
	memset( ctx, 0, sizeof( uta_ctx_t ) );

	ctx->send_retries = 1;							// default is not to sleep at all; RMr will retry about 10K times before returning
	ctx->d1_len = 4;								// data1 space in header -- 4 bytes for now

	if( flags & RMRFL_MTCALL ) {					// mt call support is on, need bigger ring
		ctx->mring = uta_mk_ring( 2048 );			// message ring filled by rcv thread
		init_mtcall( ctx );							// set up call chutes
	} else {
		ctx->mring = uta_mk_ring( 128 );			// ring filled only on blocking call
	}

	ctx->max_plen = RMR_MAX_RCV_BYTES;				// max user payload lengh
	if( max_msg_size > 0 ) {
		ctx->max_plen = max_msg_size;
	}

	// we're using a listener to get rtg updates, so we do NOT need this.
	//uta_lookup_rtg( ctx );							// attempt to fill in rtg info; rtc will handle missing values/errors

	if( nng_pull0_open( &ctx->nn_sock )  !=  0 ) {		// and assign the mode
		fprintf( stderr, "[CRI] rmr_init: unable to initialise nng listen (pull) socket: %d\n", errno );
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

	if( (gethostname( wbuf, sizeof( wbuf ) )) != 0 ) {
		fprintf( stderr, "[CRI] rmr_init: cannot determine localhost name: %s\n", strerror( errno ) );
		return NULL;
	}
	if( (tok = strchr( wbuf, '.' )) != NULL ) {
		*tok = 0;									// we don't keep domain portion
	}
	ctx->my_name = (char *) malloc( sizeof( char ) * RMR_MAX_SRC );
	if( snprintf( ctx->my_name, RMR_MAX_SRC, "%s:%s", wbuf, port ) >= RMR_MAX_SRC ) {			// our registered name is host:port
		fprintf( stderr, "[CRI] rmr_init: hostname + port must be less than %d characters; %s:%s is not\n", RMR_MAX_SRC, wbuf, port );
		return NULL;
	}

	if( (tok = getenv( ENV_NAME_ONLY )) != NULL ) {
		if( atoi( tok ) > 0 ) {
			flags |= RMRFL_NAME_ONLY;					// don't allow IP addreess to go out in messages
		}
	}

	ctx->ip_list = mk_ip_list( port );				// suss out all IP addresses we can find on the box, and bang on our port for RT comparisons
	if( flags & RMRFL_NAME_ONLY ) {
		ctx->my_ip = strdup( ctx->my_name );			// user application or env var has specified that IP address is NOT sent out, use name
	} else {
		ctx->my_ip = get_default_ip( ctx->ip_list );	// and (guess) at what should be the default to put into messages as src
		if( ctx->my_ip == NULL ) {
			fprintf( stderr, "[WARN] rmr_init: default ip address could not be sussed out, using name\n" );
			strcpy( ctx->my_ip, ctx->my_name );			// if we cannot suss it out, use the name rather than a nil pointer
		}
	}
	if( DEBUG ) fprintf( stderr, "[DBUG] default ip address: %s\n", ctx->my_ip );



	if( (interface = getenv( ENV_BIND_IF )) == NULL ) {
		interface = "0.0.0.0";
	}
	// NOTE: if there are options that might need to be configured, the listener must be created, options set, then started
	//       rather than using this generic listen() call.
	snprintf( bind_info, sizeof( bind_info ), "%s://%s:%s", proto, interface, port );
	if( (state = nng_listen( ctx->nn_sock, bind_info, NULL, NO_FLAGS )) != 0 ) {
		fprintf( stderr, "[CRIT] rmr_init: unable to start nng listener for %s: %s\n", bind_info, nng_strerror( state ) );
		nng_close( ctx->nn_sock );
		free_ctx( ctx );
		return NULL;
	}

	if( !(flags & FL_NOTHREAD) ) {										// skip if internal function that doesnt need an rtc
		if( pthread_create( &ctx->rtc_th,  NULL, rtc, (void *) ctx ) ) { 	// kick the rt collector thread
			fprintf( stderr, "[WARN] rmr_init: unable to start route table collector thread: %s", strerror( errno ) );
		}
	}

	if( (flags & RMRFL_MTCALL) && ! (ctx->flags & CFL_MTC_ENABLED) ) {	// mt call support is on, must start the listener thread if not running
		ctx->flags |= CFL_MTC_ENABLED;
		if( pthread_create( &ctx->mtc_th,  NULL, mt_receive, (void *) ctx ) ) { 	// kick the receiver
			fprintf( stderr, "[WARN] rmr_init: unable to start multi-threaded receiver: %s", strerror( errno ) );
		}
		
	}

	free( proto_port );
	return (void *) ctx;
}

/*
	Initialise the message routing environment. Flags are one of the UTAFL_
	constants. Proto_port is a protocol:port string (e.g. tcp:1234). If default protocol
	(tcp) to be used, then :port is all that is needed.

	At the moment it seems that TCP really is the only viable protocol, but
	we'll allow flexibility.

	The return value is a void pointer which must be passed to most uta functions. On
	error, a nil pointer is returned and errno should be set.

	Flags:
		No user flags supported (needed) at the moment, but this provides for extension
		without drastically changing anything. The user should invoke with RMRFL_NONE to
		avoid any misbehavour as there are internal flags which are suported
*/
extern void* rmr_init( char* uproto_port, int max_msg_size, int flags ) {
	return init( uproto_port, max_msg_size, flags & UFL_MASK  );		// ensure any internal flags are off
}

/*
	This sets the default trace length which will be added to any message buffers
	allocated.  It can be set at any time, and if rmr_set_trace() is given a
	trace len that is different than the default allcoated in a message, the message
	will be resized.

	Returns 0 on failure and 1 on success. If failure, then errno will be set.
*/
extern int rmr_init_trace( void* vctx, int tr_len ) {
	uta_ctx_t* ctx;

	errno = 0;
	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return 0;
	}

	ctx->trace_data_len = tr_len;
	return 1;
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
	Returns a file descriptor which can be used with epoll() to signal a receive
	pending. The file descriptor should NOT be read from directly, nor closed, as NNG
	does not support this.
*/
extern int rmr_get_rcvfd( void* vctx ) {
	uta_ctx_t* ctx;
	int fd;
	int state;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return -1;
	}

	if( (state = nng_getopt_int( ctx->nn_sock, NNG_OPT_RECVFD, &fd )) != 0 ) {
		fprintf( stderr, "[WRN] rmr cannot get recv fd: %s\n", nng_strerror( state ) );
		return -1;
	}

	return fd;
}


/*
	Clean up things.

	There isn't an nng_flush() per se, but we can pause, generate
	a context switch, which should allow the last sent buffer to
	flow. There isn't exactly an nng_term/close either, so there
	isn't much we can do.
*/
extern void rmr_close( void* vctx ) {
	uta_ctx_t *ctx;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return;
	}

	ctx->shutdown = 1;
	nng_close( ctx->nn_sock );
}


// ----- multi-threaded call/receive support -------------------------------------------------

/*
	Blocks on the receive ring chute semaphore and then reads from the ring
	when it is tickled.  If max_wait is -1 then the function blocks until
	a message is ready on the ring. Else max_wait is assumed to be the number
	of millaseconds to wait before returning a timeout message.
*/
extern rmr_mbuf_t* rmr_mt_rcv( void* vctx, rmr_mbuf_t* mbuf, int max_wait ) {
	uta_ctx_t*	ctx;
	uta_mhdr_t*	hdr;			// header in the transport buffer
	chute_t*	chute;
	struct timespec	ts;			// time info if we have a timeout
	long	new_ms;				// adjusted mu-sec
	long	seconds = 0;		// max wait seconds
	long	nano_sec;			// max wait xlated to nano seconds
	int		state;
	rmr_mbuf_t*	ombuf;			// mbuf user passed; if we timeout we return state here
	
	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		if( mbuf ) {
			mbuf->state = RMR_ERR_BADARG;
		}
		return mbuf;
	}

	if( ! (ctx->flags & CFL_MTC_ENABLED) ) {
		errno = EINVAL;
		if( mbuf != NULL ) {
			mbuf->state = RMR_ERR_NOTSUPP;
		}
		return mbuf;
	}

	ombuf = mbuf;
	if( ombuf ) {
		ombuf->state = RMR_ERR_TIMEOUT;			// preset if for failure
		ombuf->len = 0;
	}

	chute = &ctx->chutes[0];					// chute 0 used only for its semaphore
	
	if( max_wait > 0 ) {
		clock_gettime( CLOCK_REALTIME, &ts );	

		if( max_wait > 999 ) {
			seconds = (max_wait - 999)/1000;
			max_wait -= seconds * 1000;
			ts.tv_sec += seconds;
		}
		if( max_wait > 0 ) {
			nano_sec = max_wait * 1000000;
			ts.tv_nsec += nano_sec;
			if( ts.tv_nsec > 999999999 ) {
				ts.tv_nsec -= 999999999;
				ts.tv_sec++;
			}
		}

		seconds = 1;													// use as flag later to invoked timed wait
	}

	errno = 0;
	while( chute->mbuf == NULL && ! errno ) {
		if( seconds ) {
			state = sem_timedwait( &chute->barrier, &ts );				// wait for msg or timeout
		} else {
			state = sem_wait( &chute->barrier );
		}

		if( state < 0 && errno == EINTR ) {								// interrupted go back and wait; all other errors cause exit
			errno = 0;
		}
	}

	if( state < 0 ) {
		mbuf = ombuf;				// return caller's buffer if they passed one in
	} else {
		if( DEBUG ) fprintf( stderr, "[DBUG] mt_rcv extracting from normal ring\n" );
		if( (mbuf = (rmr_mbuf_t *) uta_ring_extract( ctx->mring )) != NULL ) {			// pop if queued
			if( mbuf ) {
				mbuf->state = RMR_OK;

				if( ombuf ) {
					rmr_free_msg( ombuf );					// we cannot reuse as mbufs are queued on the ring
				}
			} else {
				mbuf = ombuf;				// no buffer, return user's if there
			}
		}
	}

	return mbuf;
}

/*
	Accept a message buffer and caller ID, send the message and then wait
	for the receiver to tickle the semaphore letting us know that a message
	has been received. The call_id is a value between 2 and 255, inclusive; if
	it's not in this range an error will be returned. Max wait is the amount
	of time in millaseconds that the call should block for. If 0 is given
	then no timeout is set.

	If the mt_call feature has not been initialised, then the attempt to use this
	funciton will fail with RMR_ERR_NOTSUPP

	If no matching message is received before the max_wait period expires, a
	nil pointer is returned, and errno is set to ETIMEOUT. If any other error
	occurs after the message has been sent, then a nil pointer is returned
	with errno set to some other value.
*/
extern rmr_mbuf_t* rmr_mt_call( void* vctx, rmr_mbuf_t* mbuf, int call_id, int max_wait ) {
	rmr_mbuf_t* ombuf;			// original mbuf passed in
	uta_ctx_t*	ctx;
	uta_mhdr_t*	hdr;			// header in the transport buffer
	chute_t*	chute;
	unsigned char*	d1;			// d1 data in header
	struct timespec	ts;			// time info if we have a timeout
	long	new_ms;				// adjusted mu-sec
	long	seconds = 0;		// max wait seconds
	long	nano_sec;			// max wait xlated to nano seconds
	int		state;
	
	if( (ctx = (uta_ctx_t *) vctx) == NULL || mbuf == NULL ) {
		errno = EINVAL;
		if( mbuf ) {
			mbuf->state = RMR_ERR_BADARG;
		}
		return mbuf;
	}

	if( ! (ctx->flags & CFL_MTC_ENABLED) ) {
		mbuf->state = RMR_ERR_NOTSUPP;
		return mbuf;
	}

	if( call_id > MAX_CALL_ID || call_id < 2 ) {					// 0 and 1 are reserved; user app cannot supply them
		mbuf->state = RMR_ERR_BADARG;
		return mbuf;
	}

	ombuf = mbuf;													// save to return timeout status with

	chute = &ctx->chutes[call_id];
	if( chute->mbuf != NULL ) {										// probably a delayed message that wasn't dropped
		rmr_free_msg( chute->mbuf );
		chute->mbuf = NULL;
	}
	
	hdr = (uta_mhdr_t *) mbuf->header;
	hdr->flags |= HFL_CALL_MSG;										// must signal this sent with a call
	memcpy( chute->expect, mbuf->xaction, RMR_MAX_XID );			// xaction that we will wait for
	d1 = DATA1_ADDR( hdr );
	d1[D1_CALLID_IDX] = (unsigned char) call_id;					// set the caller ID for the response
	mbuf->flags |= MFL_NOALLOC;										// send message without allocating a new one (expect nil from mtosend

	if( max_wait > 0 ) {
		clock_gettime( CLOCK_REALTIME, &ts );	

		if( max_wait > 999 ) {
			seconds = (max_wait - 999)/1000;
			max_wait -= seconds * 1000;
			ts.tv_sec += seconds;
		}
		if( max_wait > 0 ) {
			nano_sec = max_wait * 1000000;
			ts.tv_nsec += nano_sec;
			if( ts.tv_nsec > 999999999 ) {
				ts.tv_nsec -= 999999999;
				ts.tv_sec++;
			}
		}

		seconds = 1;										// use as flag later to invoked timed wait
	}

	mbuf = mtosend_msg( ctx, mbuf, 0 );						// use internal function so as not to strip call-id; should be nil on success!
	if( mbuf ) {
		if( mbuf->state != RMR_OK ) {
			return mbuf;									// timeout or unable to connect or no endpoint are most likely issues
		}
	}

	errno = 0;
	while( chute->mbuf == NULL && ! errno ) {
		if( seconds ) {
			state = sem_timedwait( &chute->barrier, &ts );				// wait for msg or timeout
		} else {
			state = sem_wait( &chute->barrier );
		}

		if( state < 0 && errno == EINTR ) {								// interrupted go back and wait; all other errors cause exit
			errno = 0;
		}

		if( chute->mbuf != NULL ) {										// offload receiver thread and check xaction buffer here
			if( memcmp( chute->expect, chute->mbuf->xaction, RMR_MAX_XID ) != 0 ) {
				rmr_free_msg( chute->mbuf );
				chute->mbuf = NULL;
				errno = 0;
			}
		}
	}

	if( state < 0 ) {
		return NULL;					// leave errno as set by sem wait call
	}

	mbuf = chute->mbuf;
	mbuf->state = RMR_OK;
	chute->mbuf = NULL;

	return mbuf;
}

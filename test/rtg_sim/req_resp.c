// :vi ts=4 sw=4 noet:
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
	Mnemonic: 	req_rep.c
	Abstract:	A "library" module which allows a programme to easily be a requestor
				or replier.  Some functions are compatable with publishing (mbuf
				allocation and management).  Underlying we use the NN_PAIR and NOT
				the req/rep model as that model is an inflexible, lock step, exchange
				which does not lend well for a request that results in more than one
				response messages, or no response.

				The user must be aware that once a session is established on the
				host:port listener, another session will not be accepted until the
				first is terminated; nano makes no provision for multiple concurrent
				sesssions with either the PAIR or REQ/RESP models.

				We also support starting the publisher socket as the buffer and
				send functions can be used for the publisher too.

				CAUTION:  this is based on nanomsg, not NNG. The underlying protocols
					are compatable, and because NNG has an emulation mode it is possible
					to link successsfully with the nng library, BUT that will not
					work here.   Link only with nanomsg.

	Date:		18 January 2018
	Author: 	E. Scott Daniels

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/pubsub.h>

#include "req_resp.h"

#define NULL_SOCKET 0		// fluff that is treated like a nil pointer check by coverage checker


/*
	Connect to the host as a requestor. returns context if
	successful.
*/
extern void* rr_connect( char* host, char* port ) {
	rr_ctx_t* ctx = NULL;
	char	wbuf[1024];
	int		state;

	if( host == NULL || port == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	ctx = (rr_ctx_t *) malloc( sizeof *ctx );
	if( ctx == NULL ) {
		errno = ENOMEM;
		return NULL;
	}

	//ctx->nn_sock = nn_socket( AF_SP, NN_PAIR );
	ctx->nn_sock = nn_socket( AF_SP, NN_PUSH );
	if( ctx->nn_sock < NULL_SOCKET ) {
		free( ctx );
		return NULL;
	}
	snprintf( wbuf, sizeof( wbuf ), "tcp://%s:%s", host, port );
	state = nn_connect( ctx->nn_sock, wbuf );
	if( state < 0 ) {
		fprintf( stderr, "rr_conn: connect failed: %s: %d %s\n", wbuf, errno, strerror( errno ) );
		nn_close( ctx->nn_sock );
		free( ctx );
		return NULL;
	}

	//fprintf( stderr, "rr_conn: connect successful: %s\n", wbuf );
	return (void *) ctx;
}


/*
	Set up as a listener on any interface with the given port.
*/
extern void* rr_start_listening( char* port ) {
	rr_ctx_t* ctx;
	char	wbuf[1024];
	int		state;

	if( port == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	ctx = (rr_ctx_t *) malloc( sizeof *ctx );
	if( ctx == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	//ctx->nn_sock = nn_socket( AF_SP, NN_PAIR );
	ctx->nn_sock = nn_socket( AF_SP, NN_PULL );
	if( ctx->nn_sock < NULL_SOCKET ) {
		free( ctx );
		return NULL;
	}

	snprintf( wbuf, sizeof( wbuf ), "tcp://0.0.0.0:%s", port );
	state = nn_bind( ctx->nn_sock, wbuf );
	if( state < 0 ) {
		nn_close( ctx->nn_sock );
		free( ctx );
		return NULL;
	}

	return (void *) ctx;
}

/*
	Configure and bind the publisher. Port is a string as it's probably read from
	the command line, so no need to atoi() it for us.  We can use the rr_* functions
	for message buffers and sending, so we reuse their context rather than define our
	own.

*/
extern void*  open_publisher( char*  port ) {
	rr_ctx_t*	pctx;
	char		conn_info[1024];

	if( (pctx = (rr_ctx_t *) malloc( sizeof( *pctx )) ) == NULL ) {
		return NULL;
	}

    pctx->nn_sock = nn_socket( AF_SP, NN_PUB );		// publishing socket
    if( pctx->nn_sock < 0 ) {
        fprintf( stderr, "[CRI] unable to open publish socket: %s\n", strerror( errno ) );
		free( pctx );
        return NULL;
    }

	snprintf( conn_info, sizeof( conn_info ), "tcp://0.0.0.0:%s", port );			// listen on any interface
    if( nn_bind( pctx->nn_sock, conn_info ) < 0) {									// bind and automatically accept client sessions
        fprintf (stderr, "[CRI] unable to bind publising port: %s: %s\n", port, strerror( errno ) );
        nn_close ( pctx->nn_sock );
		free( pctx );
        return NULL;
    }

	return (void *) pctx;
}

extern rr_mbuf_t* rr_new_buffer( rr_mbuf_t* mb, int len ) {

	if( ! mb ) {
		mb = (rr_mbuf_t *) malloc( sizeof( *mb ) );
		mb->size = len;
		mb->payload = NULL;
	} else {
		if( mb->size < len ) {					// if requested len is larger than current payload
			nn_freemsg( mb->payload );
			mb->payload = NULL;
		} else {
			len = mb->size;
		}
	}
	mb->used = 0;

	if( len > 0 && !mb->payload ) {								// allow a payloadless buffer to be allocated
		mb->payload = nn_allocmsg( len, 0 );
	}

	return mb;
}

/*
	Closes the currently open session.
*/
extern void rr_close( void* vctx ) {
	rr_ctx_t* ctx;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		return;
	}

	if( ctx->nn_sock < NULL_SOCKET ) {
		return;
	}

	nn_close( ctx->nn_sock );
	ctx->nn_sock = -1;
}

extern void rr_free( void* vctx ) {
	rr_ctx_t* ctx;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		return;
	}

	rr_close( ctx );
	nn_term();
	free( ctx );
}

extern void rr_free_mbuf( rr_mbuf_t* mbuf ) {
	if( mbuf->payload ) {
		nn_freemsg( mbuf->payload );
		mbuf->payload = NULL;
		mbuf->used = -2;				// just in case they held a pointer and try to use it
	}

	free( mbuf );
}

extern rr_mbuf_t*  rr_receive( void* vctx, rr_mbuf_t* mbuf, int len ) {
	rr_ctx_t* ctx;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return NULL;
	}
	if( ctx->nn_sock < 0 ) {
		errno = ESTALE;					// stale/bad socket fd
		return NULL;
	}

	mbuf = rr_new_buffer( mbuf, len );
	if( mbuf == NULL ) {
		return NULL;
	}

	*mbuf->payload = 0;
	if( (mbuf->used = nn_recv( ctx->nn_sock, mbuf->payload, mbuf->size, 0 )) > 0 ) {
		errno = 0;						// nano doesn't seem to clear errno here
	}
	return mbuf;
}

extern rr_mbuf_t* rr_send( void* vctx, rr_mbuf_t* mbuf, int alloc_buf ) {
	rr_ctx_t* ctx;
	int len;
	int state;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return NULL;
	}

	if( ctx->nn_sock < 0 ) {
		errno = ESTALE;					// stale/bad socket fd
		return NULL;
	}

	if( ! mbuf ) {
		errno = ENOBUFS;			// not quite right, but close enough
		return NULL;
	}

	if( ! mbuf->payload ) {			// no payload????
		errno = EFAULT;				// nil is a bad address after all :)
		return mbuf;
	}

	errno = 0;
	//fprintf( stderr, "rrsend is sending %d bytes....\n", mbuf->used );
	if( (state = nn_send( ctx->nn_sock, &mbuf->payload, NN_MSG, 0 )) > 0 ) {
		//fprintf( stderr, "send ok to %d:  %d %s\n", ctx->nn_sock, state, strerror( errno ) );
		mbuf->used = 0;
		if( alloc_buf ) {
			mbuf->payload = nn_allocmsg( mbuf->size, 0 );					// allocate the next send buffer
		} else {
			mbuf->payload = NULL;
			mbuf->used = -1;
		}

		errno = 0;
	} else {
		fprintf( stderr, "send failed %d %s\n", state, strerror( errno ) );
	}

	return mbuf;
}

/*
	Set the receive timeout to time. If time >100 we assume the time is milliseconds,
	else we assume seconds. Setting -1 is always block.
	Returns the nn value (0 on success <0 on error).
*/
extern int rr_rcv_to( void* vctx, int time ) {
	rr_ctx_t* ctx;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return -1;
	}

	if( time > 0 ) {
		if( time < 100 ) {
			time = time * 1000;			// assume seconds, nn wants ms
		}
	}

	return nn_setsockopt( ctx->nn_sock, NN_SOL_SOCKET, NN_RCVTIMEO, &time, sizeof( time ) );
}

/*
	Set the send timeout to time. If time >100 we assume the time is milliseconds,
	else we assume seconds. Setting -1 is always block.
	Returns the nn value (0 on success <0 on error).
*/
extern int rr_send_to( void* vctx, int time ) {
	rr_ctx_t* ctx;

	if( (ctx = (rr_ctx_t *) vctx) == NULL ) {
		errno = EINVAL;
		return -1;
	}

	if( time > 0 ) {
		if( time < 100 ) {
			time = time * 1000;			// assume seconds, nn wants ms
		}
	}

	return nn_setsockopt( ctx->nn_sock, NN_SOL_SOCKET, NN_SNDTIMEO, &time, sizeof( time ) );
}


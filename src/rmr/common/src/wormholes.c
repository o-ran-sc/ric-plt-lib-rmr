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
	Mnemonic:	wormholes.c
	Abstract:	All functions (internal and external) needed to manage wormholes.

				Wormholes allow a user application to send directly to an endpoint.
				The application must first "open" the wormhole which allows us to
				provide the application with an ID that can be used on a wh_send()
				call. It also does the validation (future) which might not allow
				the application to open any wormholes, or may allow them only to
				specific targets.

	Author:		E. Scott Daniels
	Date:		13 February 2019
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "rmr.h"
#include "rmr_symtab.h"

/*
#ifdef NNG
#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>
#include <nng/protocol/pubsub0/sub.h>
#include <nng/protocol/pipeline0/push.h>
#include <nng/protocol/pipeline0/pull.h>

#include "rmr_nng_private.h"
#include "rt_generic_static.c"
#include "rtable_nng_static.c"
#include "sr_nng_static.c"

#else
#include <nanomsg/nn.h>
#include <nanomsg/tcp.h>
#include <nanomsg/pair.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/pubsub.h>

#include "rmr_private.h"
#include "rt_generic_static.c"
#include "rtable_static.c"
#include "sr_static.c"
#endif
*/

#include "tools_static.c"



// ----------------------- internal stuff -----------------------------------------------

/*
	This function returns true if the current application is permitted to open a wormhole
	to the desired target.

	This is a place holder for future functionality.
*/
static int wh_can_open( uta_ctx_t* ctx, char const* target ) {
	return 1;
}

/*
	Allocate and initialise the wormholes list; point context at it.
*/
static int wh_init( uta_ctx_t* ctx ) {
	wh_mgt_t*	whm;
	size_t		alloc_sz;

	if( ctx == NULL ) {
		errno = EINVAL;
		return 0;
	}

	if( ctx->wormholes != NULL ) {		// already allocated, do nothing but signal all is well
		return 1;
	}

	if( (whm  = malloc( sizeof( *whm ) )) == NULL ) {
		rmr_vlog( RMR_VL_ERR, "mem alloc failed for whm: alloc %d bytes\n", (int) sizeof( *whm ) );
		errno = ENOMEM;
		return 0;
	}

	whm->nalloc = 16;
	alloc_sz = whm->nalloc * sizeof( endpoint_t );
	if( (whm->eps = (endpoint_t **) malloc( alloc_sz )) == NULL ) {
		rmr_vlog( RMR_VL_ERR, "mem alloc failed: alloc %d bytes\n", (int) alloc_sz );
		free( whm );
		errno = ENOMEM;
		return 0;
	}

	memset( whm->eps, 0, alloc_sz );

	ctx->wormholes = whm;
	errno = 0;
	return 1;
}

/*
	Realloc the wormhole endpoint list.
	Returns 0 if failure with errno set; !0 on success.
*/
static int wh_extend( wh_mgt_t* whm ) {
	int i;
	int j;
	size_t	alloc_sz;

	i = whm->nalloc;		// starting point for initialisation after realloc
	whm->nalloc += 16;

	alloc_sz = whm->nalloc * sizeof( endpoint_t );
	if( (whm->eps = (endpoint_t **) realloc( whm->eps, alloc_sz )) == NULL ) {
		errno = ENOMEM;
		return 0;
	}

	for( j = 0; j < 16; j++ ) {
		whm->eps[i++] = NULL;			// must init the new stuff
	}

	errno = 0;
	return 1;
}

/*
	Mostly for leak analysis during testing.
*/
static void wh_nuke( uta_ctx_t* ctx ) {
	if( ctx == NULL ) {
		return;
	}

	if( ctx->wormholes ) {
		if( ctx->wormholes->eps ) {
			free( ctx->wormholes->eps );
		}
		free( ctx->wormholes );
	}

	ctx->wormholes = NULL;
}

// ----------------------- visible stuff  ------------------------------------------------

/*
	Opens a direct wormhole connection to the named target. Target is expected to be
	either hostname:port or IP:port.  If we don't have an endpoint in our hash, we'll
	create one.
	Unlike 'regular' connections to endpoints which are connected on the first send
	attempt, when a wormhole is opened we connect immediatly. In the NNG world this
	could result in a delay and immediate failure. With nanomsg the failure may not
	be detected as the connect doesn't block.
*/
extern rmr_whid_t rmr_wh_open( void* vctx, char const* target ) {
	endpoint_t*	ep;				// endpoint that represents the target
	uta_ctx_t*	ctx = NULL;
	rmr_whid_t	whid = -1;		// wormhole id is the index into the list
	wh_mgt_t*	whm;			// easy reference to wh mgt stuff
	int			i;


	if( (ctx = (uta_ctx_t *) vctx) == NULL || target == NULL || *target == 0 ) {
		errno = EINVAL;
		return whid;
	}

	if( ! wh_can_open( ctx, target ) )  {
		errno = EACCES;
		return whid;
	}

	if( ctx->wormholes == NULL ) {
		if( ! wh_init( ctx ) ) {					// first call, we need to set things up
			return whid;							// fail with errno set by init
		}
	}

	whm = ctx->wormholes;


	if( (ep = rt_ensure_ep( ctx->rtable, target )) == NULL ) {		// get pointer to ep if there, create new if not
		rmr_vlog( RMR_VL_ERR, "wormhole_open: ensure ep returned bad: target=(%s)\n", target );
		return -1;			// ensure sets errno
	}

	whid = whm->nalloc;
	for( i = 0; i < whm->nalloc; i++ ) {				// look for a pointer to the ep, and find first open spot
		if( whid == whm->nalloc && !whm->eps[i] ) {
			whid = i;									// save first open slot should we need it
		}

		if( whm->eps[i] == ep ) {
			if(  whm->eps[i]->open ) {					// we know about it and it's open
				return i;								// just send back the reference
			}

			whid = i;									// have it, but not open, reopen
			break;
		}
	}

	if( whid >= whm->nalloc ) {
		if( ! wh_extend( whm  ) ) { 					// add some; whid will point to the right place
			return -1;
		}
	}

	if( !rt_link2_ep( ctx, ep ) ) {			// start a connection if not already open
		errno = ECONNREFUSED;
		return -1;
	}

	whm->eps[whid] = ep;
	return whid;
}


/*
	Send a message directly to an open wormhole.
	As with the other send functions in RMr, we return a new zero copy buffer for the
	user application to fill in.
*/
extern rmr_mbuf_t* rmr_wh_send_msg( void* vctx, rmr_whid_t whid, rmr_mbuf_t* msg ) {
	uta_ctx_t*	ctx;
	endpoint_t*	ep;				// enpoint that wormhole ID references
	wh_mgt_t *whm;
	char* d1;					// point at the call-id in the header

	if( (ctx = (uta_ctx_t *) vctx) == NULL || msg == NULL ) {		// bad stuff, bail fast
		errno = EINVAL;												// if msg is null, this is their clue
		if( msg != NULL ) {
			msg->state = RMR_ERR_BADARG;
			errno = EINVAL;											// must ensure it's not eagain
		}
		return msg;
	}

	msg->state = RMR_OK;

	if( (whm = ctx->wormholes) == NULL ) {
		errno = EINVAL;												// no wormholes open
		msg->state = RMR_ERR_NOWHOPEN;
		return msg;
	}

	if( whid < 0 || whid >= whm->nalloc || whm->eps[whid] == NULL ) {
		errno = EINVAL;												// no wormholes open
		msg->state = RMR_ERR_WHID;
		return msg;
	}

	errno = 0;
	if( msg->header == NULL ) {
		rmr_vlog( RMR_VL_ERR, "rmr_wh_send_msg: message had no header\n" );
		msg->state = RMR_ERR_NOHDR;
		errno = EBADMSG;										// must ensure it's not eagain
		return msg;
	}

	d1 = DATA1_ADDR( msg->header );
	d1[D1_CALLID_IDX] = NO_CALL_ID;								// must blot out so it doesn't queue on a chute at the other end

	ep = whm->eps[whid];
	if( ! ep->open ) {
		rmr_wh_open( ctx, ep->name );
	}
	return send2ep( ctx, ep, msg );							// send directly to the endpoint
}

/*
	This will "close" a wormhole.  We don't actually drop the session as that might be needed
	by others, but we do pull the ep reference from the list.
*/
extern void rmr_wh_close( void* vctx, int whid ) {
	uta_ctx_t*	ctx;
	wh_mgt_t *whm;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return;
	}

	if( (whm = ctx->wormholes) == NULL || whm->eps == NULL ) {
		return;
	}

	if( whid >= whm->nalloc || whid < 0 ) {
		return;
	}

	if( whm->eps[whid] == NULL ) {
		return;
	}

	whm->eps[whid] = NULL;
}

/*
	Check the state of an endpoint that is associated with the wormhold ID
	passed in. If the state is "open" then we return RMR_OK. Other possible
	return codes:

		RMR_ERR_WHID        // wormhole id was invalid
		RMR_ERR_NOENDPT     // the endpoint connection is not open
		RMR_ERR_BADARG		// context or other arg was invalid
		RMR_ERR_NOWHOPEN	// wormhole(s) have not been initalised

*/
extern int rmr_wh_state( void* vctx, rmr_whid_t whid ) {
	uta_ctx_t*	ctx;
	wh_mgt_t*	whm;			// easy reference to wh mgt stuff
	endpoint_t*	ep;				// enpoint that wormhole ID references

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {		// bad stuff, bail fast
		errno = EINVAL;
		return RMR_ERR_BADARG;
	}

	if( (whm = ctx->wormholes) == NULL ) {
		errno = EINVAL;												// no wormholes open
		return RMR_ERR_NOWHOPEN;
	}

	if( whid < 0 || whid >= whm->nalloc || whm->eps[whid] == NULL ) {
		errno = EINVAL;
		return RMR_ERR_WHID;
	}

	errno = 0;

	if( (ep = whm->eps[whid]) != NULL ) {
		return ep->open ? RMR_OK : RMR_ERR_NOENDPT;
	}

	return RMR_ERR_NOENDPT;
}

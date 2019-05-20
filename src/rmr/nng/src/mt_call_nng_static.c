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
	Mnemonic:	mt_call_nng_static.c
	Abstract:	Static funcitons related to the multi-threaded call feature
				which are NNG specific.

	Author:		E. Scott Daniels
	Date:		20 May 2019
*/

#ifndef _mtcall_nng_static_c
#define _mtcall_nng_static_c
#include <semaphore.h>

static inline void queue_normal( uta_ctx_t* ctx, rmr_mbuf_t* mbuf ) {
	chute_t*	chute;
	int			state;

	if( ! uta_ring_insert( ctx->mring, mbuf ) ) {
		rmr_free_msg( mbuf );								// drop if ring is full
	}

	chute = &ctx->chutes[0];
	chute->mbuf = mbuf;
	state = sem_post( &chute->barrier );								// tickle the ring monitor
}

/*
	This is expected to execute in a separate thread. It is responsible for
	_all_ receives and queues them on the appropriate ring, or chute.

	The "state" of the message is checked which determines where the message
	is delivered.

		Flags indicate that the message is a call generated message, then
		the message is queued on the normal receive ring.

		Chute ID is == 0, then the message is queued on the normal receive ring.

		The transaction ID in the message matches the expected ID in the chute,
		then the message is given to the chute and the chute's semaphore is tickled.

		If none are true, the message is dropped.
*/
static void* mt_receive( void* vctx ) {
	uta_ctx_t*		ctx;
	uta_mhdr_t*		hdr;		// header of the message received
	rmr_mbuf_t*		mbuf;		// msg received
	unsigned char*	d1;			// pointer at d1 data ([0] is the call_id)
	chute_t*		chute;
	unsigned int	call_id;	// the id assigned to the call generated message

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		if( DEBUG ) fprintf( stderr, "rmr mt_receive: bad parms, thread not started\n" );
		return NULL;
	}

	fprintf( stderr, "[INFO] rmr mt_receiver is spinning\n" );

	while( ! ctx->shutdown ) {
		mbuf = rcv_msg( ctx, NULL );

		if( mbuf != NULL && (hdr = (uta_mhdr_t *) mbuf->header) != NULL ) {
			if( hdr->flags & HFL_CALL_MSG ) {					// call generated message; ignore call-id etc and queue
				queue_normal( ctx, mbuf );
			} else {
				if( RMR_D1_LEN( hdr ) <= 0 ) {					// no call-id data; just queue
					queue_normal( ctx, mbuf );
				} else {
					d1 = DATA1_ADDR( hdr );
					if( (call_id = (unsigned int) d1[D1_CALLID_IDX]) == 0 ) {					// call_id not set, just queue
						queue_normal( ctx, mbuf );
					} else {
						chute = &ctx->chutes[call_id];
						if( memcmp( mbuf->xaction, chute->expect, RMR_MAX_XID ) == 0 ) {		// match
							chute->mbuf = mbuf;
							sem_post( &chute->barrier );
						} else {
							rmr_free_msg( mbuf );
							mbuf = NULL;
						}
					}
				}
			}
		} else {
			if( ! mbuf ) {				// very very unlikely, but prevent leaks
				rmr_free_msg( mbuf );
			}
		}
	}

	return NULL;
}

#endif

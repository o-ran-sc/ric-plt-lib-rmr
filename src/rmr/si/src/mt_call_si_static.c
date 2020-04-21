// : vi ts=4 sw=4 noet:
/*
==================================================================================
	Copyright (c) 2020 Nokia
	Copyright (c) 2018-2020 AT&T Intellectual Property.

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
	Mnemonic:	mt_call_si static.c
	Abstract:	Static funcitons related to the multi-threaded call feature
				which are SI specific. The functions here also provide the
				message construction functions which build a message that
				might be split across multiple "datagrams" received from the
				underlying transport.

	Author:		E. Scott Daniels
	Date:		20 May 2019
*/

#ifndef _mtcall_si_static_c
#define _mtcall_si_static_c
#include <semaphore.h>

static inline void queue_normal( uta_ctx_t* ctx, rmr_mbuf_t* mbuf ) {
	static	int warned = 0;
	chute_t*	chute;

	if( ! uta_ring_insert( ctx->mring, mbuf ) ) {
		rmr_free_msg( mbuf );								// drop if ring is full
		if( !warned ) {
			rmr_vlog( RMR_VL_ERR, "rmr_mt_receive: application is not receiving fast enough; messages dropping\n" );
			warned++;
		}

		return;
	}

	chute = &ctx->chutes[0];
	sem_post( &chute->barrier );								// tickle the ring monitor
}

/*
	Allocate a message buffer, point it at the accumulated (raw) message,
	call ref to point to all of the various bits and set real len etc,
	then we queue it.  Raw_msg is expected to include the transport goo
	placed in front of the RMR header and payload.
*/
static void buf2mbuf( uta_ctx_t* ctx, char *raw_msg, int msg_size, int sender_fd ) {
	rmr_mbuf_t*		mbuf;
	uta_mhdr_t*		hdr;		// header of the message received
	unsigned char*	d1;			// pointer at d1 data ([0] is the call_id)
	chute_t*		chute;
	unsigned int	call_id;	// the id assigned to the call generated message

	if( PARANOID_CHECKS ) {									// PARANOID mode is slower; off by default
		if( raw_msg == NULL || msg_size <= 0 ) {
			return;
		}
	}

	if( (mbuf = alloc_mbuf( ctx, RMR_ERR_UNSET )) != NULL ) {
		//fprintf( stderr, ">>>>>>> queueing message on context: %p\n", ctx );
		mbuf->tp_buf = raw_msg;
		mbuf->rts_fd = sender_fd;
		if( msg_size > ctx->max_ibm + 1024 ) {
			mbuf->flags |= MFL_HUGE;				// prevent caching of oversized buffers
		}

		ref_tpbuf( mbuf, msg_size );				// point mbuf at bits in the datagram
		hdr = mbuf->header;							// convenience
		if( hdr->flags & HFL_CALL_MSG ) {			// call generated message; ignore call-id etc and queue
			queue_normal( ctx, mbuf );
		} else {
			if( RMR_D1_LEN( hdr ) <= 0 ) {											// no call-id data; just queue
				queue_normal( ctx, mbuf );
			} else {
				d1 = DATA1_ADDR( hdr );
				if( (call_id = (unsigned int) d1[D1_CALLID_IDX]) == 0 ) {			// call_id not set, just queue
					queue_normal( ctx, mbuf );
				} else {
					chute = &ctx->chutes[call_id];
					chute->mbuf = mbuf;
					sem_post( &chute->barrier );				// the call function can vet xaction id in their own thread
				}
			}
		}
	}
}

/*
	Given a buffer, extract the size. We assume the buffer contains one of:
		<int1><int2><mark>
		<int1>

	where <int1> is the size in native storage order (v1) and <int2>
	is the size in network order. If <mark> is present then we assume
	that <int2> is present and we use that after translating from net
	byte order. If <mark> is not present, we use <int1>. This allows
	old versions of RMR to continue to work with new versions that now
	do the right thing with byte ordering.
*/
static inline uint32_t extract_mlen( unsigned char* buf ) {
	uint32_t	size;		// adjusted (if needed) size for return
	uint32_t*	blen;		// length in the buffer to extract

	blen = (uint32_t *) buf;
	if( *(buf + sizeof( int ) * 2 ) == TP_SZ_MARKER ) {
		size = ntohl( *(blen+1) );				// pick up the second integer
		if( DEBUG > 1 ) rmr_vlog( RMR_VL_DEBUG, "extract msg len converted from net order to: %d\n", size );
	} else {
		size = *blen;							// old sender didn't encode size
		if( DEBUG > 1 ) rmr_vlog( RMR_VL_DEBUG, "extract msg len no conversion: %d\n", size );
	}

	return size;
}

/*
	This is the callback invoked when tcp data is received. It adds the data
	to the buffer for the connection and if a complete message is received
	then the message is queued onto the receive ring.

	Return value indicates only that we handled the buffer and SI should continue
	or that SI should terminate, so on error it's NOT wrong to return "ok".
*/
static int mt_data_cb( void* vctx, int fd, char* buf, int buflen ) {
	uta_ctx_t*		ctx;
	river_t*		river;			// river associated with the fd passed in
	unsigned char*	old_accum;		// old accumulator reference should we need to realloc
	int				bidx = 0;		// transport buffer index
	int				remain;			// bytes in transport buf that need to be moved
	int* 			mlen;			// pointer to spot in buffer for conversion to int
	int				need;			// bytes needed for something
	int				i;

	if( PARANOID_CHECKS ) {									// PARANOID mode is slower; off by default
		if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
			return SI_RET_OK;
		}

		if( fd >= ctx->nrivers || fd < 0 ) {
			if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "callback fd is out of range: %d nrivers=%d\n", fd, ctx->nrivers );
			return SI_RET_OK;
		}
	} else {
		ctx = (uta_ctx_t *) vctx;
	}

	if( buflen <= 0 ) {
		return SI_RET_OK;
	}

	river = &ctx->rivers[fd];
	if( river->state != RS_GOOD ) {				// all states which aren't good require reset first
		if( river->state == RS_NEW ) {
			memset( river, 0, sizeof( *river ) );
			river->nbytes = sizeof( char ) * (ctx->max_ibm + 1024);		// max inbound message size
			river->accum = (char *) malloc( river->nbytes );
			river->ipt = 0;
		} else {
			// future -- sync to next marker
			river->ipt = 0;						// insert point
		}
	}

	river->state = RS_GOOD;
	remain = buflen;
	while( remain > 0 ) {								// until we've done something with all bytes passed in
		if( DEBUG )  rmr_vlog( RMR_VL_DEBUG, "====== data callback top of loop bidx=%d msize=%d ipt=%d remain=%d\n", bidx, river->msg_size, river->ipt, remain );

		if( river->msg_size <= 0 ) {				// don't have a size yet
													// FIX ME: we need a frame indicator to ensure alignment
			need = TP_SZFIELD_LEN - river->ipt;		// what we need to compute length
			if( need > remain ) {										// the whole size isn't there
				if( DEBUG > 1 ) rmr_vlog( RMR_VL_DEBUG, "need more for size than we have: need=%d rmain=%d ipt=%d\n", need, remain, river->ipt );
				memcpy( &river->accum[river->ipt], buf+bidx, remain );			// grab what we can and depart
				river->ipt += remain;
				if( DEBUG > 1 ) rmr_vlog( RMR_VL_DEBUG, "data callback not enough bytes to compute size; need=%d have=%d\n", need, remain );
				return SI_RET_OK;
			}

			if( river->ipt > 0 ) {										// if we captured the start of size last go round
				memcpy( &river->accum[river->ipt], buf + bidx, need );
				river->ipt += need;
				bidx += need;
				remain -= need;
				river->msg_size = extract_mlen( river->accum );
				if( DEBUG ) {
					rmr_vlog( RMR_VL_DEBUG, "size from accumulator =%d\n", river->msg_size );
					if( DEBUG > 1 ) {
						dump_40( river->accum, "from accumulator:"  );
						if( river->msg_size > 100 ) {
							dump_40( river->accum + 50, "from rmr buf:"  );
						}
					}
				}
			} else {
				river->msg_size = extract_mlen( &buf[bidx] );			// pull from buf as it's all there; it will copy later
			}
			if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "data callback setting msg size: %d\n", river->msg_size );

			if( river->msg_size > river->nbytes ) {						// message bigger than app max size; grab huge buffer
				//river->flags |= RF_DROP;
				if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "received message is huge (%d) reallocating buffer\n", river->msg_size );
				old_accum = river->accum;					// need to copy any bytes we snarfed getting the size, so hold
				river->nbytes = river->msg_size + 128;					// buffer large enough with a bit of fudge room
				river->accum = (char *) malloc( river->nbytes );
				if( river->ipt > 0 ) {
					memcpy( river->accum, old_accum, river->ipt + 1 );		// copy anything snarfed in getting the sie
				}

				free( old_accum );
			}
		}

		if( river->msg_size > (river->ipt + remain) ) {					// need more than is left in buffer
			if( DEBUG > 1 ) rmr_vlog( RMR_VL_DEBUG, "data callback not enough in the buffer size=%d remain=%d\n", river->msg_size, remain );
			if( (river->flags & RF_DROP) == 0  ) {
				memcpy( &river->accum[river->ipt], buf+bidx, remain );		// buffer and go wait for more
			}
			river->ipt += remain;
			remain = 0;
		} else {
			need = river->msg_size - river->ipt;						// bytes from transport we need to have complete message
			if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "data callback enough in the buffer size=%d need=%d remain=%d flgs=%02x\n", river->msg_size, need, remain, river->flags );
			if( (river->flags & RF_DROP) == 0  ) {
				memcpy( &river->accum[river->ipt], buf+bidx, need );				// grab just what is needed (might be more)
				buf2mbuf( ctx, river->accum, river->nbytes, fd );					// build an RMR mbuf and queue
				river->nbytes = sizeof( char ) * (ctx->max_ibm + 1024);				// prevent huge size from persisting
				river->accum = (char *) malloc( sizeof( char ) *  river->nbytes );	// fresh accumulator
			} else {
				if( !(river->flags & RF_NOTIFIED) ) {
					rmr_vlog( RMR_VL_WARN, "message larger than allocated buffer (%d) arrived on fd %d\n", river->nbytes, fd );
					river->flags |= RF_NOTIFIED;
				}
			}

			river->msg_size = -1;
			river->ipt = 0;
			bidx += need;
			remain -= need;
		}
	}

	if( DEBUG >2 ) rmr_vlog( RMR_VL_DEBUG, "##### data callback finished\n" );
	return SI_RET_OK;
}

/*
	Callback driven on a disconnect notification. We will attempt to find the related
	endpoint via the fd2ep hash maintained in the context. If we find it, then we
	remove it from the hash, and mark the endpoint as closed so that the next attempt
	to send forces a reconnect attempt.

	Future: put the ep on a queue to automatically attempt to reconnect.
*/
static int mt_disc_cb( void* vctx, int fd ) {
	uta_ctx_t*	ctx;
	endpoint_t*	ep;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		return SI_RET_OK;
	}

	ep = fd2ep_del( ctx, fd );		// find ep and remove the fd from the hash
	if( ep != NULL ) {
    	pthread_mutex_lock( &ep->gate );            // wise to lock this
		ep->open = FALSE;
		ep->nn_sock = -1;
    	pthread_mutex_unlock( &ep->gate );
	}

	return SI_RET_OK;
}


/*
	This is expected to execute in a separate thread. It is responsible for
	_all_ receives and queues them on the appropriate ring, or chute.
	It does this by registering the callback function above with the SI world
	and then calling SIwait() to drive the callback when data has arrived.


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
	uta_ctx_t*	ctx;

	if( (ctx = (uta_ctx_t*) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "unable to start mt-receive: ctx was nil\n" );
		return NULL;
	}

	if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "mt_receive: registering SI95 data callback and waiting\n" );

	SIcbreg( ctx->si_ctx, SI_CB_CDATA, mt_data_cb, vctx );			// our callback called only for "cooked" (tcp) data
	SIcbreg( ctx->si_ctx, SI_CB_DISC, mt_disc_cb, vctx );			// our callback for handling disconnects

	SIwait( ctx->si_ctx );

	return NULL;		// keep the compiler happy though never can be reached as SI wait doesn't return
}

#endif

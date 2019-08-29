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
	Mnemonic:	rtable_nng_static.c
	Abstract:	Route table management functions which depend on the underlying
				transport mechanism and thus cannot be included with the generic
				route table functions.

				This module is designed to be included by any module (main) needing
				the static/private stuff.

	Author:		E. Scott Daniels
	Date:		29 November 2018
*/

#ifndef rtable_static_c
#define	rtable_static_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// -----------------------------------------------------------------------------------------------------

/*
	Establish a TCP connection to the indicated target (IP address).
	Target assumed to be address:port.  The new socket is returned via the
	user supplied pointer so that a success/fail code is returned directly.
	Return value is 0 (false) on failure, 1 (true)  on success.

	In order to support multi-threaded user applications we must hold a lock before
	we attempt to create a dialer and connect. NNG is thread safe, but we can
	get things into a bad state if we allow a collision here.  The lock grab
	only happens on the intial session setup.
*/
//static int uta_link2( char* target, nng_socket* nn_sock, nng_dialer* dialer, pthread_mutex* gate ) {
static int uta_link2( endpoint_t* ep ) {
	char* 		target;
	nng_socket*	nn_sock;
	nng_dialer*	dialer;
	char		conn_info[NNG_MAXADDRLEN];	// string to give to nano to make the connection
	char*		addr;
	int			state = FALSE;

	if( ep == NULL ) {
		return FALSE;
	}

	target = ep->addr;
	nn_sock = &ep->nn_sock;
	dialer = &ep->dialer;

	if( target == NULL  ||  (addr = strchr( target, ':' )) == NULL ) {		// bad address:port
		fprintf( stderr, "rmr: link2: unable to create link: bad target: %s\n", target == NULL ? "<nil>" : target );
		return FALSE;
	}

	if( nn_sock == NULL ) {
		errno = EINVAL;
		return FALSE;
	}

	pthread_mutex_lock( &ep->gate );			// grab the lock
	if( ep->open ) {
		pthread_mutex_unlock( &ep->gate );
		return TRUE;
	}


	if( nng_push0_open( nn_sock ) != 0 ) {			// and assign the mode
		pthread_mutex_unlock( &ep->gate );
		fprintf( stderr, "[CRI] rmr: link2: unable to initialise nanomsg push socket to: %s\n", target );
		return FALSE;
	}

	snprintf( conn_info, sizeof( conn_info ), "tcp://%s", target );
	if( (state = nng_dialer_create( dialer, *nn_sock, conn_info )) != 0 ) {
		pthread_mutex_unlock( &ep->gate );
		fprintf( stderr, "[WRN] rmr: link2: unable to create dialer for link to target: %s: %d\n", target, errno );
		nng_close( *nn_sock );
		return FALSE;
	}

	nng_dialer_setopt_ms( *dialer,  NNG_OPT_RECONNMAXT, 2000 );		// cap backoff on retries to reasonable amount (2s)
	nng_dialer_setopt_ms( *dialer,  NNG_OPT_RECONNMINT, 100 );		// start retry 100m after last failure with 2s cap

	if( (state = nng_dialer_start( *dialer, NO_FLAGS )) != 0 ) {						// can fail immediatly (unlike nanomsg)
		pthread_mutex_unlock( &ep->gate );
		fprintf( stderr, "[WRN] rmr: unable to create link to target: %s: %s\n", target, nng_strerror( state ) );
		nng_close( *nn_sock );
		return FALSE;
	}

	if( DEBUG ) fprintf( stderr, "[INFO] rmr_link2l: dial was successful: %s\n", target );

	ep->open = TRUE;						// must set before release
	pthread_mutex_unlock( &ep->gate );
	return TRUE;
}

/*
	This provides a protocol independent mechanism for establishing the connection to an endpoint.
	Return is true (1) if the link was opened; false on error.
*/
static int rt_link2_ep( endpoint_t* ep ) {
	if( ep == NULL ) {
		return FALSE;
	}

	if( ep->open )  {			// already open, do nothing
		return TRUE;
	}

	uta_link2( ep );
	return ep->open;
}


/*
	Add an endpoint to a route table entry for the group given. If the endpoint isn't in the
	hash we add it and create the endpoint struct.

	The caller must supply the specific route table (we assume it will be pending, but they
	could live on the edge and update the active one, though that's not at all a good idea).
*/
extern endpoint_t*  uta_add_ep( route_table_t* rt, rtable_ent_t* rte, char* ep_name, int group  ) {
	endpoint_t*	ep;
	rrgroup_t* rrg;				// pointer at group to update

	if( ! rte || ! rt ) {
		fprintf( stderr, "[WRN] uda_add_ep didn't get a valid rt and/or rte pointer\n" );
		return NULL;
	}

	if( rte->nrrgroups <= group ) {
		fprintf( stderr, "[WRN] uda_add_ep group out of range: %d (max == %d)\n", group, rte->nrrgroups );
		return NULL;
	}

	if( (rrg = rte->rrgroups[group]) == NULL ) {
		if( (rrg = (rrgroup_t *) malloc( sizeof( *rrg ) )) == NULL ) {
			fprintf( stderr, "[WRN] rmr_add_ep: malloc failed for round robin group: group=%d\n", group );
			return NULL;
		}
		memset( rrg, 0, sizeof( *rrg ) );

		if( (rrg->epts = (endpoint_t **) malloc( sizeof( endpoint_t ) * MAX_EP_GROUP )) == NULL ) {
			fprintf( stderr, "[WRN] rmr_add_ep: malloc failed for group endpoint array: group=%d\n", group );
			return NULL;
		}
		memset( rrg->epts, 0, sizeof( endpoint_t ) * MAX_EP_GROUP );

		rte->rrgroups[group] = rrg;

		rrg->ep_idx = 0;						// next endpoint to send to
		rrg->nused = 0;							// number populated
		rrg->nendpts = MAX_EP_GROUP;			// number allocated
	}

	ep = rt_ensure_ep( rt, ep_name ); 			// get the ep and create one if not known

	if( rrg != NULL ) {
		if( rrg->nused >= rrg->nendpts ) {
			// future: reallocate
			fprintf( stderr, "[WRN] endpoint array for mtype/group %d/%d is full!\n", rte->mtype, group );
			return NULL;
		}

		rrg->epts[rrg->nused] = ep;
		rrg->nused++;
	}

	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] endpoint added to mtype/group: %d/%d %s\n", rte->mtype, group, ep_name );
	return ep;
}


/*
	Given a name, find the nano socket needed to send to it. Returns the socket via
	the user pointer passed in and sets the return value to true (1). If the
	endpoint cannot be found false (0) is returned.
*/
static int uta_epsock_byname( route_table_t* rt, char* ep_name, nng_socket* nn_sock ) {
	endpoint_t* ep;
	int state = FALSE;

	if( rt == NULL ) {
		return FALSE;
	}

	ep =  rmr_sym_get( rt->hash, ep_name, 1 );
	if( ep == NULL ) {
		if( DEBUG ) fprintf( stderr, "[DBUG] get ep by name for %s not in hash!\n", ep_name );
		if( ! ep_name || (ep = rt_ensure_ep( rt, ep_name)) == NULL ) {				// create one if not in rt (support rts without entry in our table)
			return FALSE;
		}
	}

	if( ! ep->open )  {										// not open -- connect now
		if( DEBUG ) fprintf( stderr, "[DBUG] get ep by name for %s session not started... starting\n", ep_name );
		if( ep->addr == NULL ) {					// name didn't resolve before, try again
			ep->addr = strdup( ep->name );			// use the name directly; if not IP then transport will do dns lookup
		}
		if( uta_link2( ep ) ) {											// find entry in table and create link
			state = TRUE;
			ep->open = TRUE;
			*nn_sock = ep->nn_sock;							// pass socket back to caller
		}
		if( DEBUG ) fprintf( stderr, "[DBUG] epsock_bn: connection state: %s %s\n", state ? "[OK]" : "[FAIL]", ep->name );
	} else {
		*nn_sock = ep->nn_sock;
		state = TRUE;
	}

	return state;
}

/*
	Make a round robin selection within a round robin group for a route table
	entry. Returns the nanomsg socket if there is a rte for the message
	key, and group is defined. Socket is returned via pointer in the parm
	list (nn_sock).

	The group is the group number to select from.

	The user supplied (via pointer to) integer 'more' will be set if there are
	additional groups beyond the one selected. This allows the caller to
	to easily iterate over the group list -- more is set when the group should
	be incremented and the function invoked again. Groups start at 0.

	The return value is true (>0) if the socket was found and *nn_sock was updated
	and false (0) if there is no associated socket for the msg type, group combination.
	We return the index+1 from the round robin table on success so that we can verify
	during test that different entries are being seleted; we cannot depend on the nng
	socket being different as we could with nano.

	NOTE:	The round robin selection index increment might collide with other
		threads if multiple threads are attempting to send to the same round
		robin group; the consequences are small and avoid locking. The only side
		effect is either sending two messages in a row to, or skipping, an endpoint.
		Both of these, in the grand scheme of things, is minor compared to the
		overhead of grabbing a lock on each call.
*/
static int uta_epsock_rr( rtable_ent_t *rte, int group, int* more, nng_socket* nn_sock ) {
	//rtable_ent_t* rte;			// matching rt entry
	endpoint_t*	ep;				// seected end point
	int  state = FALSE;			// processing state
	int dummy;
	rrgroup_t* rrg;
	int	idx;


	if( ! more ) {				// eliminate cheks each time we need to use
		more = &dummy;
	}

	if( ! nn_sock ) {			// user didn't supply a pointer
		errno = EINVAL;
		*more = 0;
		return FALSE;
	}

	if( rte == NULL ) {
		*more = 0;
		return FALSE;
	}

	if( group < 0 || group >= rte->nrrgroups ) {
		//if( DEBUG ) fprintf( stderr, ">>>> group out of range: group=%d max=%d\n", group, rte->nrrgroups );
		*more = 0;
		return FALSE;
	}

	if( (rrg = rte->rrgroups[group]) == NULL ) {
		//if( DEBUG ) fprintf( stderr, ">>>> rrg not found for group \n", group );
		*more = 0; 					// groups are inserted contig, so nothing should be after a nil pointer
		return FALSE;
	}

	*more = group < rte->nrrgroups-1 ? (rte->rrgroups[group+1] != NULL): 0;	// more if something in next group slot

	switch( rrg->nused ) {
		case 0:				// nothing allocated, just punt
			//if( DEBUG ) fprintf( stderr, ">>>> nothing allocated for the rrg\n" );
			return FALSE;

		case 1:				// exactly one, no rr to deal with
			ep = rrg->epts[0];
			//if( DEBUG ) fprintf( stderr, ">>>> _rr returning socket with one choice in group \n" );
			state = TRUE;
			break;

		default:										// need to pick one and adjust rr counts

			idx = rrg->ep_idx++ % rrg->nused;			// see note above
			ep = rrg->epts[idx];						// select next endpoint
			//if( DEBUG ) fprintf( stderr, ">>>> _rr returning socket with multiple choices in group idx=%d \n", rrg->ep_idx );
			state = idx + 1;							// unit test checks to see that we're cycling through, so must not just be TRUE
			break;
	}

	if( state ) {										// end point selected, open if not, get socket either way
		if( ! ep->open ) {								// not connected
			if( ep->addr == NULL ) {					// name didn't resolve before, try again
				ep->addr = strdup( ep->name );			// use the name directly; if not IP then transport will do dns lookup
			}

			if( uta_link2( ep ) ) {											// find entry in table and create link
				ep->open = TRUE;
				*nn_sock = ep->nn_sock;							// pass socket back to caller
			} else {
				state = FALSE;
			}
			if( DEBUG ) fprintf( stderr, "[DBUG] epsock_rr: connection attempted with %s: %s\n", ep->name, state ? "[OK]" : "[FAIL]" );
		} else {
			*nn_sock = ep->nn_sock;
		}
	}

	return state;
}

/*
	Finds the rtable entry which matches the key. Returns a nil pointer if
	no entry is found. If try_alternate is set, then we will attempt 
	to find the entry with a key based only on the message type.
*/
static inline rtable_ent_t*  uta_get_rte( route_table_t *rt, int sid, int mtype, int try_alt ) {
	uint64_t key;			// key is sub id and mtype banged together
	rtable_ent_t* rte;		// the entry we found

	if( rt == NULL || rt->hash == NULL ) {
		return NULL;
	}

	key = build_rt_key( sid, mtype );											// first try with a 'full' key
	if( ((rte = rmr_sym_pull( rt->hash, key )) != NULL)  ||  ! try_alt ) {		// found or not allowed to try the alternate, return what we have
		return rte;
	}

	if( sid != UNSET_SUBID ) {								// not found, and allowed to try alternate; and the sub_id was set
		key = build_rt_key( UNSET_SUBID, mtype );			// rebuild key
		rte = rmr_sym_pull( rt->hash, key );				// see what we get with this
	}

	return rte;
}

#endif

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
	Mnemonic:	rtable_static.c
	Abstract:	Route table management functions.
	Author:		E. Scott Daniels
	Date:		29 November 2018
*/

#ifndef rtable_static_c
#define rtable_static_c

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



/*
	Establish a TCP connection to the indicated target (IP address).
	Target assumed to be address:port. Requires a separate nano socket;
	the socket number (for future sends) is returned or -1 on error.
*/
static int uta_link2( char* target ) {
	char	conn_info[NN_SOCKADDR_MAX];	// string to give to nano to make the connection
	int		nn_sock;					// the nano socket for this link
	char*	addr;

	if( target == NULL  ||  (addr = strchr( target, ':' )) == NULL ) {		// bad address:port
		fprintf( stderr, "[INFO] rmr: rmr_link2: unable to create link: invalid target: %s\n", target == NULL ? "<nil>" : target );
		return -1;
	}

    nn_sock = nn_socket( AF_SP, NN_PUSH );		// the socket we'll use to connect to the target
	if( nn_sock < 0 ) {
		fprintf( stderr, "[WARN] rmr: link2: unable to create socket for link to target: %s: %d\n", target, errno );
		return -1;
	}

	snprintf( conn_info, sizeof( conn_info ), "tcp://%s", target );
    if( nn_connect( nn_sock, conn_info ) < 0 ) {							// connect failed
		fprintf( stderr, "[WARN] rmr: link2: unable to create link to target: %s: %d\n", target, errno );
		nn_close( nn_sock );
		return -1;
	}

	return nn_sock;
}

/*
	This provides a protocol independent mechanism for establishing the connection to an endpoint.
	Returns true on success; false otherwise.
*/
static int rt_link2_ep( endpoint_t* ep ) {
	if( ep == NULL ) {
		return FALSE;
	}

	if( ep->open ) {
		return TRUE;
	}

	ep->nn_sock =  uta_link2( ep->addr ) >= 0;			// open if a valid socket returned
	ep->open = ep->nn_sock >= 0;
	return ep->open;
}

/*
	Add an endpoint to a route table entry for the group given. If the endpoint isn't in the 
	hash we add it and create the endpoint struct.

	The caller must supply the specific route table (we assume it will be pending, but they
	could live on the edge and update the active one, though that's not at all a good idea).
*/
static endpoint_t*  uta_add_ep( route_table_t* rt, rtable_ent_t* rte, char* ep_name, int group  ) {
	endpoint_t*	ep;
	rrgroup_t* rrg;				// pointer at group to update

	if( ! rte || ! rt ) {
		fprintf( stderr, "[WARN] rmr_add_ep didn't get a valid rt and/or rte pointer\n" );
		return NULL;
	}

	if( rte->nrrgroups <= group ) {
		fprintf( stderr, "[WARN] rmr_add_ep group out of range: %d (max == %d)\n", group, rte->nrrgroups );
		return NULL;
	}

	if( (rrg = rte->rrgroups[group]) == NULL ) {
		if( (rrg = (rrgroup_t *) malloc( sizeof( *rrg ) )) == NULL ) {
			fprintf( stderr, "[WARN] rmr_add_ep: malloc failed for round robin group: group=%d\n", group );
			return NULL;
		}
		memset( rrg, 0, sizeof( *rrg ) );

		if( (rrg->epts = (endpoint_t **) malloc( sizeof( endpoint_t ) * MAX_EP_GROUP )) == NULL ) {
			fprintf( stderr, "[WARN] rmr_add_ep: malloc failed for group endpoint array: group=%d\n", group );
			return NULL;
		}
		memset( rrg->epts, 0, sizeof( endpoint_t ) * MAX_EP_GROUP );

		rte->rrgroups[group] = rrg;

		rrg->ep_idx = 0;						// next to send to
		rrg->nused = 0;							// number populated
		rrg->nendpts = MAX_EP_GROUP;			// number allocated
	}

	if( (ep = uta_get_ep( rt, ep_name )) == NULL ) { 					// not there yet, make
		if( (ep = (endpoint_t *) malloc( sizeof( *ep ) )) == NULL ) {
			fprintf( stderr, "uta: [WARN] malloc failed for endpoint creation: %s\n", ep_name );
			return NULL;
		}

		ep->nn_sock = -1;					// not connected
		ep->addr = uta_h2ip( ep_name );
		ep->name = strdup( ep_name );

		rmr_sym_put( rt->hash, ep_name, 1, ep );
	}

	if( rrg != NULL ) {
		if( rrg->nused >= rrg->nendpts ) {
			// future: reallocate
			fprintf( stderr, "[WARN] endpoint array for mtype/group %d/%d is full!\n", rte->mtype, group );
			return NULL;
		}

		rrg->epts[rrg->nused] = ep;
		rrg->nused++;
	}

	if( DEBUG > 1 ) fprintf( stderr, "[DBUG] endpoint added to mtype/group: %d/%d %s\n", rte->mtype, group, ep_name );
	return ep;
}

/*
	Given a name, find the nano socket needed to send to it. Returns the socket number if
	found; -1 on error.
*/
static int uta_epsock_byname( route_table_t* rt, char* ep_name ) {
	endpoint_t* ep;

	if( rt == NULL ) {
		return -1;
	}

	ep =  rmr_sym_get( rt->hash, ep_name, 1 );
	if( ep == NULL ) {
		if( ! ep_name || (ep = rt_ensure_ep( rt, ep_name)) == NULL ) {				// create one if not in rt (support rts without entry in our table)
			return -1;
		}
	}

	if( !ep->open  ) {								// not connected; must connect now
		if( ep->addr == NULL ) {					// name didn't resolve before, try again
			ep->addr = uta_h2ip( ep->name );
		}
		ep->nn_sock = uta_link2( ep->addr );
		ep->open = ep->nn_sock >= 0;
		if( DEBUG ) fprintf( stderr, "[DBUG] epsock_bn: connection state: %s %s\n", ep->nn_sock >= 0 ? "[OK]" : "[FAIL]", ep->name );
	}

	return ep->nn_sock;
}

/*
	Make a round robin selection within a round robin group for a route table
	entry. Returns the nanomsg socket number if there is a rte for the message
	type, and group is defined, else returns -1.

	The group is the group number to select from. 

	The user supplied integer 'more' will be set if there are additional groups
	defined to the matching route table entry which have a higher group number.
	This assumes the caller is making a sequential pass across groups starting
	with group 0. If more is set, the caller may increase the group number and
	invoke this function again to make a selection against that group. If there
	are no more groups, more is set to 0.
*/
static int uta_epsock_rr( route_table_t *rt, int mtype, int group, int* more ) {
	rtable_ent_t* rte;			// matching rt entry
	endpoint_t*	ep;				// seected end point
	int nn_sock = -1;
	int dummy;
	rrgroup_t* rrg;

	if( ! more ) {				// eliminate cheks each time we need to user
		more = &dummy;
	}

	if( rt == NULL ) {
		*more = 0;
		return -1;
	}

	if( (rte = rmr_sym_pull( rt->hash, mtype )) == NULL ) {
		*more = 0;
		//if( DEBUG ) fprintf( stderr, ">>>> rte not found for type = %d\n", mtype );
		return -1;
	}

	if( group < 0 || group >= rte->nrrgroups ) {
		//if( DEBUG ) fprintf( stderr, ">>>> group out of range: mtype=%d group=%d max=%d\n", mtype, group, rte->nrrgroups );
		*more = 0;
		return -1;
	}

	if( (rrg = rte->rrgroups[group]) == NULL ) {
		//if( DEBUG ) fprintf( stderr, ">>>> rrg not found for type = %d\n", mtype );
		*more = 0; 					// groups are inserted contig, so nothing should be after a nil pointer
		return -1;
	}

	*more = group < rte->nrrgroups-1 ? (rte->rrgroups[group+1] != NULL): 0;	// more if something in next group slot

	switch( rrg->nused ) {
		case 0:				// nothing allocated, just punt
			//if( DEBUG ) fprintf( stderr, ">>>> nothing allocated for the rrg\n" );
			return -1;

		case 1:				// exactly one, no rr to deal with and more is not possible even if fanout > 1
			nn_sock = rrg->epts[0]->nn_sock;
			ep = rrg->epts[0];
			break;
	
		default:										// need to pick one and adjust rr counts
			ep = rrg->epts[rrg->ep_idx];
			nn_sock = rrg->epts[rrg->ep_idx++]->nn_sock;
			if( rrg->ep_idx >= rrg->nused ) {
				rrg->ep_idx = 0;
			}
			break;
	}

	if( ! ep->open ) {				// not connected
		if( ep->addr == NULL ) {					// name didn't resolve before, try again
			ep->addr = uta_h2ip( ep->name );
		}
		ep->nn_sock = nn_sock = uta_link2( ep->addr );
		ep->open = ep->nn_sock >= 0;
		if( DEBUG ) fprintf( stderr, "[DBUG] epsock_rr: connection state to %s: %s\n", ep->name, nn_sock >= 0 ? "[OK]" : "[FAIL]" );
	}

	return nn_sock;
}


#endif

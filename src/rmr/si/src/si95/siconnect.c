// vim: noet sw=4 ts=4:
/*
==================================================================================
    Copyright (c) 2020 Nokia
    Copyright (c) 2020 AT&T Intellectual Property.

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
***************************************************************************
*
*  Mnemonic: 	SIconnect
*  Abstract: 	This module contains functions to make the connection using
*				a transport block which has been given a transport (tcp) family
*				address structure.
*
*  Date:		March 1995
*  Author:		E. Scott Daniels
*
*  Mod:			08 Mar 2007 - conversion of sorts to support ipv6
*				17 Apr 2020 - Add safe connect capabilities
******************************************************************************
*/
#include "sisetup.h"
#include "sitransport.h"

// ---------------- internal functions ----------------------------------------------

/*
	Attempts a connection to addr, and ensures that the linux even port
	connect bug does not establish the connection back to our process.
	If we detect this, then we abort the connection and return -1. 0
	returned on a good connection.

	If we are hit with the even port connection bug we have no choice but
	to abort the connection which will CLOSE the caller's FD. This may
	not be expected in some situations, and when we do the errno is set to
	EBADFD to indicate this. We ensure that this state is returned ONLY
	if the failure results in a closed fd which the caller will need to reopen.
*/
int safe_connect( int fd, struct sockaddr* addr, int alen ) {
	int state = 0;
	char	caddr[255];			// work buffer for get sock name (v6 addr ~28 bytes, so this is plenty)
	int		calen;				// len of the connect generated address

	if( (state = CONNECT( fd, addr, alen )) != 0 ) {
		if( errno == EBADFD ) {			// ensure we return bad fd ONLY if we abort later
			errno = ECONNABORTED;
		}
		return state;
	}

	if( PARANOID_CHECKS ) {
		if( alen > sizeof( caddr ) ) {		// shouldn't happen, but be safe
			fprintf( stderr, "safe_connect: address buffer for connect exceeds work space %d > %lu\n", alen, sizeof( caddr ) );
			errno = E2BIG;
			return -1;
		}
	}

	calen = alen;			// we assume a bound address will have the same type, and thus len, as the connect to addr
	if( getsockname( fd, (struct sockaddr *) &caddr, &calen ) == 0 ) {
		if( calen != alen || memcmp( addr, &caddr, calen ) != 0 ) {			// addresses differ, so it's good
			errno = 0;
			return 0;
		}
	}

	siabort_conn( fd );
	errno = EBADFD;
	return -1;
}

/*
	Accept a file descriptor and add it to the map.
*/
extern void SImap_fd( struct ginfo_blk *gptr, int fd, struct tp_blk* tpptr ) {
	if( fd < MAX_FDS ) {
		gptr->tp_map[fd] = tpptr;
	} else {
		rmr_vlog( RMR_VL_WARN, "fd on connected session is out of range: %d\n", fd );
	}
}

/*
	Creates a connection to the target endpoint using the address in the
	buffer provided.  The address may be one of three forms:
		hostname:port
		IPv4-address:port
		[IPv6-address]:port

	On success the open file descriptor is returned; else -1 is returned. Errno
	will be left set by the underlying connect() call.

	To avoid the even port connect bug in the linux connect() systeem call,
	we will use safe_connect() if indicated during the connection prep
	process.
*/
extern int SIconnect( struct ginfo_blk *gptr, char *abuf ) {
	int status;
	struct tp_blk *tpptr;       	//  pointer to new block
	struct sockaddr *taddr;     	// convenience pointer to addr of target
	int alen = 0;					//  len of address struct
	int fd = SI_ERROR;             	//  file descriptor to return to caller

	if( PARANOID_CHECKS ) {
		if( gptr == NULL ) {
			return SI_ERROR;
		}

		if( gptr->magicnum != MAGICNUM ) {		// no cookie -- no connection
			return SI_ERROR;
		}
	}

	tpptr = SIconn_prep( gptr, TCP_DEVICE, abuf, 0 );			// create tp struct, and socket. get peer address 0 == any family that suits the addr
	if( tpptr != NULL ) {
		taddr = tpptr->paddr;
		errno = 0;
		if( tpptr->flags & TPF_SAFEC ) {
			if( safe_connect( tpptr->fd, taddr, tpptr->palen ) != 0 ) {		// fd closed on failure
				SItrash( TP_BLK, tpptr );
				tpptr->fd = -1;
			}
		} else {
			if( CONNECT( tpptr->fd, taddr, tpptr->palen ) != 0 ) {
				CLOSE( tpptr->fd );     			// clean up fd and tp_block
				tpptr->fd = -1;
				SItrash( TP_BLK, tpptr );       	// free the trasnsport block
			}
		}

		if( tpptr->fd >= 0 ) {								// connect ok
			tpptr->flags |= TPF_SESSION;    		//  indicate we have a session here
			tpptr->next = gptr->tplist;     		//  add block to the list
			if( tpptr->next != NULL ) {
				tpptr->next->prev = tpptr;     		//  if there - point back at new
			}

			gptr->tplist = tpptr;           		//  point at new head
			fd = tpptr->fd;                 		//  save for return value
			SImap_fd( gptr, fd, tpptr );
		}
	}

 	return fd;
}

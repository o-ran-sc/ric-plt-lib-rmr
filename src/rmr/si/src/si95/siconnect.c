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
*  Abstract: 	Start a TCP/IP session with another process.
*  Parms:    
*            	addr - Pointer to a string containing the process' address
*				The address is either ipv4 or ipv6 formmat with the
*				port number separated with a semicolon (::1;4444,
*				localhost;4444 climber;4444 129.168.0.4;4444).
*  Returns:  	The session number if all goes well, SI_ERROR if not.
*
*  Date:		March 1995
*  Author:		E. Scott Daniels
*
*  Mod:		08 Mar 2007 - conversion of sorts to support ipv6
******************************************************************************
*/
#include "sisetup.h"
#include "sitransport.h"

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

extern int SIconnect( struct ginfo_blk *gptr, char *abuf ) {
	int status;
	struct tp_blk *tpptr;       	//  pointer to new block 
	struct sockaddr *taddr;     	// convenience pointer to addr of target
	int alen = 0;					//  len of address struct 
	int fd = SI_ERROR;             	//  file descriptor to return to caller 

	if( PARINOID_CHECKS ) {
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
		if( connect( tpptr->fd, taddr, tpptr->palen ) != 0 ) {
			close( tpptr->fd );     			//  clean up fd and tp_block 
			SItrash( TP_BLK, tpptr );       	//  free the trasnsport block 
			fd = SI_ERROR;             			//  send bad session id num back 
		} else  {                      			//  connect ok 
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

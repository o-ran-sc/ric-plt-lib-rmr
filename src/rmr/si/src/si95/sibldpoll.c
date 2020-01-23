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
*****************************************************************************
*  Mnemonic: SIbldpoll
*  Abstract: This routine will fill in the read and write fdsets in the
*            general info struct based on the current transport provider
*            list. Those tb blocks that have something queued to send will
*            be added to the write fdset. The fdcount variable will be set to
*            the highest sid + 1 and it can be passed to the select system
*            call when it is made.
*
*  Parms:    gptr  - Pointer to the general info structure
*  Returns:  Nothing
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
***************************************************************************
*/
#include "sisetup.h"      //  get definitions etc 
#include "sitransport.h"

extern void SIbldpoll( struct ginfo_blk* gptr  ) {
	struct tp_blk *tpptr;					//  pointer into tp list 
	struct tp_blk *nextb;					//  pointer into tp list 


	gptr->fdcount = -1;					//  reset largest sid found 

	FD_ZERO( &gptr->readfds );			//  reset the read and write sets 
	FD_ZERO( &gptr->writefds );
	FD_ZERO( &gptr->execpfds );

	for( tpptr = gptr->tplist; tpptr != NULL; tpptr = nextb ) {
		nextb = tpptr->next;
		if( tpptr->flags & TPF_DELETE ) {
			SIterm( gptr, tpptr );
		} else {
			if( tpptr->fd >= 0 ) {                       //  if valid file descriptor 
				if( tpptr->fd >= gptr->fdcount ) {	
					gptr->fdcount = tpptr->fd + 1;     //  save largest fd (+1) for select 
				}

				FD_SET( tpptr->fd, &gptr->execpfds );     //  set all fds for execpts 

				if( !(tpptr->flags & TPF_DRAIN) ) {                  //  if not draining 
					FD_SET( tpptr->fd, &gptr->readfds );       //  set test for data flag 
				}

				if( tpptr->squeue != NULL ) {                  //  stuff pending to send ? 
					FD_SET( tpptr->fd, &gptr->writefds );   //  set flag to see if writable 
				}
			}
		}
	}
}

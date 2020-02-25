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
**************************************************************************
*  Mnemonic:	SIterm
*  Abstract:	Manage the transport provider block information relating to
*				the need to terminate the session. The block is left in the
*				list; it is unsafe to clean the lsit up outside of the SIwait 
*				thread.  When safe, the SIrm_tpb() function can be called to
*				do the rest of the work that was originally done by SIterm.
*
*  Date:     	18 January 1995
*  Author:		E. Scott Daniels
*
**************************************************************************
*/
#include "sisetup.h"     //  get the setup stuff 
#include "sitransport.h"

/*
	Close the FD and mark the transport block as unusable/closed.
	Removal of the block from the list is safe only from the siwait
	thread.
*/
extern void SIterm( struct ginfo_blk* gptr, struct tp_blk *tpptr ) {

	if( tpptr != NULL ) {
		if( tpptr->fd >= 0 ) {
			CLOSE( tpptr->fd );    
			if( tpptr->fd < MAX_FDS ) {
				gptr->tp_map[tpptr->fd] = NULL;		// drop reference
			}
		}

		tpptr->fd = -1;								// prevent future sends etc.
		tpptr->flags |= TPF_DELETE;					// signal block deletion needed when safe
	}
}

/*
	It is safe to remove the block from the list; if it was in the list
	in the first place. 
*/
extern void SIrm_tpb( struct ginfo_blk *gptr, struct tp_blk *tpptr ) {

	if( tpptr != NULL ) {
		if( tpptr->prev != NULL || tpptr->next != NULL ) {	// in the list
			if( tpptr->prev != NULL ) {            //  remove from the list 
				tpptr->prev->next = tpptr->next;    //  point previous at the next 
			} else {
				gptr->tplist = tpptr->next;        //  this was head, make next new head 
			}
	
			if( tpptr->next != NULL ) {
				tpptr->next->prev = tpptr->prev;  //  point next one back behind this one 
			}
		}

		free( tpptr->addr );             //  release the address bufers 
		free( tpptr->paddr );
		free( tpptr );                   //  and release the block 
	}
}

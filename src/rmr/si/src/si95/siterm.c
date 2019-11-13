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
*  Mnemonic: SIterm
*  Abstract: This routine will terminate a session based on the tp_blk
*            that is passed into the routine. The transport session block
*            is released and removed from the ginfo list. The session is
*            terminated by issuing a t_unbind call (if the unbind flag is
*            on in the tpptr block), and then issuing a t_close.
*  Parms:    gptr - Pointer to the global information block
*            tpptr - Pointer to tp block that defines the open fd.
*  Returns:  Nothing.
*  Date:     18 January 1995
*  Author:   E. Scott Daniels
*
**************************************************************************
*/
#include "sisetup.h"     //  get the setup stuff 
#include "sitransport.h"

extern void SIterm( struct ginfo_blk* gptr, struct tp_blk *tpptr ) {

	if( tpptr != NULL ) {
		if( tpptr->fd >= 0 ) {
			CLOSE( tpptr->fd );    
			if( tpptr->fd < MAX_FDS ) {
				gptr->tp_map[tpptr->fd] = NULL;		// drop reference
			}
		}

		if( tpptr->prev != NULL ) {            //  remove from the list 
			tpptr->prev->next = tpptr->next;    //  point previous at the next 
		} else {
			gptr->tplist = tpptr->next;        //  this was head, make next new head 
		}

		if( tpptr->next != NULL ) {
			tpptr->next->prev = tpptr->prev;  //  point next one back behind this one 
		}

		free( tpptr->addr );             //  release the address bufers 
		free( tpptr->paddr );
		free( tpptr );                   //  and release the block 
	}
}

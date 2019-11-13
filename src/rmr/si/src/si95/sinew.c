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
*******************************************************************************
*
*  Mnemonic: SInew
*  Abstract: This routine is responsible for alocating a new block based on
*            the block type and initializing it.
*  Parms:    type - Block id to create
*  Returns:  Pointer to the new block or NULL if not successful
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*  Mod:		22 Feb 2002 - To ensure new field in tp block is initialised
*
******************************************************************************
*/
#include "sisetup.h"

extern void *SInew( int type ) {
	void *retptr;                  //  generic pointer for return
	struct tp_blk *tpptr;          //  pointer at a new tp block
	struct ginfo_blk *gptr;        //  pointer at gen info blk
	struct ioq_blk *qptr;          //  pointer to an I/O queue block

 switch( type ) {
		case IOQ_BLK:              //  make an I/O queue block
			if( (qptr = (struct ioq_blk *) malloc( sizeof( struct ioq_blk) )) != NULL ) {
				qptr->addr = NULL;
				qptr->next = NULL;
				qptr->data = NULL;
				qptr->dlen = 0;
			}
			retptr = (void *) qptr;    //  set pointer for return
			break;

		case TP_BLK:
			if( (tpptr = (struct tp_blk *) malloc( sizeof( struct tp_blk ) )) != NULL ) {
				memset( tpptr, 0, sizeof( *tpptr ) );
				tpptr->fd = -1;
				tpptr->type = -1;
				tpptr->flags = TPF_UNBIND;   //  default to unbind on termination
			}
			retptr = (void *) tpptr;   //  setup for later return
			break;

		case GI_BLK:                //  create global info block
			if( (gptr = (struct ginfo_blk *) malloc( sizeof( struct ginfo_blk ) )) != NULL ) {
				memset( gptr, 0, sizeof( *gptr ) );

				gptr->magicnum = MAGICNUM;   //  inidicates valid block
				gptr->flags = 0;
				gptr->tplist = NULL;
				FD_ZERO( &gptr->readfds);      //  clear the fdsets
				FD_ZERO( &gptr->writefds) ;
				FD_ZERO( &gptr->execpfds );
				gptr->rbuf = NULL;             //  no read buffer
				gptr->cbtab = NULL;
				gptr->rbuflen = 0;
			}

    		retptr = (void *) gptr;    //  set up for return at end
    		break;

		default:
			retptr = NULL;           //  bad type - just return null
    		break;
	}                          //  end switch

	return( retptr );           //  send back the new pointer
}

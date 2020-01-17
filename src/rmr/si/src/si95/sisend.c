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
******************************************************************************
*
*  Mnemonic: SIsend
*  Abstract: This routine is called to send a buffer of data to a partner
*            if the buffer has been queued waiting for the session to
*            unblock. The top qio block is removed from the tp block's
*            queue and is sent. It is assumed that for UDP data the
*            unit data structure was created and contains the buffer and
*            address and is pointed to by the qio block. The block and
*            associated buffers are then freed.
*  Parms:
*            tpptr- Pointer to the tp block
*
*  Returns:	Nothing.
*  Date:	27 March 1995
*  Author:	E. Scott Daniels
*  Mod:		22 Feb 2002 - To support sendqueue tail 
*
******************************************************************************
*/
#include "sisetup.h"      //  get include files etc 
#include "sitransport.h"

extern void SIsend( struct ginfo_blk *gptr, struct tp_blk *tpptr ) {
	struct t_unitdata *udata;      //  pointer at UDP unit data 
	struct ioq_blk *qptr;          //  pointer at qio block for free 
	int status;
//static int announced = 0;	// TESTING

	if( tpptr->squeue == NULL )    //  who knows why we were called 
		return;                        //  nothing queued - just leave 

/*
	if( tpptr->type == SOCK_DGRAM ) {                                //  udp send?  
		sendto( tpptr->fd, tpptr->squeue->data, tpptr->squeue->dlen, 0, tpptr->squeue->addr, sizeof( struct sockaddr ) );
		if( tpptr->squeue->addr != NULL )
			free( tpptr->squeue->data );
		tpptr->squeue->addr = NULL;
	} else {
*/
		status= SEND( tpptr->fd, tpptr->squeue->data, tpptr->squeue->dlen, 0 );
/*
	}
*/


/*
//TESTING
if( !announced && status < tpptr->squeue->dlen ) {
announced = 1;
fprintf( stderr, ">>>>>>> !!!!!! SIsend: short send: %d != %d\n", status, tpptr->squeue->dlen );
}
*/
	free( tpptr->squeue->data );           //  trash buffer or the udp block 
	qptr = tpptr->squeue;                  //  hold pointer for free 
	tpptr->squeue = tpptr->squeue->next;   //  next in queue becommes head 
	if( !tpptr->squeue )
		tpptr->sqtail = NULL;		//  no tail left either 

	free( qptr );

	if( (tpptr->flags & TPF_DRAIN) && tpptr->squeue == NULL )  //  done w/ drain? 
	{
		SIterm( gptr, tpptr );     //  trash the tp block 
	}
}                      //  SIsend 

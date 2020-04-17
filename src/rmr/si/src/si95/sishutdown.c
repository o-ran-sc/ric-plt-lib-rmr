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
*  Mnemonic: SIshutdown
*  Abstract: Shutdown and abort functions.
*
*  Date:     23 March 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include "sisetup.h"                   //  get includes and defines 

/*
*/
static void sishutdown( struct ginfo_blk *gptr, int flags ) {
	struct tp_blk*	tpb;

	if( gptr != NULL && gptr->magicnum == MAGICNUM )
	{
 		gptr->flags |=  GIF_SHUTDOWN;    //  signal shutdown 
		for( tpb = gptr->tplist; tpb != NULL; tpb = tpb->next )
		{
			tpb->flags |= (TPF_UNBIND | flags);    //  force unbind on session  and set caller flags
			SIterm( gptr, tpb );					// term marks ok to delete but does NOT remove it
		}
	}
}            

/*
	Run the list of known transport sessions and close them gracefully. This
	will result in time-waits which might prevent the application from
	restarting immediately as the listen port(s) might not be usable.
*/
extern void SIshutdown( struct ginfo_blk *gptr ) {
	sishutdown( gptr, 0 );
}            

/*
	Run the list of known transport sessions and close them by aborting
	(resetting the connection). This can result in buffered, but untransmitted,
	data from being lost; the risk should be known by the caller.
*/
extern void SIabort( struct ginfo_blk *gptr ) {
	sishutdown( gptr, TPF_ABORT );
}            

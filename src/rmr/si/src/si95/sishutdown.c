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
*  Abstract: This routine will ensure that all tp blocks have been closed
*            with the transport provider and removed from the list. The
*            shutdown flag is set in addition.
*  Parms:    gptr - pointer to the ginfo structure (SIHANDLE)
*  Retrns:   Nothing.
*  Date:     23 March 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include "sisetup.h"                   //  get includes and defines 

extern void SIshutdown( struct ginfo_blk *gptr ) {
	gptr->sierr = SI_ERR_HANDLE;
	if( gptr != NULL && gptr->magicnum == MAGICNUM )
	{
 		gptr->flags |=  GIF_SHUTDOWN;    //  signal shutdown 
		while( gptr->tplist != NULL )
		{
			gptr->tplist->flags |= TPF_UNBIND;    //  force unbind on session 
			SIterm( gptr, gptr->tplist );         //  and drop the session 
		}                                      //  end while 
		gptr->sierr = 0;
	}
}            

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
*  Mnemonic: SIgetaddr
*  Abstract: This routine will get the address of the first listening
*            block on the tp list and return it in ASCII format to the
*            caller.
*  Parms:    gptr - Pointer to the global information block
*            buf  - Pointer to the buffer to hold the ascii string
*  Returns:  NI_OK if block found, NI_ERROR if no listen block exists
*  Date:     18 April 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include "sisetup.h"        //  get the standard include stuff 

extern int SIgetaddr( struct ginfo_blk *gptr, char *buf ) {
	struct tp_blk *tpptr;       //  Pointer into tp list 
	int status = SI_ERROR;       //  return status 
	char	*ibuf;		//  SIaddr now points us at a string, rather than filling ours 

 	for( tpptr = gptr->tplist; tpptr != NULL && !(tpptr->flags & TPF_LISTENFD);
      		tpptr = tpptr->next );

 	if( tpptr != NULL )
  	{
   		SIaddress( tpptr->addr, (void *) &ibuf, AC_TODOT );   //  convert to dot fmt 
		strcpy( buf, ibuf );				//  copy into caller's buffer 
		free( ibuf );
   		status = SI_OK;                               //  ok status for return 
  	}

 	return status;
}          

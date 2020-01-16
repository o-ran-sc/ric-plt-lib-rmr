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

/* X
*****************************************************************************
*
*  Mnemonic: SIcbstat
*  Abstract: This routine is responsible for the generic handling of
*            the return status from a call back routine.
*  Parms:    gptr - pointer to the ginfo block
*            status - The status that was returned by the call back
*            type   - Type of callback (incase unregister)
*  Returns:  Nothing.
*  Date:     23 January 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include "sisetup.h"     //  get necessary defs etc 

extern void SIcbstat( struct ginfo_blk *gptr, int status, int type )
{

 switch( status )
  {
   case SI_RET_UNREG:                   //  unregister the callback 
    gptr->cbtab[type].cbrtn = NULL;     //  no pointer - cannot call 
    break;

   case SI_RET_QUIT:                 //  callback wants us to stop 
    gptr->flags |= GIF_SHUTDOWN;    //  so turn on shutdown flag 
    break;

   default:                 //  ignore the others 
    break;
  }   //  end switch 
}         //  SIcbstat 

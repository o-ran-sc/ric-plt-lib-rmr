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
****************************************************************************
*
*  Mnemonic: SIcbreg
*  Abstract: This routine will register a callback in the table. Callbacks
*            are "unregistered" by passing a null function pointer.
*  Parms:    gptr - pointer to the general information block (SIHANDLE)
*            type - Type of callback to register (SI_CB_xxxxx)
*            fptr - Pointer to the function to register
*            dptr - Pointer that the user wants the callback function to get
*  Returns:  Nothing.
*  Date:     23 January 1995
*  Author:   E. Scott Daniels
*
****************************************************************************
*/
#include "sisetup.h"     //  get defs and stuff 

extern void SIcbreg( struct ginfo_blk *gptr, int type, int ((*fptr)()), void * dptr ) {

	if( gptr == NULL ) {
		fprintf( stderr, "[ERR] SIcbreg -- gptr was nil\n" );
		exit( 1 );
	}

	 if( gptr->magicnum == MAGICNUM ) {    			//  valid block from user ? 
		if( type >= 0 && type < MAX_CBS ) {   		//  if cb type is in range 
			gptr->cbtab[type].cbdata = dptr;   		//  put in data 
			gptr->cbtab[type].cbrtn = fptr;    		//  save function ptr  
		}
	}
}

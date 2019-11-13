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
***************************************************************************
*  
*  Mnemonic: sitrash
*  Abstract: Delete all things referenced by a struct and then free the memory.
*    
*  Returns:  Nothing
*  Date:     08 March 2007
*  Author:   E. Scott Daniels                            
*
******************************************************************************   
*/

#include        "sisetup.h"
#include		"sitransport.h"
                 
extern void SItrash( int type, void *bp )
{
        struct tp_blk *tp = NULL;
        struct ioq_blk *iptr;
        struct ioq_blk *inext;

        switch( type )
        {
                case IOQ_BLK:
                        iptr = (struct ioq_blk *) bp;
                        free( iptr->data );
                        free( iptr->addr );
                        free( iptr );
                        break;
            
                case TP_BLK:                                            //  we assume its off the list 
                        tp = (struct tp_blk *) bp;
                        for( iptr = tp->squeue; iptr; iptr = inext )            //  trash any pending send buffers 
                        {
                                inext = iptr->next;
                                free( iptr->data );          //  we could recurse, but that seems silly 
                                free( iptr->addr );
                                free( iptr );
                        }

						if( tp->fd >= 0 ) {
							CLOSE( tp->fd );
						}
     
                        free( tp->addr );             //  release the address bufers 
                        free( tp->paddr );        
                        free( tp );                   //  and release the block 
                        break;
        }
}


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
******************************************************************************
*
*  Mnemonic: SIclose
*  Abstract: This routine allows the user application to close a port
*            associated with a file descriptor. The port is unbound from
*            the transport providor even if it is marked as a listen
*            port. If the fd passed in is less than 0 this routine assumes
*            that the UDP port opened during init is to be closed (user never
*            receives a fd on this one).
*  Parms:    gptr - The pointer to the ginfo block (SIHANDLE to the user)
*            fd   - FD to close.
*  Returns:  SI_OK if all goes well, SI_ERROR with SIerrno set if there is
*            a problem.
*  Date:     3 February 1995
*  Author:   E. Scott Daniels
*
*  Modified: 19 Feb 1995 - To set TP blk to drain if output pending.
*            10 May 1995 - To change SOCK_RAW to SOCK_DGRAM
*		22 Feb 2002 - To accept TCP_LISTEN_PORT or UDP_PORT as fd
******************************************************************************
*/
#include "sisetup.h"

extern int SIclose( struct ginfo_blk *gptr, int fd )
{

 struct tp_blk *tpptr;      //  pointer into tp list 
 int status = SI_ERROR;     //  status of processing 

 gptr->sierr = SI_ERR_HANDLE;
 if( gptr->magicnum == MAGICNUM )   //  good cookie at the gptr address? 
  {
   gptr->sierr = SI_ERR_SESSID;

   if( fd >= 0 )     //  if caller knew the fd number 
    {
     for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd;
          tpptr = tpptr->next );   //  find the tppblock to close 
    }
   else  //  user did not know the fd - find first Listener or UDP tp blk 
   {
	if( fd == TCP_LISTEN_PORT )			//  close first tcp listen port; else first udp 
     		for( tpptr = gptr->tplist; tpptr != NULL && !(tpptr->flags&& TPF_LISTENFD); tpptr = tpptr->next );   
	else
    		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->type != SOCK_DGRAM; tpptr = tpptr->next );
   }

   if( tpptr != NULL )
    {
     gptr->sierr = SI_ERR_TP;

     if( tpptr->squeue == NULL )   //  if nothing is queued to send... 
      {
       tpptr->flags |= TPF_UNBIND;   //  ensure port is unbound from tp 
	tpptr->flags |= TPF_DELETE;
	{
		int x = 1;

		setsockopt(tpptr->fd, SOL_SOCKET, SO_LINGER, (char *)&x, sizeof( x ) ) ;
	}
	close( tpptr->fd );
	tpptr->fd = -1;
	tpptr->type = -1;
			//  siterm now called in build poll if tp is marked delete 
       // SIterm( gptr, gptr, tpptr );*/        /* cleanup and remove from the list 
      }
     else                       	//  stuff queued to send - mark port to drain 
      tpptr->flags |= TPF_DRAIN;   //  and we will term the port when q empty 

     status = SI_OK;               //  give caller a good status 
    }                              //  end if we found a tpptr 
  }                                //  end if the handle was good 

 return( status );                 //  send the status back to the caller 
}                                  //  SIclose 

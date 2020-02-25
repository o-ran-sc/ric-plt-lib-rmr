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
*  Mnemonic: SIrcv
*  Abstract: This routine allows the user program to receive data on a
*            session without using the callback structure of the library.
*            It is the caller's responsibility to provide a buffer large
*            enough to handle the received data.
*  Parms:    gptr - The SIHANDLE that the user received on init call
*            sid  - The session id that the user wants to check
*            buf  - Pointer to buffer to receive data in
*            abuf - Pointer to buffer to return address of UDP sender in (!null)
*            buflen-Length of the receive buffer
*            delay- Value to pass to poll (time out) -1 == block until data
*  Returns:  SI_ERROR - (SIerrno will contain reason) if failure, else the
*            number of bytes read. If the number read is 0 SIerrno will indicate
*            why: time out exceeded, signal received.
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*  Mods:     26 Mar 20001 - Changed to support UDP reads
*
******************************************************************************
*/
#include "sisetup.h"    //  get start up stuff 
#include "sitransport.h"

extern int SIrcv( struct ginfo_blk *gptr, int sid, char *buf, int buflen, char *abuf, int delay ) {
 //extern int sigflags;           //  signal flags 
 int status = SI_ERROR;         //  assume the worst to return to caller 
 struct tp_blk *tpptr;          //  pointer to transport provider info 
 int flags = 0;                 //  receive flags 
 int remainder;                 //  # of bytes remaining after rcv if more 
 fd_set readfds;                //  special set of read fds for this call 
 fd_set execpfds;               //  special set of read fds for this call 
 struct timeval *tptr = NULL;   //  time info for select call 
 struct timeval time;
 struct sockaddr *uaddr;       //  pointer to udp address 
	char 	*acbuf;		//  pointer to converted address 
 int addrlen;

 if( gptr->magicnum != MAGICNUM )     //  if not a valid ginfo block 
  return SI_ERROR;

 for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != sid;
      tpptr = tpptr->next );      //  find transport block 
 if( tpptr == NULL )
  return SI_ERROR;                      //  signal bad block 

 uaddr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
 addrlen = sizeof( *uaddr );

 if( ! (gptr->flags & GIF_SHUTDOWN) )
  {                        //  if not in shutdown and no signal flags  
   FD_ZERO( &readfds );               //  clear select info 
   FD_SET( tpptr->fd, &readfds );     //  set to check read status 

   FD_ZERO( &execpfds );               //  clear select info 
   FD_SET( tpptr->fd, &execpfds );     //  set to check read status 

   if( delay >= 0 )                //  user asked for a fininte time limit 
    {
     tptr = &time;                 //  point at the local struct 
     tptr->tv_sec = 0;             //  setup time for select call 
     tptr->tv_usec = delay;
    }

   if( (select( tpptr->fd + 1, &readfds, NULL, &execpfds, tptr ) < 0 ) )
    gptr->flags |= GIF_SHUTDOWN;     //  we must shut on error or signal 
   else
    {                                //  poll was successful - see if data ? 
     if( FD_ISSET( tpptr->fd, &execpfds ) )   //  session error? 
      {
       SIterm( gptr, tpptr );                 //  clean up our end of things 
      }
     else
      {
       if( (FD_ISSET( tpptr->fd, &readfds )) )
        {                                       //  process data if no signal 
		if( tpptr->type == SOCK_DGRAM )        //  raw data received 
		{
			status = RECVFROM( sid, buf, buflen, 0, uaddr, &addrlen );
			if( abuf )
			{
				SIaddress( uaddr, (void **) &acbuf, AC_TODOT );	//  address returns pointer to buf now rather than filling 
				strcpy( abuf, acbuf );			//  must be back compat with old versions 
				free( acbuf );
			}
			if( status < 0 )                        //  session terminated? 
				SIterm( gptr, tpptr );	                 //  so close our end 
          	}
         	else                                      //  cooked data received 
          	{
           		status = RECV( sid, buf, buflen, 0 );   //  read data into user buf 
           		if( status < 0 )                        //  session terminated? 
            			SIterm( gptr, tpptr );                 //  so close our end 
          	}
        }                                         //  end event was received 
       else                                       //  no event was received  
        status = 0;                               //  status is just ok 
      }                       //  end else - not in shutdown mode after poll 
    }                     //  end else pole was successful 
  }                                 //  end if not already signal shutdown 

 if( gptr->flags & GIF_SHUTDOWN  &&  gptr->tplist != NULL )
  {             //  shutdown received but sessions not cleaned up 
   SIshutdown( gptr );
   status = SI_ERROR;                //  indicate failure on return 
  }                                  //  end if shut but not clean 

 free( uaddr );
 return status;          //  send back the status 
}                           //  SIrcv 

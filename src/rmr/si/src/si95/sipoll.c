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
**************************************************************************
*  Mnemonic: SIpoll
*  Abstract: This routine will poll the sockets that are open for
*            an event and return after the delay period has expired, or
*            an event has been processed.
*  Parms:    gptr   - Pointer to the global information block
*            msdelay- 100ths of seconds to delay
*  Returns:  SI_OK if the caller can continue, SI_ERROR if all sessions have been
*            stopped, or the interface cannot proceed. When SI_ERROR is
*            returned the caller should cleanup and exit immediatly (we
*            have probably received a sigter or sigquit.
*  Date:     10 April 1995
*  Author:   E. Scott Daniels
*
**************************************************************************
*/
#include  "sisetup.h"     //  get the setup stuff
#include "sitransport.h"
#include <wait.h>


extern int SIpoll( struct ginfo_blk *gptr, int msdelay )
{
 //extern int deaths;       //  number of children that died and are zombies
 //extern int sigflags;     //  flags set by the signal handler routine

 int fd;                       //  file descriptor for use in this routine
 int ((*cbptr)());             //  pointer to callback routine to call
 int status = SI_OK;              //  return status
 int addrlen = 0;              //  length of address from recvfrom call
 char *buf;		               //  work buffer pointer
 char ibuf[1025];
 int i;                        //  loop index
 struct tp_blk *tpptr;         //  pointer at tp stuff
 struct tp_blk *nextone = NULL;	//  pointer at next block to process
 int pstat;                    //  poll status
 int kstat;                    //  keyboard status
 struct timeval  delay;        //  delay to use on select call
 struct sockaddr *uaddr;       //  pointer to udp address

 if( gptr->flags & GIF_SHUTDOWN )     //  cannot do if we should shutdown
  return( SI_ERROR );                    //  so just get out


 if( gptr->magicnum != MAGICNUM )     //  if not a valid ginfo block
  return( SI_ERROR );

   delay.tv_sec = msdelay/100;                //  user submits 100ths, cvt to seconds and milliseconds
   delay.tv_usec = (msdelay%100) * 10;


   SIbldpoll( gptr );                 //  build the fdlist for poll
   pstat = 0;                         //  ensure good code

   if( gptr->fdcount > 0 )
    pstat = select( gptr->fdcount, &gptr->readfds, &gptr->writefds,
                               &gptr->execpfds, &delay );

   if( (pstat < 0 && errno != EINTR)  )
    {                             //  poll fail or termination signal rcvd
     gptr->fdcount = 0;           //  prevent trying to look at a session
     gptr->flags |= GIF_SHUTDOWN; //  cause cleanup and exit at end
    }

   if( pstat > 0  &&  (! (gptr->flags & GIF_SHUTDOWN)) )
    {
     if( FD_ISSET( 0, &gptr->readfds ) )       //  check for keybd input
      {
       fgets( ibuf, 1024, stdin );   //  get the stuff from keyboard
       if( (cbptr = gptr->cbtab[SI_CB_KDATA].cbrtn) != NULL )
        {
         status = (*cbptr)( gptr->cbtab[SI_CB_KDATA].cbdata, ibuf );
         SIcbstat(  gptr, status, SI_CB_KDATA );    //  handle status
        }                                 //  end if call back was defined
      }

	tpptr = gptr->tplist; 
	while( tpptr != NULL ) {
		nextone = tpptr->next;					//  allow for a delete in loop

       if( tpptr->squeue != NULL && (FD_ISSET( tpptr->fd, &gptr->writefds )) )
        SIsend( gptr, tpptr );              //  send if clear to send

       if( FD_ISSET( tpptr->fd, &gptr->execpfds ) )
        {
         ; //  sunos seems to set except for unknown reasons; ignore
        }
       else
       if( FD_ISSET( tpptr->fd, &gptr->readfds ) )  //  read event pending?
        {
         fd = tpptr->fd;                     //  quick ref to the fd

         if( tpptr->flags & TPF_LISTENFD )     //  listen port setup by init?
          {                                    //  yes-assume new session req
           status = SInewsession( gptr, tpptr );    //  make new session
          }
         else                              //  data received on a regular port
          if( tpptr->type == SOCK_DGRAM )          //  udp socket?
           {
            uaddr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
            status = RECVFROM( fd, gptr->rbuf, MAX_RBUF, 0, uaddr, &addrlen );
            if( status >= 0 && ! (tpptr->flags & TPF_DRAIN) )
             {					                         //  if good status call cb routine
              if( (cbptr = gptr->cbtab[SI_CB_RDATA].cbrtn) != NULL )
               {
                SIaddress( uaddr, (void **) &buf, AC_TODOT );
                status = (*cbptr)( gptr->cbtab[SI_CB_RDATA].cbdata, gptr->rbuf, status, buf );
                SIcbstat( gptr, status, SI_CB_RDATA );    //  handle status
				free( buf );
               }                              //  end if call back was defined
             }                                //  end if status was ok
            free( uaddr );
           }                                  //  end if udp
          else
           {                                //  else receive on tcp session
            status = RECV( fd, gptr->rbuf, MAX_RBUF, 0 );    //  read data

            if( status > SI_OK  &&  ! (tpptr->flags & TPF_DRAIN) )
             {
              if( (cbptr = gptr->cbtab[SI_CB_CDATA].cbrtn) != NULL )
               {
                status = (*cbptr)( gptr->cbtab[SI_CB_CDATA].cbdata, fd, gptr->rbuf, status );
                SIcbstat( gptr, status, SI_CB_CDATA );   //  handle cb status
               }                            //  end if call back was defined
             }                                     //  end if status was ok
            else   //  sunos seems to send 0 bytes as indication of disc
             {
              if( (cbptr = gptr->cbtab[SI_CB_DISC].cbrtn) != NULL )
               {
                status = (*cbptr)( gptr->cbtab[SI_CB_DISC].cbdata, tpptr->fd );
                SIcbstat( gptr, status, SI_CB_DISC );    //  handle status
               }
              SIterm( gptr, tpptr );
            }
           }                                                //  end tcp read
        }                    //  end if event on this fd

		tpptr = nextone;
      }                      //  end for each fd in the list
    }                        //  end if not in shutdown


 if( gptr->flags & GIF_SHUTDOWN )      //  we need to stop for some reason
  {
   status = SI_ERROR;                //  status should indicate to user to die
   SIshutdown( gptr );            //  clean things up
  }
 else
  status = SI_OK;                    //  user can continue to process

 return( status );                //  send status back to caller
}                                 //  SIwait

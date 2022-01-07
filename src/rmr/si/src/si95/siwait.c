// vim: noet sw=4 ts=4:
/*
==================================================================================
    Copyright (c) 2020-2021 Nokia
    Copyright (c) 2020-2021 AT&T Intellectual Property.

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
*  Mnemonic: SIwait
*  Abstract: This  routine will wait for an event to occur on the
*            connections in tplist. When an event is received on a fd
*            the status of the fd is checked and the event handled, driving
*            a callback routine if necessary. The system call poll is usd
*            to wait, and will be interrupted if a signal is caught,
*            therefore the routine will handle any work that is required
*            when a signal is received. The routine continues to loop
*            until the shutdown flag is set, or until there are no open
*            file descriptors on which to wait.
*  Parms:    gptr - Pointer to the global information block
*  Returns:  SI_OK if the caller can continue, SI_ERROR if all sessions have been
*            stopped, or the interface cannot proceed. When SI_ERROR is
*            returned the caller should cleanup and exit immediatly (we
*            have probably received a sigter or sigquit.
*  Date:     28 March 1995
*  Author:   E. Scott Daniels
*
*  Modified: 11 Apr 1995 - To pass execption to select when no keyboard
*            19 Apr 1995 - To call do key to do better keyboard editing
*            18 Aug 1995 - To init kstat to 0 to prevent key hold if
*                          network data pending prior to entry.
*			31 Jul 2016 - Major formatting clean up in the main while loop.
**************************************************************************
*/
#include  "sisetup.h"     //  get the setup stuff
#include "sitransport.h"
#include	<sys/wait.h>

/*
	The select timeout is about 300 mu-sec. This is fast enough to add a new
	outbound connection to the poll list before the other side responds,
	but slow enough so as not to consume excess CPU when idle.
*/
#define SI_SELECT_TIMEOUT 300000

#ifndef SYSTEM_UNDER_TEST
#	define SYSTEM_UNDER_TEST 0
#endif

extern int SIwait( struct ginfo_blk *gptr ) {
	int fd = -1;					//  file descriptor for use in this routine
	int ((*cbptr)());				//  pointer to callback routine to call
	int status = SI_OK;				//  return status
	int addrlen = 0;				//  length of address from recvfrom call
	int i;							//  loop index
	struct tp_blk *tpptr = NULL;	//  pointer at tp stuff
	struct tp_blk *nextone= NULL;	//  point at next block to process in loop
	int pstat = 0;					//  poll status
	struct timeval  timeout;		//  delay to use on select call
	// char *buf = NULL;	// DEPRECATED -- seems unnecessary
	// char *ibuf = NULL;	// DEPRECATED -- seems unnecessary

	if( gptr->magicnum != MAGICNUM ) {				//  if not a valid ginfo block
		rmr_vlog( RMR_VL_CRIT, "SI95: wait: bad global info struct magic number is wrong\n" );
		return SI_ERROR;
	}

	if( gptr->flags & GIF_SHUTDOWN ) {				//  cannot do if we should shutdown
		return SI_ERROR;							//  so just get out
	}

	// DEPRECATED -- seems unnecessary
	// if( ( ibuf = (char *) malloc( 2048 ) ) == NULL ) {
	// 		rmr_vlog( RMR_VL_WARN, "ibuf malloc fail\n" );
	// 		return SI_ERROR;
	// }

	do {									// spin until a callback says to stop (likely never)
		timeout.tv_sec = 0;					// must be reset on every call!
		timeout.tv_usec = SI_SELECT_TIMEOUT;

		SIbldpoll( gptr );					// poll list is trashed on each pop; must rebuild
		pstat = select( gptr->fdcount, &gptr->readfds, &gptr->writefds, &gptr->execpfds, &timeout );

		if( (pstat < 0 && errno != EINTR)  ) {
			gptr->fdcount = 0;				//  prevent trying to look at a session
			gptr->flags |= GIF_SHUTDOWN;	//  cause cleanup and exit at end
		}

		if( pstat > 0  &&  (! (gptr->flags & GIF_SHUTDOWN)) ) {
			tpptr = gptr->tplist;
			while( tpptr != NULL ) {
				nextone = tpptr->next;				//  prevent issues if we delete the block during loop

				if( tpptr->fd >= 0 ) {
					if( tpptr->squeue != NULL && (FD_ISSET( tpptr->fd, &gptr->writefds )) ) {
						SIsend( gptr, tpptr );			//  send if clear to send
					}

					if( FD_ISSET( tpptr->fd, &gptr->execpfds ) ) {
							;				// sunos seems to set the except flag for unknown reasons; ignore it
					} else {
						if( FD_ISSET( tpptr->fd, &gptr->readfds ) ) {			// ready to read
							fd = tpptr->fd;
							tpptr->rcvd++;

							if( tpptr->flags & TPF_LISTENFD ) {					// new session request
								errno=0;
								status = SInewsession( gptr, tpptr );			// accept connection
							} else  {											//  data received on a regular port (we support just tcp now
								status = RECV( fd, gptr->rbuf, MAX_RBUF, 0 );	//  read data
								if( status > 0  &&  ! (tpptr->flags & TPF_DRAIN) ) {
									if( (cbptr = gptr->cbtab[SI_CB_CDATA].cbrtn) != NULL ) {
										status = (*cbptr)( gptr->cbtab[SI_CB_CDATA].cbdata, fd, gptr->rbuf, status );
										SIcbstat( gptr, status, SI_CB_CDATA );	//  handle cb status
									}
								} else {										// no bites, but read flagged indicates disconnect
									if( (cbptr = gptr->cbtab[SI_CB_DISC].cbrtn) != NULL ) {
										status = (*cbptr)( gptr->cbtab[SI_CB_DISC].cbdata, tpptr->fd );
										SIcbstat( gptr, status, SI_CB_DISC );	//  handle status
									}
									SIterm( gptr, tpptr );			// close FD and mark block for deletion
								}
							}
						}
					}
				}								//  if still good fd

				tpptr = nextone;
			}
		}

		if( SYSTEM_UNDER_TEST ) {				 // enabled only during uint testing to prevent blocking
			break;
		}
	} while( gptr->tplist != NULL && !(gptr->flags & GIF_SHUTDOWN) );

	// free( ibuf );	// DEPRECATED -- seems unnecessary
	if( gptr->tplist == NULL )					//  indicate all fds closed

	if( gptr->flags & GIF_SHUTDOWN ) {			//  we need to stop for some reason
		status = SI_ERROR;						//  status should indicate to user to die
		SIshutdown( gptr );						//  clean things up
	} else {
		status = SI_OK;							//  user can continue to process
	}

	return status;
}

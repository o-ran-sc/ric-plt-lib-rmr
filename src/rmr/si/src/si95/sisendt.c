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
****************************************************************************
*
*  Mnemonic: SIsendt
*  Abstract: This module contains various send functions:
*				SIsendt -- send tcp with queuing if would block
*				SIsendt_nq - send tcp without queuing if blocking
*
*  Date:     27 March 1995
*  Author:   E. Scott Daniels
*  Mod:		22 Feb 2002 - To better process queued data
*			14 Feb 2020 - To fix index bug if fd < 0.
*
*****************************************************************************
*/

#include "sisetup.h"     //  get setup stuff
#include "sitransport.h"

/*
	Send a message on what is assumed to be a tcp connection. If the session
	would block, then SI_ERR_BLOCKED is returned. Else, SI_OK or SI_ERROR
	is returned to indicate state. Errno should be set to reflect error state:
		EBADFD - error from system; fd was closed
		EBUSY	- system would block the send call
		EINVAL	- fd was not valid or did not reference an open session
*/
//extern int SIsendt_nq( struct ginfo_blk *gptr, int fd, char *ubuf, int ulen ) {
extern int SIsendt( struct ginfo_blk *gptr, int fd, char *ubuf, int ulen ) {
	int status = SI_ERROR;      //  assume we fail
	fd_set writefds;            //  local write fdset to check blockage
	fd_set execpfds;            //  exception fdset to check errors
	struct tp_blk *tpptr;       //  pointer at the tp_blk for the session
	struct ioq_blk *qptr;       //  pointer at i/o queue block
	struct timeval time;        //  delay time parameter for select call
	int	sidx = 0;				// send index

	errno = EINVAL;

	if( fd < 0 ) {
		errno = EBADFD;
		return SI_ERROR;					// bad form trying to use this fd
	}

	if( fd < MAX_FDS ) {					// straight from map if possible
		tpptr = gptr->tp_map[fd];
	} else {
		// list should be locked before traversing
		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd; tpptr = tpptr->next ) ; //  find the block if out of map's range
	}
	if( tpptr != NULL ) {
		if( (fd = tpptr->fd) < 0 || (fd = tpptr->fd) >= FD_SETSIZE ) {			// fd user given might not be real, and this might be closed already
			errno = EBADFD;
			return SI_ERROR;
		}

		tpptr->sent++;				// investigate: this may over count

		FD_ZERO( &writefds );       //  clear for select call
		FD_SET( fd, &writefds );    //  set to see if this one was writable
		FD_ZERO( &execpfds );       //  clear and set execptions fdset
		FD_SET( fd, &execpfds );

		time.tv_sec = 0;			//  set both to 0 if we just want a poll, else we block at max this amount
		time.tv_usec = 1;			// small pause on check to help drain things

		if( select( fd + 1, NULL, &writefds, &execpfds, &time ) > 0 ) {		//  would block if <= 0
			if( FD_ISSET( fd, &execpfds ) ) {		//  error?
				errno = EBADFD;
				SIterm( gptr, tpptr );				// mark block for deletion when safe
				return SI_ERROR;					// and bail from this sinking ship
			} else {
				errno = 0;
				while( ulen > 0 ) {				// once we start, we must ensure that it all goes out
					status =  SEND( tpptr->fd, ubuf+sidx, (unsigned int) ulen, 0 );
					if( status >= 0 ) {
						sidx += status;
						ulen -= status;
						status = SI_OK;
					} else {
						if( errno != EINTR || errno != EAGAIN ) {
							status = SI_ERROR;
							break;
						}
					}
				}
			}
		} else {
			errno = EBUSY;
			status = SI_ERR_BLOCKED;
		}
	} else {
		errno = EBADFD;			// fd in a bad state (probably lost)
	}

	return status;
}


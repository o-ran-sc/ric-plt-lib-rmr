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

*  Date:     27 March 1995
*  Author:   E. Scott Daniels
*  Mod:		22 Feb 2002 - To better process queued data 
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
	gptr->sierr = SI_ERR_SESSID;

	if( fd < MAX_FDS ) {					// straight from map if possible
		tpptr = gptr->tp_map[fd];
	} else {
		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd; tpptr = tpptr->next ); //  find the block if out of map's range
	}

	if( tpptr != NULL ) {
		tpptr->sent++;				// investigate: this may over count

		FD_ZERO( &writefds );       //  clear for select call 
		FD_SET( fd, &writefds );    //  set to see if this one was writable 
		FD_ZERO( &execpfds );       //  clear and set execptions fdset 
		FD_SET( fd, &execpfds );

		time.tv_sec = 0;			//  set both to 0 if we just want a poll, else we block at max this amount
		time.tv_usec = 1;			// small pause on check to help drain things

		if( select( fd + 1, NULL, &writefds, &execpfds, &time ) > 0 ) {		//  would block if <= 0
			gptr->sierr = SI_ERR_TP;
			if( FD_ISSET( fd, &execpfds ) ) {   	//  error? 
				errno = EBADFD;
				SIterm( gptr, tpptr );   			//  clean up our portion of the session 
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
		errno = EBADFD;  		// fd in a bad state (probably losed)
	}

	return status;
}

/*
	This routine will send a datagram to the TCP session partner
	that is connected via the FD number that is passed in.
	If the send would cause the process to block, the send is
	queued on the tp_blk for the session and is sent later as
	a function of the SIwait process.  If the buffer must be
	queued, a copy of the buffer is created such that the
	user program may free, or reuse, the buffer upon return.

	Parms:i		gptr - The pointer to the global info structure (context)
	            fd   - File descriptor (session number)
	            ubuf - User buffer to send.
	            ulen - Lenght of the user buffer.

	Returns:  SI_OK if sent, SI_QUEUED if queued for later, SI_ERROR if error.
*/
#ifdef KEEP
extern int new_SIsendt( struct ginfo_blk *gptr, int fd, char *ubuf, int ulen ) {
	int status = SI_OK;         //  status of processing 
	fd_set writefds;            //  local write fdset to check blockage 
	fd_set execpfds;            //  exception fdset to check errors 
	struct tp_blk *tpptr;       //  pointer at the tp_blk for the session 
	struct ioq_blk *qptr;       //  pointer at i/o queue block 
	struct timeval time;        //  delay time parameter for select call 

	gptr->sierr = SI_ERR_HANDLE;

	//if( gptr->magicnum == MAGICNUM ) {     //  ensure cookie is good  -- we need to be too performant for this
	//{                                   //  mmmm oatmeal, my favorite 
		gptr->sierr = SI_ERR_SESSID;

		if( fd < MAX_FDS ) {					// straight from map if possible
			tpptr = gptr->tp_map[fd];
		} else {
			for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd; tpptr = tpptr->next ); //  find the block if out of map's range
		}

		if( tpptr != NULL ) {
			tpptr->sent++;

			FD_ZERO( &writefds );       //  clear for select call 
			FD_SET( fd, &writefds );    //  set to see if this one was writable 
			FD_ZERO( &execpfds );       //  clear and set execptions fdset 
			FD_SET( fd, &execpfds );

			time.tv_sec = 0;			//  set both to 0 if we just want a poll, else we block at max this amount
			time.tv_usec = 1;			// small pause on check to help drain things

			if( select( fd + 1, NULL, &writefds, &execpfds, &time ) > 0 ) {		//  see if it would block
				gptr->sierr = SI_ERR_TP;
				if( FD_ISSET( fd, &execpfds ) ) {   //  error? 
					SIterm( gptr, tpptr );   			//  clean up our portion of the session 
					return SI_ERROR;					// and bail from this sinking ship
				} else {
					if( tpptr->squeue ) {
						SIsend( gptr, tpptr );			//  something queued; send off queue and queue this
					} else {
						return SEND( tpptr->fd, ubuf, (unsigned int) ulen, 0 );   //  done after send 
					}
				}
			}

			gptr->sierr = SI_ERR_NOMEM;

			tpptr->qcount++;
			if( (qptr = SInew( IOQ_BLK )) != NULL ) {		//  alloc a queue block 
				if( tpptr->sqtail == NULL ) {				//  if nothing on the queue 
					tpptr->squeue = qptr;         //  simple add to the tp blk q 
					tpptr->sqtail = qptr;
	 			} else  {                          		//  else - add at end of the q 
	 				tpptr->sqtail->next = qptr;		
					tpptr->sqtail = qptr;	
					qptr->next = NULL;		//  new block is the last one now 
				}    		                       //  end add block at end of queue 

				qptr->dlen = ulen;           //  copy info to queue block 
				qptr->data = (char *) malloc( ulen );  //  get buffer 
				memcpy( qptr->data, (const char*) ubuf, ulen );
	
				gptr->sierr = SI_QUEUED;                //  indicate queued to caller 
				status = SI_QUEUED;						// for return
			}
		}							//  end if tpptr was not found 
	//}								//  ginfo pointer was corrupted 

	return status;
}
#endif

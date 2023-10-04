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
*****************************************************************************
*
*  Mnemonic:	SInewsession
*  Abstract:	This routine can be called when a request for connection is
*				received. It will establish a new fd, create a new
*				transport provider block (added to the head of the list).
*				The security callback and connection callback
*				routines are driven from this routine.
*  Parms:    gptr - Pointer to the general information block
*            tpptr- Pointer to the tp block that describes the fd that
*                   received the connection request (the listen tp block).
*  Returns:  SI_OK if all went well, SI_ERROR if not.
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include "sisetup.h"          //  get necessary defs etc
#include "sitransport.h"
#include <netinet/tcp.h>

extern int SInewsession( struct ginfo_blk *gptr, struct tp_blk *tpptr ) {
	struct sockaddr *addr;             //  pointer to address of caller
	struct spxopt_s *sopts;            //  pointer to spx options
	struct tp_blk *newtp;              //  pointer at new tp block
	int status = SI_OK;                //  processing status
	int (*cbptr)();                    //  pointer to callback function
	unsigned int addrlen;				//  length of address from accept
	char *buf = NULL;					//  pointer to address
	int optval;

	addr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
	addrlen = sizeof( struct sockaddr );
	memset( addr, 0, sizeof( struct sockaddr ) );

	status = accept( tpptr->fd, addr, &addrlen );	//  accept and assign new fd (status)
	if( status < 0 ) {
		free( addr );
		return SI_ERROR;
	}

	newtp = SInew( TP_BLK );			      //  get a new tp block for the session
	if( newtp == NULL ) {
		CLOSE( status );						// must disconnect the other side
		free( addr );
		return SI_ERROR;
	}

	newtp->next = gptr->tplist;					//  add new block to the head of the list
	if( newtp->next != NULL ) {
		newtp->next->prev = newtp;				//  back chain to us
	}
	gptr->tplist = newtp;
	newtp->paddr = (struct sockaddr *) addr;	//  partner address
	newtp->fd = status;                         //  save the fd from accept

	if( gptr->tcp_flags & SI_TF_NODELAY ) {		// set on/off for no delay configuration
		optval = 1;
	} else {
		optval = 0;
	}
	SETSOCKOPT( tpptr->fd, SOL_TCP, TCP_NODELAY, (void *)&optval, sizeof( optval) );

	if( gptr->tcp_flags & SI_TF_FASTACK ) {		// set on/off for fast ack config
		optval = 1;
	} else {
		optval = 0;
	}
	SETSOCKOPT( tpptr->fd, SOL_TCP, TCP_QUICKACK, (void *)&optval, sizeof( optval) ) ;

	if( gptr->tcp_flags & SI_TF_QUICK ) {
		optval = 1;
		SETSOCKOPT( tpptr->fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&optval, sizeof( optval) ) ;
		optval = 1;
		SETSOCKOPT( tpptr->fd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&optval, sizeof( optval) ) ;
		optval = 1;
		SETSOCKOPT( tpptr->fd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&optval, sizeof( optval) ) ;
		optval = 5;
		SETSOCKOPT( tpptr->fd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&optval, sizeof( optval) ) ;
	}

	SIaddress( addr, (void **) &buf, AC_TODOT );							// get addr of remote side; buf must be freed
	if( (cbptr = gptr->cbtab[SI_CB_SECURITY].cbrtn) != NULL ) {				//   invoke the security callback function if there
		status = (*cbptr)( gptr->cbtab[SI_CB_SECURITY].cbdata, buf );
		if( status == SI_RET_ERROR ) {										//  session to be rejected
			SIterm( gptr, newtp );											//  terminate new tp block (do NOT call trash)
			// free( addr ); // not required, will be eventually freed by SItrash
			free( buf );
			return SI_ERROR;
		} else {
			SIcbstat( gptr, status, SI_CB_SECURITY );		//  allow for unreg or shutdown signal
		}
	}

	newtp->flags |= TPF_SESSION;     //  indicate a session here

	if( (cbptr = gptr->cbtab[SI_CB_CONN].cbrtn) != NULL ) {		// drive connection callback
		status=(*cbptr)( gptr->cbtab[SI_CB_CONN].cbdata, newtp->fd, buf );
		SIcbstat(  gptr, status, SI_CB_CONN );               //  handle status
	}

	SImap_fd( gptr, newtp->fd, newtp );		// add fd to the map

	free( buf );
	return SI_OK;
}


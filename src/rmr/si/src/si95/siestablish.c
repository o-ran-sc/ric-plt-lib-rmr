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
*-----------------------------------------------------------------------------------
*
* Mnemonic:		SIestablish
* Abstract:i	Prep functions that set up a socket for listening or making a
*				connection.
* Date:     	26 March 1995
* Author:   	E. Scott Daniels
*
* Modified: 	19 Apr 1995 - To keep returned address of the port.
*				08 Mar 2007 - conversion for ipv6.
*				12 Oct 2020 - split into connect prep and listen prep
*								functions.
*-----------------------------------------------------------------------------------
*/


#include "sisetup.h"       //  include the necessary setup stuff 
#include "sitransport.h"
#include <errno.h>
#include <netinet/tcp.h>

#ifndef SO_REUSEPORT
#define SO_REUSEPORT 0
#endif

/*
	Prep a socket for "listening."
	This routine will open a socket and bind an address to it in
	preparation for listening for connections or inbound UDP
	datagrams. A file descriptor for the socket is captured and all
	related information is placed into a transport provider (tp) block.

	Type is the SI constant UDP_DEVICE or TCP_DEVICE
	abuf points to the address that is to be bound to the socket.
	Family is one of the AF_* constants (AF_ANY, AF_INET or AF_INET6)

	The address should be one of these forms:
			[::1]:port			// v6 localhost device (loop back)
			localhost:port		// v4 or 6 loopback depending on /etc/hosts
			0.0.0.0:port		// any interface
			addr:port			// an address assigned to one of the devices

	Returns a transport struct which is the main context for the listener.
*/
extern struct tp_blk *SIlisten_prep( struct ginfo_blk *gptr, int type, char* abuf, int family ) {
	struct tp_blk *tptr;         //  pointer at new tp block 
	int status = SI_OK;             //  processing status 
	struct sockaddr *addr;    	//  IP address we are requesting 
	int protocol;                //  protocol for socket call 
	char buf[256];               //  buffer to build request address in 
	int optval = 0;
	int alen = 0;

	tptr = (struct tp_blk *) SInew( TP_BLK );     //  new transport info block 

	if( tptr != NULL )
	{
		addr = NULL;

		switch( type )			//  things specifc to tcp or udp 
		{
			case UDP_DEVICE:
				tptr->type = SOCK_DGRAM;
				protocol = IPPROTO_UDP;
				break;

			case TCP_DEVICE:
			default:
				tptr->type = SOCK_STREAM;
				protocol = IPPROTO_TCP;
		}

		alen = SIgenaddr( abuf, protocol, family, tptr->type, &addr );	//  family == 0 for type that suits the address passed in 
		if( alen <= 0 ) {
			return NULL;
		}

		tptr->family = addr->sa_family;

		if( (tptr->fd = SOCKET( tptr->family, tptr->type, protocol )) >= SI_OK ) {
			optval = 1;
			if( SO_REUSEPORT ) {
				SETSOCKOPT(tptr->fd, SOL_SOCKET, SO_REUSEPORT, (char *)&optval, sizeof( optval) ) ;
			}

			status = BIND( tptr->fd, (struct sockaddr *) addr, alen );
			if( status == SI_OK ) {
				tptr->addr = addr;         	//  save address 
			} else {
				fprintf( stderr, ">>>>> siestablish: bind failed: fam=%d type=%d pro=%d %s\n", tptr->family, tptr->type, protocol, strerror( errno ) );
				close( tptr->fd );
			}
		} else {
			status = ! SI_OK;       		//  force bad return later 
			fprintf( stderr, ">>>>> siestablish: socket not esablished: fam=%d type=%d pro=%d %s\n", tptr->family, tptr->type, protocol, strerror( errno ) );
		}

		if( status != SI_OK ) {    			//  socket or bind call failed - clean up stuff 
			fprintf( stderr, ">>>>> siestablish: bad state -- returning nil pointer\n" );
			free( addr );
			SItrash( TP_BLK, tptr );	//  free the trasnsport block 
			tptr = NULL;        		//  set to return nothing 
		}
	}

	return tptr;
}

/*
	Look at the address and determine if the connect attempt to this address must
	use safe_connect() rather than the system connect_call(). On linux, a smart
	connect is needed if the target port is >32K and is even. This makes the assumption
	that the local port rage floor is 32K; we could read something in /proc, but 
	at this point won't bother.  Returns true if we determine that it is best to
	use safe_connect().
*/
static int need_smartc( char* abuf ) {
	char*	tok;
	int		state = 1;
	int		v;

	if( (tok = strchr( abuf, ':')) != NULL ) {
		v = atoi( tok+1 );
		if( v < 32767  || v % 2 != 0 ) {
			state = 0;
		}
	}

	return state;
}

/*
	Prep a socket to use to connect to a listener.
	Establish a transport block and target address in prep to connect.
	Type is the SI constant UDP_DEVICE or TCP_DEVICE. The abuf pointer
	should point to either a name:port or IP:port string. Family should
	be 0 to select the family best suited to the address provided, or
	any (v4 or v6) if the address is a name. If a perticular type is
	desired family should be either AF_INET or AF_INET6.  Using a
	family of 0 (AF_ANY) is usually the best choice.
*/
extern struct tp_blk *SIconn_prep( struct ginfo_blk *gptr, int type, char *abuf, int family ) {
	struct tp_blk *tptr;         //  pointer at new tp block 
	struct sockaddr *addr;    	//  IP address we are requesting 
	int protocol;                //  protocol for socket call 
	char buf[256];               //  buffer to build request address in 
	int optval = 0;
	int alen = 0;

	tptr = (struct tp_blk *) SInew( TP_BLK );     //  new transport info block 

	if( tptr != NULL )
	{
		addr = NULL;

		switch( type )			//  things specifc to tcp or udp 
		{
			case UDP_DEVICE:
				tptr->type = SOCK_DGRAM;
				protocol = IPPROTO_UDP;
				break;

			case TCP_DEVICE:
			default:
				tptr->type = SOCK_STREAM;
				protocol = IPPROTO_TCP;
		}

		alen = SIgenaddr( abuf, protocol, family, tptr->type, &addr );	//  family == 0 for type that suits the address passed in 
		if( alen <= 0 )
		{
			//fprintf( stderr, ">>>>> siconn_prep: error generating an address struct for %s(abuf) %d(proto) %d(type): %s\n",
			//	abuf, protocol, tptr->type, strerror( errno ) );
			return NULL;
		}

		tptr->family = addr->sa_family;
		tptr->palen = alen;

		if( (tptr->fd = SOCKET( tptr->family, tptr->type, protocol )) >= SI_OK ) {
			optval = 1;

			if( gptr->tcp_flags & SI_TF_NODELAY ) {
				optval = 1;
			} else {
				optval = 0;
			}
			SETSOCKOPT( tptr->fd, SOL_TCP, TCP_NODELAY, (void *)&optval, sizeof( optval) ) ;

			if( gptr->tcp_flags & SI_TF_FASTACK ) {
				optval = 1;
			} else {
				optval = 0;
			}
			SETSOCKOPT( tptr->fd, SOL_TCP, TCP_QUICKACK, (void *)&optval, sizeof( optval) ) ;

			tptr->paddr = addr;				// tuck the remote peer address away
			if( need_smartc( abuf ) ) {
				tptr->flags |= TPF_SAFEC;
			}
		} else {
			free( addr );
			SItrash( TP_BLK, tptr );       	// free the trasnsport block 
			tptr = NULL;					// we'll return nil
		}
	}

	return tptr;
}

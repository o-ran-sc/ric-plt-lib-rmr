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
*
*  Mnemonic: SIaddress
*  Abstract: This routine will convert a sockaddr_in structure to a
*            dotted decimal address, or visa versa.
*            If type == AC_TOADDR the src string may be: 
*            xxx.xxx.xxx.xxx.portnumber or host-name.portnumber
*            xxx.xxx.xxx.xxx.service[.protocol] or hostname;service[;protocol]
*            if protocol is not supplied then tcp is assumed.
*            hostname may be something like godzilla.moviemania.com
*  Parms:    src - Pointer to source buffer
*            dest- Pointer to dest buffer pointer 
*            type- Type of conversion AC_TODOT converts sockaddr to human readable. AC_TOADDR 
*				converts character buffer to sockaddr.
*  Returns:  Nothing.
*  Date:     19 January 1995
*  Author:   E. Scott Daniels
*
*  Modified: 22 Mar 1995 - To add support for ipx addresses.
*			18 Oct 2020 - drop old port separator (;)
*
*  CAUTION: The netdb.h header file is a bit off when it sets up the 
*           hostent structure. It claims that h_addr_list is a pointer 
*           to character pointers, but it is really a pointer to a list 
*           of pointers to integers!!!
*       
***************************************************************************
*/
#include "sisetup.h"      //  get necessary defs and other stuff 
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>

/* 
	target: buffer with address  e.g.  192.168.0.1:4444  :4444 (listen) [::1]4444
	family: PF_INET[6]  (let it be 0 to select based on addr in buffer 
	proto: IPPROTO_TCP IPPROTO_UDP
	type:   SOCK_STREAM SOCK_DGRAM

	returns length of struct pointed to by rap (return addr blockpointer)
*/
extern int SIgenaddr( char *target, int proto, int family, int socktype, struct sockaddr **rap ) {
	struct addrinfo hint;				//  hints to give getaddrinfo 
	struct addrinfo *list = NULL;		//  list of what comes back 
	int	ga_flags = 0;					//  flags to pass to getaddrinfo in hints 
	int	error = 0;
	int	rlen = 0;						//  length of the addr that rap points to on return 
	char	*pstr;						//  port string 
	char	*dstr;						//  a copy of the users target that we can destroy 
	char*	fptr;						// ptr we allocated and need to free (we may adjust dstr)

    fptr = dstr = strdup( (char *) target );	//  copy so we can destroy it with strtok 
	*rap = NULL;						//  ensure null incase something breaks 

	while( isspace( *dstr ) ) {
		dstr++;
	}

	if( *dstr == ':' ) {		//  user passed in :port -- so we assume this is for bind 
		pstr = dstr;
		*(pstr++) = 0;

		ga_flags = AI_PASSIVE;
	} else {
		if( *dstr == '[' ) {				// strip [ and ] from v6 and point pstring if port there
			dstr++;
			pstr = strchr( dstr, ']' );
			if( *pstr != ']' ) {
				return -1;
			}

			*(pstr++) = 0;
			if( *pstr == ':' ) {
				*(pstr++) = 0;
			} else {
				pstr = NULL;
			}
		} else {							// assume name or v4; point at port if there
			pstr = strchr( dstr, ':' );
			if( pstr != NULL ) {
				*(pstr++) = 0;
			}
		}
		ga_flags = AI_ADDRCONFIG;			// don't return IPVx addresses unless one such address is configured
	}

	memset( &hint, 0, sizeof( hint  ) );
	hint.ai_family = family;			//  AF_INET AF_INET6...  let this be 0 to select best based on addr 
	hint.ai_socktype = socktype;		//  SOCK_DGRAM SOCK_STREAM 
	hint.ai_protocol = proto;			//  IPPORTO_TCP IPPROTO_UDP 
	hint.ai_flags = ga_flags;

	if( DEBUG ) 
		fprintf( stderr, "[DBUG] siaddress: calling getaddrinfo flags=%x proto=%d family=%d target=%s host=%s port=%s\n", 
				ga_flags, proto, family, target, dstr, pstr );

	if( (error = getaddrinfo( dstr, pstr, &hint, &list )) ) {
		fprintf( stderr, "error from getaddrinfo: target=%s host=%s port=%s(port): error=(%d) %s\n", target, dstr, pstr, error, gai_strerror( error ) );
	} else {
		*rap = (struct sockaddr *) malloc(  list->ai_addrlen );		//  alloc a buffer and give address to caller 
		memcpy( *rap, list->ai_addr, list->ai_addrlen  );

		rlen = list->ai_addrlen;
		
		freeaddrinfo( list );		//  ditch system allocated memory 
	}

	free( dstr );
	return rlen;
}


/* 
	Given a source address convert from one form to another based on type constant.
	Type const == AC_TODOT   Convert source address structure to human readable string.
	Type const == AC_TOADDR6 Convert source string (host:port or ipv6 address [n:n...:n]:port) to an address struct
	Type const == AC_TOADDR  Convert source string (host:port or ipv4 dotted decimal address) to an address struct
*/
extern int SIaddress( void *src, void **dest, int type ) {
	struct sockaddr_in *addr;       //  pointer to the address 
	unsigned char *num;             //  pointer at the address number 
	char wbuf[256];                 //  work buffer 
	int i;         
	int	rlen = 0;					//  return len - len of address struct or string

	switch( type ) {
		case AC_TODOT:					//  convert from a struct to human readable "dotted decimal"
   			addr = (struct sockaddr_in *) src;
   			num = (char *) &addr->sin_addr.s_addr;    //  point at the long 

			if( addr->sin_family == AF_INET6 ) {
     				sprintf( wbuf, "[%u:%u:%u:%u:%u:%u]:%d", 
						*(num+0), *(num+1), *(num+2), 
						*(num+3), *(num+4), *(num+5) , 
						(int) ntohs( addr->sin_port ) );
			} else {
				sprintf( wbuf, "%u.%u.%u.%u;%d", *(num+0), *(num+1), *(num+2), *(num+3), (int) ntohs(addr->sin_port) );
			}

			*dest = (void *) strdup( wbuf );
			rlen = strlen( *dest );
			break;

  		case AC_TOADDR6:         		//  from hostname;port string to address for send etc 
			return SIgenaddr( src, PF_INET6, IPPROTO_TCP, SOCK_STREAM, (struct sockaddr **) dest );
			break; 

  		case AC_TOADDR:         		//  from dotted decimal to address struct ip4 
			return SIgenaddr( src, PF_INET, IPPROTO_TCP, SOCK_STREAM, (struct sockaddr **) dest );
			break;
	}

	return rlen;
}


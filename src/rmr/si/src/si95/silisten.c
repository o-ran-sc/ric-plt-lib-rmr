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
******************************************************************************
*
*  Mnemonic: 	SIlistener
*  Abstract: 	Open a port on which connection requests (TCP) or datagrams (UDP)
*				can be received. SIlistener will open the ipv4/6 port based on
*				the address buffer passed in. The listener() obsoletes SIopen()
*				with regard to opening udp ports.
*				Allows the user to open multiple secondary listening ports
*  Parms:   	type - TCP_DEVICE or UDP_DEVICE
*				abuf - buffer containing either 0.0.0.0;port or ::1;port
*
*  Returns: 	The file descriptor of the port, <0 if error
*  Date:    	26 March 1995 -- revised 13 Mar 2007 to support both ipv4 and 6
*  Author:  	E. Scott Daniels
*
*  Modified: 	10 May 1995 - To change SOCK_RAW to SOCK_DGRAM
*				14 Mar 2007 - To enhance for ipv6
******************************************************************************
*/
#include "sisetup.h"
#include "sitransport.h"

extern int SIlistener( struct ginfo_blk *gptr, int type, char *abuf ) {
	struct tp_blk *tpptr;      		//  pointer into tp list
	int status = SI_ERROR;     		//  status of processing

	if( PARANOID_CHECKS ) {
		if( gptr == NULL ) {
			return status;
		}
		if( gptr->magicnum != MAGICNUM )   			//  good cookie at the gptr address?
			return status;
	}

	tpptr = SIlisten_prep( type, abuf, 0 );

	if( tpptr != NULL )                          //  established a fd bound to the port ok
	{                   	                        //  enable connection reqs
		if( type == TCP_DEVICE )
		{
			if( (status = LISTEN( tpptr->fd, 1 )) < SI_OK )
				return SI_ERROR;

			tpptr->flags |= TPF_LISTENFD;          //  flag it so we can search it out if needed
		}

		tpptr->next = gptr->tplist;  		//  add to the list
		if( tpptr->next != NULL )
			tpptr->next->prev = tpptr;
		gptr->tplist = tpptr;
		status = tpptr->fd;			//  return the fd of the listener
	}

	return status;
}

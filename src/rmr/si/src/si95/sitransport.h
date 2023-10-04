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

	Mnemonic:	sitransport.h
	Abstract:	This file contains definitions needed to set up specific macros
				that allow for an underlying transport mechanism such as f-stack
				to be substituted for the normal system calls.  The underlying
				transport must support the same socket, bind, listen, etc., 
				calls and call parameters as the system calls.

	Date:		5 November 2019	
	Author:		E. Scott Daniels


****************************************************************************
*/

#ifndef _sitransport_h

#ifdef F_STACK

#include "ff_config.h"
#include "ff_api.h"


// TCP/UDP stack provied by f-stack
#define BIND		ff_bind
#define LISTEN		ff_listen
#define SOCKET		ff_socket
#define CONNECT		ff_connect
#define ACCEPT		ff_accept
#define CLOSE		ff_close
#define SHUTDOWN	ff_shutdown
#define	GETSOCKOPT	ff_getscokopt 
#define SETSOCKOPT	ff_setsockopt
#define READ		ff_read
#define WRITE		ff_write
#define SEND		ff_send
#define SENDTO		ff_sendto
#define RECV		ff_recv
#define RECVMSG		ff_recvmsg
#define RECVFROM	ff_recvfrom

#else

// support normal system TCP/UDP stack
#define BIND		bind
#define LISTEN		listen
#define SOCKET		socket
#define CONNECT		connect
#define ACCEPT		accept
#define CLOSE		close
#define SHUTDOWN	shutdown
#define	GETSOCKOPT	getsockopt 
#define SETSOCKOPT	setsockopt
#define READ		read
#define WRITE		write
#define SEND		send
#define SENDTO		sendto
#define RECV		recv
#define RECVFROM	recvfrom
#define RECVMSG		recvmsg

#endif



#endif

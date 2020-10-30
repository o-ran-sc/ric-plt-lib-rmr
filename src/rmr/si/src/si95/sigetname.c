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
 ------------------------------------------------------------------------
 Mnemonic:	sigetname
 Abstract:	returns the name of the socket for a given sid. On failure returns
			a nil pointer.
 Parms:		sid - the socket ID; a fd as returned by open/listen/connect
 Date:		21 July 2003
 Author:	E. Scott Daniels

 Mods:		30 Oct 2020 - address issues raised by code scan 1654181
 ------------------------------------------------------------------------
*/
#include "sisetup.h"

extern char* SIgetname( int sid ) {
	struct sockaddr oaddr;     //  pointer to address in TCP binary format
	char	*buf = NULL;
	int	len;

	len = sizeof( oaddr );
	if( getsockname( sid, &oaddr, &len ) < 0 ) {
		return NULL;
	}

	if( SIaddress(  &oaddr, (void **) &buf, AC_TODOT ) <= 0 ) {
		return NULL;
	}

	return buf;
}

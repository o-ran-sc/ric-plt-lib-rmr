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
*  Mnemonic: nisetup.h
*  Abstract: This file contains the necessary include statements for each
*            individually compiled module.
*  Date:     17 January 1995
*  Author:   E. Scott Daniels
*****************************************************************************
*/
 
#ifndef	_sisetup_h
#define _sisetup_h

#include <stdio.h>              //  standard io 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>              //  error number constants 
#include <fcntl.h>              //  file control 
#include <netinet/in.h>         //  inter networking stuff 
#include <signal.h>             //  info for signal stuff 
#include <string.h>
#include <errno.h>
#include <sys/types.h>          //  various system files - types 
#include <sys/socket.h>         //  socket defs 

#include <rmr_logging.h>

//  pure bsd supports SIGCHLD not SIGCLD as in bastard flavours 
#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

#include "socket_if.h"			// public definitions; dummy types/structs
#include "siconst.h"			//  internal constants  and prototypes 
#include "sistruct.h"			//  real structure defs
#include "siproto.h"

#endif

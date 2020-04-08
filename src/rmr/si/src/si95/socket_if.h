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
*  Mnemonic:	socket_if.h
*  Abstract:	Main set of definitions needed by the SI native functions and
*				any user code.
*  Date:	    26 March 1995	(original)
*				3 January 2020  (revised)
*  Author:		E. Scott Daniels
*
*****************************************************************************
*/

#ifndef _SOCKET_IF_H
#define _SOCKET_IF_H

#ifndef PARANOID_CHECKS
#	define PARANOID_CHECKS 0
#endif

#define TCP_DEVICE	0     	//  device type of socket
#define UDP_DEVICE	1

//  these are for SIclose, must be negative so as to be distinguished from real fd values
#define TCP_LISTEN_PORT	(-1)	//  close first listen port found
#define UDP_PORT	(-2)	//  close first udp port found

#define SI_BAD_HANDLE  ((void *) 0)

#define SI_OPT_NONE		0		// initialisation options
#define SI_OPT_FORK    0x01      //  fork new sessions
#define SI_OPT_FG      0x02      //  keep process in the "foreground"
#define SI_OPT_TTY     0x04      //  processes keyboard interrupts if fg
#define SI_OPT_ALRM    0x08      //  cause setsig to be called with alarm flg

                                 //  offsets of callbacks in table
                                 //  used to indentify cb in SIcbreg
#define SI_CB_SIGNAL   0         //  usr signal/alarm received
#define SI_CB_RDATA    1         //  handles data arrival on raw interface
#define SI_CB_CDATA    2         //  handles data arrival on cooked interface
#define SI_CB_KDATA    3         //  handles data arrival from keyboard
#define SI_CB_SECURITY 4         //  authorizes acceptance of a conect req
#define SI_CB_CONN     5         //  called when a session is accepted
#define SI_CB_DISC     6         //  called when a session is lost
#define SI_CB_POLL     7

                                 //  return values callbacks are expected to produce
#define SI_RET_OK      0         //  processing ok -- continue
#define SI_RET_ERROR   (-1)      //  processing not ok -- reject/disallow
#define SI_RET_UNREG   1         //  unregester (never call again) the cb
#define SI_RET_QUIT    2         //  set the shutdown flag  and terminate the SI environment

                                 //  values returned to user by SI routines
#define SI_ERROR       (-1)      //  unable to process
#define SI_OK          0         //  processing completed successfully
#define SI_QUEUED      1         //  send messages was queued

                                  //  flags passed to signal callback
#define SI_SF_QUIT     0x01      //  program should terminate
#define SI_SF_USR1     0x02      //  user 1 signal received
#define SI_SF_USR2     0x04      //  user 2 signal received
#define SI_SF_ALRM     0x08      //  alarm clock signal received

                                 //  signal bitmasks for the setsig routine
#define SI_SIG_QUIT    0x01      //  please catch quit signal
#define SI_SIG_HUP     0x02      //  catch hangup signal
#define SI_SIG_TERM    0x04      //  catch the term signal
#define SI_SIG_USR1    0x08      //  catch user signals
#define SI_SIG_USR2    0x10
#define SI_SIG_ALRM    0x20      //  catch alarm signals
#define SI_DEF_SIGS    0x1F      //  default signals to catch

                                 //  SIerrno values set in public rtns
#define SI_ERR_NONE     0        //  no error as far as we can tell
#define SI_ERR_QUEUED   1	//  must be same as queued
#define SI_ERR_TPORT    2        //  could not bind to requested tcp port
#define SI_ERR_UPORT    3        //  could not bind to requested udp port
#define SI_ERR_FORK     4        //  could not fork to become daemon
#define SI_ERR_HANDLE   5        //  global information pointer went off
#define SI_ERR_SESSID   6        //  invalid session id
#define SI_ERR_TP       7        //  error occured in transport provider
#define SI_ERR_SHUTD    8        //  cannot process because in shutdown mode
#define SI_ERR_NOFDS    9        //  no file descriptors are open
#define SI_ERR_SIGUSR1  10       //  signal received data not read
#define SI_ERR_SIGUSR2  11       //  signal received data not read
#define SI_ERR_DISC     12       //  session disconnected
#define SI_ERR_TIMEOUT  13       //  poll attempt timed out - no data
#define SI_ERR_ORDREL   14       //  orderly release received
#define SI_ERR_SIGALRM  15       //  alarm signal received
#define SI_ERR_NOMEM    16       //  could not allocate needed memory
#define SI_ERR_ADDR    	17       //  address conversion failed
#define SI_ERR_BLOCKED	18		// operation would block

#define SI_TF_NONE		0		// tcp flags in the global info applied to each session
#define SI_TF_NODELAY	0x01	// set nagle's off for each connection
#define SI_TF_FASTACK	0x02	// set fast ack on for each connection

#ifndef _SI_ERRNO
extern int SIerrno;               //  error number set by public routines
#define _SI_ERRNO
#endif

typedef struct ginfo_blk si_ctx_t;		// generic context reference for users

#endif

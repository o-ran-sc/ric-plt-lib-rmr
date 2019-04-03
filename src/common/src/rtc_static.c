// : vi ts=4 sw=4 noet :
/*
==================================================================================
	Copyright (c) 2019 Nokia
	Copyright (c) 2018-2019 AT&T Intellectual Property.

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
	Mnemonic:	rt_collector.c
	Abstract:	The route table collector is started as a separate pthread and
				is responsible for listening for route table updates from a
				route manager or route table generator process.

	Author:		E. Scott Daniels
	Date:		29 November 2018 (extracted to common 13 March 2019)
*/

#ifndef _rt_collector_c
#define _rt_collector_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
	Route Table Collector
	A side thread which opens a socket and subscribes to a routing table generator.
	It may do other things along the way (latency measurements?).

	The pointer is a pointer to the context.

	Listens for records from the route table generation publisher, expecting
	one of the following, newline terminated, ASCII records:
		rte|msg-type||]name:port,name:port,...;name:port,...			// route table entry with one or more groups of endpoints
		new|start								// start of new table
		new|end									// end of new table; complete

		Name must be a host name which can be looked up via gethostbyname() (DNS).

		Multiple endpoints (name:port) may be given separated by a comma; an endpoint is selected using round robin
			for each message of the type that is sent.

		Multiple endpoint groups can be given as a comma separated list of endpoints, separated by semicolons:
				group1n1:port,group1n2:port,group1n3:port;group2n1:port,group2n2:port

		If multiple groups are given, when send() is called for the cooresponding message type,
		the message will be sent to one endpoint in each group.

		msg-type is the numeric message type (e.g. 102). If it is given as n,name then it is assumed
		that the entry applies only to the instance running with the hostname 'name.'

	Buffers received from the route table generator can contain multiple newline terminated
	records, but each buffer must be less than 4K in length, and the last record in a
	buffere may NOT be split across buffers.

*/
static void* rtc( void* vctx ) {
	uta_ctx_t*	ctx;					// context user has -- where we pin the route table
	uta_ctx_t*	pvt_cx;					// private context for session with rtg
	rmr_mbuf_t*	msg = NULL;				// message from rtg
	char*	payload;					// payload in the message
	size_t	mlen;
	size_t	clen;						// length to copy and mark
	char*	port;						// a port number we listen/connect to
	char*	fport;						// pointer to the real buffer to free
	size_t	buf_size;					// nng needs var pointer not just size?
	char*	nextr;						// pointer at next record in the message	
	char*	curr;						// current record
	int 	i;
	long	blabber = 0;				// time of last blabber so we don't flood if rtg goes bad
	int		cstate = -1;				// connection state to rtg
	int		state;						// processing state of some nng function
	char*	tokens[128];
	char	wbuf[128];
	char*	pbuf;
	int		pbuf_size = 0;				// number allocated in pbuf
	int		ntoks;
	int		raw_interface = 1;			// rtg is using raw NNG/Nano not RMr to send updates
	int		vfd = -1;					// verbose file des if we have one
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char*	eptr;

	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		fprintf( stderr, "[CRI] rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	if( (eptr = getenv( ENV_VERBOSE_FILE )) != NULL ) {
		vfd = open( eptr, O_RDONLY ); 
		if( vfd >= 0 ) {
			wbuf[0] = 0;
			lseek( vfd, 0, 0 );
			read( vfd, wbuf, 10 );
			vlevel = atoi( wbuf );
		}
	}

	read_static_rt( ctx, vlevel );						// seed the route table if one provided

	if( (port = getenv( ENV_RTG_PORT )) == NULL || ! *port ) {		// port we need to open to listen for RTG connections
		port = strdup( DEF_RTG_PORT );
	} else {
		port = strdup( port );
	}

	if( (curr = getenv( ENV_RTG_RAW )) != NULL ) {
		raw_interface = atoi( curr ) > 0;				// if > 0 we assume that rtg messages are NOT coming from an RMr based process
	}

	fport = port;		// must hold to free

	ntoks = uta_tokenise( port, tokens, 120, ':' );			// assume tcp:port, but it could be port or old style host:port
	switch( ntoks ) {
		case 1:	
				port = tokens[0];			// just the port
				break;

		case 2:
				port = tokens[1];			// tcp:port or :port
				break;

		default:
				port = DEF_RTG_PORT;		// this shouldn't happen, but parnioia is good
				break;
	}

	if( (pvt_cx = init( port, MAX_RTG_MSG_SZ, FL_NOTHREAD )) == NULL ) {				// open a private context
		fprintf( stderr, "[CRI] rmr_rtc: unable to initialise listen port for RTG (pvt_cx)\n" );
		free( fport );
		return NULL;
	}

	if( DEBUG ) fprintf( stderr, "[DBUG] rtc thread is running and listening; listening for rtg conns on %s\n", port );
	free( fport );

	// future:  if we need to register with the rtg, then build a message and send it through a wormhole here

	blabber = 0;
	while( 1 ) {			// until the cows return, pigs fly, or somesuch event likely not to happen
		if( raw_interface ) {
			msg = (rmr_mbuf_t *) rcv_payload( pvt_cx, msg );		// receive from non-RMr sender
		} else {
			msg = rmr_rcv_msg( pvt_cx, msg );		// receive from an RMr sender
		}

		if( vfd >= 0 ) {							// if changed since last go round
			wbuf[0] = 0;
			lseek( vfd, 0, 0 );
			read( vfd, wbuf, 10 );
			vlevel = atoi( wbuf );
		}

		if( msg != NULL && msg->len > 0 ) {
			payload = msg->payload;
			mlen = msg->len;					// usable bytes in the payload
			if( vlevel > 1 ) {
				fprintf( stderr, "[DBUG] rmr_rtc: received rt message; %d bytes (%s)\n", (int) mlen, msg->payload );
			} else {
				if( DEBUG > 1 || (vlevel > 0) ) fprintf( stderr, "[DBUG] rmr_rtc: received rt message; %d bytes\n", (int) mlen );
			}

			if( pbuf_size <= mlen ) {
				if( pbuf ) {
					free( pbuf );
				}
				pbuf = (char *) malloc( sizeof( char ) * mlen *2 );
				pbuf_size = mlen * 2;
			}
			memcpy( pbuf, payload, mlen );
			pbuf[mlen] = 0;										// don't depend on sender making this a legit string

			curr = pbuf;
			while( curr ) {								// loop over each record in the buffer
				nextr = strchr( curr, '\n' );			// allow multiple newline records, find end of current and mark
	
				if( nextr ) {
					*(nextr++) = 0;
				}
	
				if( vlevel > 1 ) {
					fprintf( stderr, "[DBUG] rmr_rtc: processing (%s)\n", curr );
				}
				parse_rt_rec( ctx, curr, vlevel );		// parse record and add to in progress table
		
				curr = nextr;
			}
	
			if( ctx->shutdown ) {		// mostly for testing, but allows user app to close us down if rmr_*() function sets this
				break;
			}
		} else {
			if( time( NULL ) > blabber  ) {
				fprintf( stderr, "[WRN] rmr_rtc: nil buffer, or 0 len msg, received from rtg\n" );
				blabber = time( NULL ) + 180;			// limit to 1 every 3 min or so
			}
		}
	}

	return NULL;	// unreachable, but some compilers don't see that and complain.
}


#endif

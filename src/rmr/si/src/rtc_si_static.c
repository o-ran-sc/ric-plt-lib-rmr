// : vi ts=4 sw=4 noet :
/*
==================================================================================
	Copyright (c) 2019-2020 Nokia
	Copyright (c) 2018-2020 AT&T Intellectual Property.

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
	Mnemonic:	rtc_si_static.c
	Abstract:	The route table collector is started as a separate pthread and
				is responsible for listening for route table updates from a
				route manager or route table generator process.

				This comes from the common src and may be moved back there once
				it is not necessary to support raw sessions (all route table
				gen messages are received over rmr channel).

	Author:		E. Scott Daniels
	Date:		29 November 2018 (extracted to common 13 March 2019)
				Imported to si base 17 Jan 2020.
*/


#ifndef _rtc_si_staic_c
#define _rtc_si_staic_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
	Loop forever (assuming we're running in a pthread reading the static table
	every minute or so.
*/
static void* rtc_file( void* vctx ) {
	uta_ctx_t*	ctx;					// context user has -- where we pin the route table
	char*	eptr;
	int		vfd = -1;					// verbose file des if we have one
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char	wbuf[256];


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	if( (eptr = getenv( ENV_VERBOSE_FILE )) != NULL ) {
		vfd = open( eptr, O_RDONLY );
	}

	while( 1 ) {
		if( vfd >= 0 ) {
			wbuf[0] = 0;
			lseek( vfd, 0, 0 );
			read( vfd, wbuf, 10 );
			vlevel = atoi( wbuf );
		}

		read_static_rt( ctx, vlevel );						// seed the route table if one provided

		sleep( 60 );
	}

}

static int refresh_vlevel( int vfd ) {
	int vlevel = 0;
	char	rbuf[128];

	if( vfd >= 0 ) {					// if file is open, read current value
		rbuf[0] = 0;
		lseek( vfd, 0, 0 );
		read( vfd, rbuf, 10 );
		vlevel = atoi( rbuf );
	}

	return vlevel;
}

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
	buffer may NOT be split across buffers.

	Other chores:
	In addition to the primary task of getting, vetting, and installing a new route table, or
	updates to the existing table, this thread will periodically cause the send counts for each
	endpoint known to be written to standard error. The frequency is once every 180 seconds, and
	more frequently if verbose mode (see ENV_VERBOSE_FILE) is > 0.
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
	char*	pbuf = NULL;
	int		pbuf_size = 0;				// number allocated in pbuf
	int		ntoks;
	int		raw_interface = 0;			// rtg is using raw NNG/Nano not RMr to send updates
	int		vfd = -1;					// verbose file des if we have one
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char*	eptr;
	int		epfd = -1;					// fd for epoll so we can multi-task
	struct  epoll_event events[1];		// list of events to give to epoll; we only have one we care about
	struct  epoll_event epe;			// event definition for event to listen to
	int		count_delay = 30;			// number of seconds between writing count info; initially every 30s
	int		bump_freq = 0;				// time at which we will bump count frequency to every 5 minutes


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	if( (eptr = getenv( ENV_VERBOSE_FILE )) != NULL ) {
		vfd = open( eptr, O_RDONLY );
		vlevel = refresh_vlevel( vfd );
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

	if( (pvt_cx = init( port, MAX_RTG_MSG_SZ, FL_NOTHREAD )) == NULL ) {				// open a private context (no RT listener!)
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: unable to initialise listen port for RTG (pvt_cx)\n" );

		while( TRUE ) {												// no listen port, just dump counts now and then
			sleep( count_delay );
			rt_epcounts( ctx->rtable, ctx->my_name );
		}

		free( fport );					// parinoid free and return
		return NULL;
	}

	if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "rtc thread is running and listening; listening for rtg conns on %s\n", port );
	free( fport );

	// future:  if we need to register with the rtg, then build a message and send it through a wormhole here

	bump_freq = time( NULL ) + 300;				// after 5 minutes we decrease the count frequency
	blabber = 0;
	while( 1 ) {			// until the cows return, pigs fly, or somesuch event likely not to happen
		while( msg == NULL || msg->len <= 0 ) {							// until we actually have something from the other side
			msg = rmr_torcv_msg( pvt_cx, msg, 1000 );

			if( time( NULL ) > blabber  ) {
				vlevel = refresh_vlevel( vfd );
				if( vlevel >= 0 ) {										// allow it to be forced off with -n in verbose file
					blabber = time( NULL ) + count_delay;				// set next time to blabber, then do so
					if( blabber > bump_freq ) {
						count_delay = 300;
					}
					rt_epcounts( ctx->rtable, ctx->my_name );
				}
			}
		}

		vlevel = refresh_vlevel( vfd );			// ensure it's fresh when we get a message

		if( msg != NULL && msg->len > 0 ) {
			payload = msg->payload;
			mlen = msg->len;					// usable bytes in the payload
			if( vlevel > 1 ) {
				rmr_vlog( RMR_VL_DEBUG, "rmr_rtc: received rt message; %d bytes (%s)\n", (int) mlen, msg->payload );
			} else {
				if( DEBUG > 1 || (vlevel > 0) ) rmr_vlog( RMR_VL_DEBUG, "rmr_rtc: received rt message; %d bytes\n", (int) mlen );
			}

			if( pbuf_size <= mlen ) {
				if( pbuf ) {
					free( pbuf );
				}
				if( mlen < 512 ) {
					pbuf_size = 512;
				} else {
					pbuf_size = mlen * 2;
				}
				pbuf = (char *) malloc( sizeof( char ) * pbuf_size );
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
					rmr_vlog( RMR_VL_DEBUG, "rmr_rtc: processing (%s)\n", curr );
				}
				parse_rt_rec( ctx, curr, vlevel );		// parse record and add to in progress table

				curr = nextr;
			}

			if( ctx->shutdown ) {		// mostly for testing, but allows user app to close us down if rmr_*() function sets this
				break;
			}

			msg->len = 0;				// force back into the listen loop
		}
	}

	return NULL;	// unreachable, but some compilers don't see that and complain.
}


#endif

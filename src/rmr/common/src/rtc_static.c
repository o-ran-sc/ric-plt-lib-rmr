// : vi ts=4 sw=4 noet :
/*
==================================================================================
	Copyright (c) 2019-2021 Nokia
	Copyright (c) 2018-2021 AT&T Intellectual Property.

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

#include <RIC_message_types.h>		// needed for RMR/Rt Mgr msg types

// ---- local constants ------------------
										// flags
#define RTCFL_HAVE_UPDATE	0x01		// an update from RM was received

#define MAX_RTC_BUF	5 * 1024			// max buffer size we'll expect is 4k, add some fudge room

// ------------------------------------------------------------------------------------------------

/*
	Opens the vlevel control file if needed and reads the vlevel from it.
	The file is rewound if already open so that external updates are captured.
	The current level is returnd; 0 on error.

	The environment variable (ENV_VERBOSE_FILE) is used to supply the file to
	open and read. If missing, we will try /tmp/rmr.v.  We will try to open the file
	on each call if not alrady open; this allows the value to be supplied after
	start which helps debugging.

	If close_file is true, then we will close the open vfd and return 0;
*/
extern int refresh_vlevel( int close_file ) {
	static int vfd = -1;

	char*	eptr;
	char	wbuf[128];			// read buffer; MUST be 11 or greater
	int		vlevel = 0;

	if( close_file ) {
		if( vfd >= 0 ) {
			close( vfd );
			vfd = -1;
		}
		return 0;
	}

	if( vfd < 0 ) {				// attempt to find/open on all calls if not open
		if( (eptr = getenv( ENV_VERBOSE_FILE )) != NULL ) {
			vfd = open( eptr, O_RDONLY );
		} else {
			vfd = open( "/tmp/rmr.v", O_RDONLY );
		}
		if( vfd < 0 ) {
			return 0;
		}
	}

	memset( wbuf, 0, sizeof( char ) * 11 );			// ensure what we read will be nil terminated
	if( lseek( vfd, 0, SEEK_SET ) == 0 && read( vfd, wbuf, 10 ) > 0 ) {
		vlevel = atoi( wbuf );
	}

	return vlevel;
}

/*
	Loop forever (assuming we're running in a pthread reading the static table
	every minute or so.
*/
static void* rtc_file( void* vctx ) {
	uta_ctx_t*	ctx;					// context user has -- where we pin the route table
	char*	eptr;
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char	wbuf[256];


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	ctx->flags |= CFL_NO_RTACK;				// no attempt to ack when reading from a file
	while( 1 ) {
		vlevel = refresh_vlevel( 0 );
		read_static_rt( ctx, vlevel );						// refresh from the file

		if( ctx->shutdown != 0 ) {							// allow for graceful termination and unit testing
			refresh_vlevel( 1 );								// close the verbose file if open
			return NULL;
		}

		if( ctx->rtable_ready ) {
			sleep( 60 );
		} else {
			sleep( 1 );										// check every second until we have a good one
		}
	}
}

/*
	Rtc_parse_msg parses a single message from the route manager. We allow multiple, newline terminated,
	records in each message; it is required that the last record in the message be complete (we do not
	reconstruct records split over multiple messages).  For each record, we call the record parser
	to parse and add the information to the table being built.

	This function was broken from the main rtc() function in order to be able to unit test it. Without
	this as a standalone funciton, it was impossible to simulate a message arriving on the RTC's private
	context.

	To reduce malloc/free cycles, we allocate a static work buffer and expand it when needed; in other
	words, this is not thread safe but it shouldn't need to be.
*/
static void rtc_parse_msg( uta_ctx_t *ctx, uta_ctx_t* pvt_cx, rmr_mbuf_t* msg, int vlevel,  int* flags ) {
	static	unsigned char* pbuf = NULL;
	static	int pbuf_size = 0;

	unsigned char* payload;
	unsigned char* curr;
	unsigned char* nextr;
	int mlen;

	payload = msg->payload;
	mlen = msg->len;					// usable bytes in the payload

	if( DEBUG > 1 || (vlevel > 0) ) rmr_vlog( RMR_VL_DEBUG, "rmr_rtc: received rt message type=%d len=%d\n", msg->mtype, (int) mlen );
	switch( msg->mtype ) {
		case RMRRM_TABLE_DATA:
			if( (*flags & RTCFL_HAVE_UPDATE) == 0 ) {
				*flags |= RTCFL_HAVE_UPDATE;
				rmr_vlog( RMR_VL_INFO, "message flow from route manager starts\n" );
			}

			if( pbuf_size <= mlen ) {
				if( pbuf ) {
					free( pbuf );
				}
				if( mlen < 512 ) {
					pbuf_size = 1024;
				} else {
					pbuf_size = mlen * 2;
				}
				pbuf = (char *) malloc( sizeof( char ) * pbuf_size );
			}
			memcpy( pbuf, payload, mlen );
			pbuf[mlen] = 0;										// don't depend on sender making this a legit string
			if( vlevel > 1 ) {
				rmr_vlog_force( RMR_VL_DEBUG, "rmr_rtc: rt message: (%s)\n", pbuf );
			}

			curr = pbuf;
			while( curr ) {										// loop over each record in the buffer
				nextr = strchr( (char *) curr, '\n' );			// allow multiple newline records, find end of current and mark

				if( nextr ) {
					*(nextr++) = 0;
				}

				if( vlevel > 1 ) {
					rmr_vlog_force( RMR_VL_DEBUG, "rmr_rtc_parse_msg: snarf_fd=%d processing (%s)\n", ctx ? ctx->snarf_rt_fd : -99, curr );
				}
				parse_rt_rec( ctx, pvt_cx, curr, vlevel, msg );		// parse record and add to in progress table; ack using rts to msg

				curr = nextr;
			}

			msg->len = 0;				// force back into the listen loop
			break;

		default:
			rmr_vlog( RMR_VL_WARN, "rmr_rtc: invalid message type=%d len=%d\n", msg->mtype, (int) msg->len );
			break;
	}
}

/*
	Route Table Collector
	A side thread which either attempts to connect and request a table
	from the Route Manager, or opens a port and listens for Route Manager
	to push table updates.

	It may do other things along the way (latency measurements, alarms,
	respond to RMR pings, etc.).

	The behaviour with respect to listening for Route Manager updates vs
	the initiation of the connection and sending a request depends on the
	value of the ENV_RTG_ADDR (RMR_RTG_SVC)  environment variable. If
	host:port, or IP:port, is given, then we assume that we make the connection
	and send a request for the table (request mode).  If the variable is just
	a port, then we assume Route Manager will connect and push updates (original
	method).

	If the variable is not defined, the default behaviour, in order to be
	backwards compatable, depends on the presence of the ENV_CTL_PORT
	(RMR_CTL_PORT) variable (new with the support for requesting a table).


	ENV_CTL_PORT    ENV_RTG_ADDR	Behaviour
	unset			unset			Open default CTL port (DEF_CTL_PORT) and
									wait for Rt Mgr to push tables

	set				unset			Use the default Rt Mgr wellknown addr
									and port (DEF_RTG_WK_ADDR) to connect
									and request a table. The control port
									used is the value set by ENV_CTL_PORT.

	unset			set				As described above. The default control
									port (DEF_CTL_PORT) is used.

	When we are running in request mode, then we will send the RMR message
	RMRRM_REFRESH to this address (wormhole) as a request for the route manager
	to send a new table. We will attempt to connect and send requests until
	we have a table. Calls to rmr_ready() will report FALSE until a table is
	loaded _unless_ a seed table was given.

	Route table information is expected to arrive on RMR messages with type
	RMRRM_TABLE_DATA.  There is NOT a specific message type for each possible
	table record, so the payload is as it appears in the seed file or as
	delivered in old versions.  It may take several RMRRM_TABLE_DATA messages
	to completely supply a new table or table update. See the header for parse_rt_rec
	in common for a description of possible message contents.

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
	route_table_t* rt;					// the routing table that will be traversed to print statistics
	char*	my_port;					// the port number that we will listen on (4561 has been the default for this)
	char*	rtg_addr;					// host:port address of route table generator (route manager)
	char*	daddr;						// duplicated rtg address string to parse/trash
	size_t	buf_size;					// nng needs var pointer not just size?
	int		i;
	long	blabber = 0;				// time of last blabber so we don't flood if rtg goes bad
	int		cstate = -1;				// connection state to rtg
	int		state;						// processing state of some nng function
	char*	tokens[128];
	char	wbuf[128];
	int		ntoks;
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char*	eptr;
	int		epfd = -1;					// fd for epoll so we can multi-task
	struct  epoll_event events[1];		// list of events to give to epoll; we only have one we care about
	struct  epoll_event epe;			// event definition for event to listen to
	int		count_delay = 30;			// number of seconds between writing count info; initially every 30s
	int		bump_freq = 0;				// time at which we will bump count frequency to every 5 minutes
	int		flags = 0;
	int		rt_req_freq = DEF_RTREQ_FREQ;	// request frequency (sec) when wanting a new table
	int		nxt_rt_req = 0;					// time of next request


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	vlevel = refresh_vlevel( 0 );

	if( (eptr = getenv( ENV_RTREQ_FREA )) != NULL ) {
		rt_req_freq = atoi( eptr );
		if( rt_req_freq < 1 || rt_req_freq > 300 ) {
			rt_req_freq = DEF_RTREQ_FREQ;
			rmr_vlog( RMR_VL_WARN, "rmr_rtc: RT request frequency (%d) out of range (1-300), using default (%d)\n", rt_req_freq, DEF_RTREQ_FREQ );
		}
	}
	rmr_vlog( RMR_VL_INFO, "rmr_rtc: RT request frequency set to: %d seconds", rt_req_freq );

	ctx->flags |= CFL_NO_RTACK;				// don't ack when reading from a file
	read_static_rt( ctx, vlevel );			// seed the route table if one provided
	ctx->flags &= ~CFL_NO_RTACK;
	ctx->flags &= ~CFL_FULLRT;				// even though rmr-ready goes true, the seed doesn't count as a full RT from route generator


	my_port = getenv( ENV_CTL_PORT );				// default port to listen on (likely 4561)
	if( my_port == NULL || ! *my_port ) {			// if undefined, then go with default
		my_port = DEF_CTL_PORT;
		daddr = DEF_CTL_PORT;						// backwards compat; if ctl port not hard defined, default is to listen
	} else {
		daddr = DEF_RTG_WK_ADDR;					// if ctl port is defined, then default changes to connecting to well known RM addr
	}

	if( (rtg_addr = getenv( ENV_RTG_ADDR )) == NULL || ! *rtg_addr ) {		// undefined, use default set above
		rtg_addr = daddr;
	}

	daddr = strdup( rtg_addr );									// dup to destroy during parse

	ntoks = uta_tokenise( daddr, tokens, 120, ':' );			// should be host:ip of rt mgr (could be port only which we assume is old listen port)
	switch( ntoks ) {
		case 0:									// should not happen, but prevent accidents and allow default to ignore additional tokens
			break;

		case 1:
			my_port = tokens[0];			// just port -- assume backlevel environment where we just listen
			flags |= RTCFL_HAVE_UPDATE;		// prevent sending update reqests
			break;

		default:
			if( strcmp( tokens[0], "tcp" ) == 0 ) {			// old school nng tcp:xxxx so we listen on xxx
				flags |= RTCFL_HAVE_UPDATE;					// and signal not to try to request an update
				my_port = tokens[1];
			} else {
				// rtg_addr points at rt mgr address and my port set from env or default stands as is
			}
			break;
	}

	if( (pvt_cx = init( my_port, MAX_RTC_BUF, FL_NOTHREAD )) == NULL ) {				// open a private context (no RT listener!)
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: unable to initialise listen port for RTG (pvt_cx)\n" );

		while( TRUE ) {												// no listen port, just dump counts now and then
			sleep( count_delay );
			rt_epcounts( ctx->rtable, ctx->my_name );
		}

		return NULL;
	}

	ctx->rtg_whid = -1;

	cycle_snarfed_rt( ctx );				// cause the nrt to be opened

	if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "rtc thread is running and listening; listening for rtg conns on %s\n", my_port );

	bump_freq = time( NULL ) + 300;				// after 5 minutes we decrease the count frequency
	blabber = 0;
	while( 1 ) {														// until the cows return, pigs fly, or somesuch event likely not to happen
		while( msg == NULL || msg->len <= 0 ) {							// until we actually have something from the other side
			if( (flags & RTCFL_HAVE_UPDATE) == 0 && time( NULL ) >= nxt_rt_req ) {			// no route table updated from rt mgr; request one
				if( ctx->rtg_whid < 0 ) {
					ctx->rtg_whid = rmr_wh_open( pvt_cx, rtg_addr );
				}
				send_update_req( pvt_cx, ctx );
				nxt_rt_req = time( NULL ) + rt_req_freq;
			}

			msg = rmr_torcv_msg( pvt_cx, msg, 1000 );

			if( time( NULL ) > blabber  ) {
				vlevel = refresh_vlevel( 0 );
				blabber = time( NULL ) + count_delay;				// set next time to blabber, then do so
				if( blabber > bump_freq ) {
					count_delay = 300;
				}
				if( vlevel >= 0 ) {										// allow it to be forced off with -n in verbose file
					rt = get_rt( ctx );									// get active route table and up ref count
					rt_epcounts( rt, ctx->my_name );
					release_rt( ctx, rt );								// dec safely the ref counter
				}
			}

			if( ctx->shutdown != 0 ) {
				break;							// mostly for unit test, but allows a forced stop
			}

												// extra housekeeping chores can be added here...
			alarm_if_drops( ctx, pvt_cx );		// send an alarm if message are dropping, clear if we set one and thtings are better
		}

		vlevel = refresh_vlevel( 0 );			// ensure it's fresh when we get a message

		if( msg != NULL && msg->len > 0 ) {
			rtc_parse_msg( ctx, pvt_cx, msg, vlevel, &flags );
		}

		if( ctx->shutdown ) {		// mostly for testing, but allows user app to close us down if rmr_*() function sets this
			return NULL;
		}

	}

	return NULL;	// unreachable, but some compilers don't see that and complain.
}

#ifndef SI95_BUILD
// this is nng specific inas much as we allow raw (non-RMR) messages

/*
	NOTE:	This is the original rtc code when we supported "raw" nano/nng messages
			from the route manger.  It is deprecated in favour of managing all RM-RMR
			communications via an RMR session.

			The rtc() function above is the new and preferred function regardless
			of transport.

	-----------------------------------------------------------------------------------
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
static void* raw_rtc( void* vctx ) {
	uta_ctx_t*	ctx;					// context user has -- where we pin the route table
	uta_ctx_t*	pvt_cx;					// private context for session with rtg
	rmr_mbuf_t*	msg = NULL;				// message from rtg
	route_table_t* rt;					// the routing table that will be traversed to print statistics
	char*	payload;					// payload in the message
	size_t	mlen;
	char*	port;						// a port number we listen/connect to
	char*	fport;						// pointer to the real buffer to free
	size_t	buf_size;					// nng needs var pointer not just size?
	char*	nextr;						// pointer at next record in the message
	char*	curr;						// current record
	int		i;
	long	blabber = 0;				// time of last blabber so we don't flood if rtg goes bad
	int		cstate = -1;				// connection state to rtg
	int		state;						// processing state of some nng function
	char*	tokens[128];
	char	wbuf[128];
	char*	pbuf = NULL;
	int		pbuf_size = 0;				// number allocated in pbuf
	int		ntoks;
	int		raw_interface = 1;			// rtg is using raw NNG/Nano not RMr to send updates
	int		vlevel = 0;					// how chatty we should be 0== no nattering allowed
	char*	eptr;
	int		epfd = -1;					// fd for epoll so we can multi-task
	struct  epoll_event events[1];		// list of events to give to epoll; we only have one we care about
	struct  epoll_event epe;			// event definition for event to listen to
	int		rcv_fd = -1;				// pollable file des from NNG to use for timeout
	int		count_delay = 30;			// number of seconds between writing count info; initially every 30s
	int		bump_freq = 0;				// time at which we will bump count frequency to every 5 minutes


	if( (ctx = (uta_ctx_t *) vctx) == NULL ) {
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: internal mishap: context passed in was nil\n" );
		return NULL;
	}

	vlevel = refresh_vlevel( 0 );
	read_static_rt( ctx, vlevel );						// seed the route table if one provided

	if( (port = getenv( ENV_RTG_PORT )) == NULL || ! *port ) {		// port we need to open to listen for RTG connections
		port = strdup( DEF_RTG_PORT );
	} else {
		port = strdup( port );
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
		rmr_vlog( RMR_VL_CRIT, "rmr_rtc: unable to initialise listen port for RTG (pvt_cx)\n" );

		while( TRUE ) {												// no listen port, just dump counts now and then
			sleep( count_delay );
			rt_epcounts( ctx->rtable, ctx->my_name );
		}

		free( fport );					// parinoid free and return
		return NULL;
	}

	if( (rcv_fd = rmr_get_rcvfd( pvt_cx )) >= 0 ) {            // get the epoll fd for the rtg socket
		if( rcv_fd < 0 ) {
			rmr_vlog( RMR_VL_WARN, "cannot get epoll fd for rtg session; stats will generate only after update from rt manager\n" );
		} else {
			if( (epfd = epoll_create1( 0 )) < 0 ) {
				rmr_vlog( RMR_VL_WARN, "stats will generate only after rt manager update; unable to create epoll fd for rtg session: %s\n", strerror( errno ) );
				rcv_fd = -1;
			} else {
				epe.events = EPOLLIN;
				epe.data.fd = rcv_fd;

				if( epoll_ctl( epfd, EPOLL_CTL_ADD, rcv_fd, &epe ) != 0 )  {
					rmr_vlog( RMR_VL_WARN, "stats will generate only after rt manager update; unable to init epoll_ctl: %s\n", strerror( errno ) );
					rcv_fd = -1;
				}
			}
		}
	}

	if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "rtc thread is running and listening; listening for rtg conns on %s\n", port );
	free( fport );

	// future:  if we need to register with the rtg, then build a message and send it through a wormhole here

	bump_freq = time( NULL ) + 300;				// after 5 minutes we decrease the count frequency
	blabber = 0;
	while( 1 ) {			// until the cows return, pigs fly, or somesuch event likely not to happen
		while( msg == NULL || msg->len <= 0 ) {							// until we actually have something from the other side
			if( rcv_fd < 0 || epoll_wait( epfd, events, 1, 1000 ) > 0 )  {	// skip epoll if init failed, else block for max 1 sec
				if( raw_interface ) {
					msg = (rmr_mbuf_t *) rcv_payload( pvt_cx, msg );		// receive from non-RMr sender
				} else {
					msg = rmr_rcv_msg( pvt_cx, msg );		// receive from an RMr sender
				}
			} else {													// no msg, do extra tasks
				if( msg != NULL ) {										// if we were working with a message; ensure no len
					msg->len = 0;
					msg->state = RMR_ERR_TIMEOUT;
				}
			}

			if( time( NULL ) > blabber  ) {
				vlevel = refresh_vlevel( 0 );
				if( vlevel >= 0 ) {										// allow it to be forced off with -n in verbose file
					blabber = time( NULL ) + count_delay;				// set next time to blabber, then do so
					if( blabber > bump_freq ) {
						count_delay = 300;
					}
					rt = get_rt( ctx );									// get active route table and up ref count
					rt_epcounts( rt, ctx->my_name );
					release_rt( ctx, rt );								// dec safely the ref counter
				}
			}

			alarm_if_drops( ctx );				// send an alarm if message are dropping, clear if we set one and thtings are better
		}

		vlevel = refresh_vlevel( 0 );			// ensure it's fresh when we get a message

		if( msg != NULL && msg->len > 0 ) {
			payload = msg->payload;
			mlen = msg->len;					// usable bytes in the payload
			if( vlevel > 1 ) {
				rmr_vlog_force( RMR_VL_DEBUG, "rmr_rtc: received rt message; %d bytes (%s)\n", (int) mlen, msg->payload );
			} else {
				if( DEBUG > 1 || (vlevel > 0) ) rmr_vlog_force( RMR_VL_DEBUG, "rmr_rtc: received rt message; %d bytes\n", (int) mlen );
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
					rmr_vlog_force( RMR_VL_DEBUG, "rmr_rtc: processing (%s)\n", curr );
				}
				if( raw_interface ) {
					parse_rt_rec( ctx, NULL, curr, vlevel, NULL );		// nil pvt to parser as we can't ack messages
				} else {
					parse_rt_rec( ctx, pvt_cx, curr, vlevel, msg );		// parse record and add to in progress table
				}

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


#endif

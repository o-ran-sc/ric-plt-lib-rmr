// :vim ts=4 sw=4 noet:
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
	Mnemonic:	health_ck.c
	Abstract:	This is a generic, and very simplistic, health check application
				which will send n RMR messages to an application (localhost:4560
				by default) and expect to receive n responses.  Messages are sent
				with the message type RIC_HEALTH_CHECK_REQ and responses are 
				expected to have a message type RIC_HEALTH_CHECK_RESP (defined in
				the RIC message type header file).

				Responses are expected to have a payload containing ASCII data.
				The first, space separated token, is expected to be one of:
					OK
					ERR

				to indicate the state of the response. The ERR token may optionally
				be followed with a text string; when present it will be written on
				standard error as an aide to problem determination if needed. 

				The programme exit code will be 0 on success (all received messages
				had the OK token), or 1 to indicate failure. A failure reason will
				also be written to standard error.

				Command line options and parameters:
					[-h host:port]		target application to "ping"
					[-n num-msgs]		total number of "pings" to send
					[-p port]			specific port to open and use for responses
					[-t seconds]		max timeout (default 3 seconds)

				Route table:  While we don't need a route table to do wormhole sends we
				do need for RMR to initialise an empty one. To avoid having to have a
				dummy table on disk somewhere, we'll create one and "point" RMR at it.

	Date:		26 February 2020
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <fcntl.h>

#include <rmr/rmr.h>
#include <rmr/RIC_message_types.h>

#ifndef RIC_HEALTH_CHECK_REQ
#define RIC_HEALTH_CHECK_REQ	100
#define RIC_HEALTH_CHECK_RESP	101
#endif


#define MAX_MSG_SZ	2048		// max size we'll expect back
#define PAYLOAD_SZ	1024		// size of payload we will send

#define VERBOSE(...) if(verbose){ fprintf( stderr,  "[INFO] " __VA_ARGS__);}

/*
	we can compute the latency.
typedef struct trace_data {
	char	id[10];					// allows us to verify that it's valid
	struct	timespec out_ts;		// time this payload was sent
} trace_data_t;
*/

// ---------------------------------------------------------------------------

/*
	Compute the elapsed time between ts1 and ts2.
	Returns mu-seconds.
*/
static int elapsed( struct timespec* start_ts, struct timespec* end_ts ) {
	long long start;
	long long end;
	int bin;

	start = ( start_ts->tv_sec * 1000000000) + start_ts->tv_nsec;
	end = ( end_ts->tv_sec * 1000000000) + end_ts->tv_nsec;

	bin = (end - start) / 1000;			// to mu-sec
	//bin = (end - start);

	return bin;
}

int main( int argc, char** argv ) {
	int		ai = 1;							// arg index
	int		i;

	void* mrc;      						// msg router context
	rmr_mbuf_t*		mbuf;					// message buffer
	char*	payload;						// direct reference to msg payload
	long	expiry;							// point at which we give up (expire)
	long	now;							// current time
	long	max_timeout = 3;				// (seconds) -t to overrride
	char*	target = "localhost:4560";		// address of target to ping
	char*	listen_port = NULL;				// the port we open for "backhaul" connections (somewhat random by default)
	int		rand_port = 1;					// -r turns off and we use a default port value
	char*	tok;							// pointer at token in a buffer
	char	wbuf[MAX_MSG_SZ];				// work buffer
	char*	rbuf;							// spot to work on the response
	int		whid;							// id of wormhole
	int		num2send = 1;					// number of messages to send
	int		count = 0;
	int		good = 0;						// num good messages back
	int		verbose = 0;
	int		et;								// elapsed time
	struct	timespec out_ts;				// time we sent the message
	struct	timespec in_ts;					// time we got response

	// ---- simple arg parsing ------
	while( ai < argc ) {
		if( *argv[ai] == '-' ) {
			switch( argv[ai][1] ) {
				case 'h':					// host:port
					ai++;
					target = strdup( argv[ai] );
					break;

				case 'n':					// num to send
					ai++;
					num2send = atoi( argv[ai] );
					break;

				case 'p':					// num to send
					ai++;
					listen_port = strdup( argv[ai] );
					break;

				case 'r':					// generate random listen port
					rand_port = 0;
					;;

				case 't':					// timeout
					ai++;
					max_timeout = atoi( argv[ai] );
					break;

				case 'v':
					verbose = 1;
					break;

				default:
					fprintf( stderr, "[FAIL] unrecognised option: %s\n", argv[ai] );
					exit( 1 );
			}

			ai++;
		} else {
			break;		// not an option, leave with a1 @ first positional parm
		}
	}

	
	if( listen_port == NULL ) {
		if( rand_port ) {				// generate a somewhat random listen port (RMR needs)
			srand( time( NULL ) );
			snprintf( wbuf, sizeof( wbuf ), "%d", 43000 + (rand() % 1000) );
			listen_port = strdup( wbuf );
		} else {
			listen_port = "43086";		// -r given to disable random; go with static value
		}
	}


	VERBOSE( "listen port: %s; sending %d messages\n", listen_port, num2send );

	if( (mrc = rmr_init( listen_port, MAX_MSG_SZ, RMRFL_NOTHREAD )) == NULL ) {		// start without route table listener thread
		fprintf( stderr, "[FAIL] unable to initialise RMR\n" );
		exit( 1 );
	}
	while( ! rmr_ready( mrc ) ) {
		VERBOSE( "waiting for RMR to show ready\n" );
		sleep( 1 );
	}
	VERBOSE( "RMR initialised\n" );

	mbuf = rmr_alloc_msg( mrc, MAX_MSG_SZ );		// enough room for us, and provide app with extra room for response

	VERBOSE( "starting session with %s, starting to send\n", target );
	whid = rmr_wh_open( mrc, target );								// open a wormhole directly to the target
	if( whid < 0 ) {
		fprintf( stderr, "[FAIL] unable to connect to %s\n", target );
		exit( 1 );
	}

	VERBOSE( "connected to %s, sending %d pings\n", target, num2send );

	now = time( NULL );
	expiry =  now + max_timeout;						// when we can end this madness

	while( count < num2send && now < expiry ) {			// until we've sent and recevied n things, or we expire
		if( !mbuf ) {
			fprintf( stderr, "[FAIL] internal mishap: mbuf is nil?\n" );
			exit( 1 );
		}

		payload = mbuf->payload;
		snprintf( payload, PAYLOAD_SZ-1, "health check request prev=%d <eom>", count );

		mbuf->mtype = RIC_HEALTH_CHECK_REQ;
		mbuf->sub_id = -1;
		mbuf->len =  PAYLOAD_SZ;		// yes; we're sending more than we filled so xapp has good size for response data if needed
		mbuf->state = 0;

		VERBOSE( "sending message: %s\n", payload );
		clock_gettime( CLOCK_MONOTONIC, &out_ts );		// mark time out
		mbuf = rmr_wh_send_msg( mrc, whid, mbuf );

		if( mbuf->state == RMR_OK ) {							// good send, wait for response
			do {
				mbuf = rmr_torcv_msg( mrc, mbuf, 250 );					// wait for a response, but break often to check for timeout
				clock_gettime( CLOCK_MONOTONIC, &in_ts );				// mark time in (assuming there is a message)
				now = time( NULL );
			} while( mbuf->state == RMR_ERR_TIMEOUT && now < expiry );

			if( mbuf->state == RMR_OK ) {
				if( mbuf->mtype == RIC_HEALTH_CHECK_RESP ) {
					payload = mbuf->payload;
					memset( wbuf, 0, sizeof( wbuf ) );
					memcpy( wbuf, payload, mbuf->len > sizeof( wbuf ) ? sizeof(wbuf)-1 : mbuf->len );
					VERBOSE( "got: (%s) state=%d\n", wbuf, mbuf->state );

					if( strncmp( payload, "OK", 2 ) == 0 ) {
						good++;
						et = elapsed( &out_ts, &in_ts );
						VERBOSE( "good response received; elapsed time = %d mu-sec\n", et );
					} else {
						fprintf( stderr, "[WARN] xAPP response: %s\n", wbuf );
					}
				} else {
					fprintf( stderr, "[WARN] invalid message type received: %d\n", mbuf->mtype );
				}

				count++;
			} else {
				fprintf( stderr, "[FAIL] too few messages recevied during timeout window: wanted %d got %d\n", num2send, count );
				break;
			}
		} else {
			fprintf( stderr, "[FAIL] send failed: %d %d %s\n", mbuf->state, errno, strerror( errno ) );
			break;
		}

		now = time( NULL );
	}

	rmr_wh_close( mrc, whid );

	return good == 0;			// if none were good, then exit 1
}


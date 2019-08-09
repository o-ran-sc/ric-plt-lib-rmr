// :vim ts=4 sw=4 noet:
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
	Mnemonic:	health_check.c
	Abstract:	This is a simple programme which sends a 'health check' message to
				an application and waits for a response. By default, the application
				is assumed to be running on the local host, and listening on 4560,
				but both host and port can be configured as needed. Connection is 
				made via a wormhole, so there is no need for a routing table.

				The application being checked is expected to recognise the health
				check message type, and to return the message using the RMR return
				to sender function after changing the message type to  "health response,"
				and leaving the remainder of the payload _unchanged_.  

				A timestamp is placed into the outbound payload, and the round trip
				latency is reported (the reason the pinged application should not modify
				the payload.


				Command line options and parameters:
					[-h host:port]		target
					[-n num-msgs]		total number to send
					[-t seconds]		max timeout per message

				Route table:  While we don't need a route table to do wormhole sends we
				do need for RMR to initialise an empty one. To avoid having to have a
				dummy table on disk somewhere, we'll create one and "point" RMR at it.

	Date:		9 August 2019
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
// include message types header

#ifndef HEALTH_CHECK
#define HEALTH_CHECK	100		// message types
#define HEALTH_RESP		101
#endif

/*
	Our message payload.
*/
typedef struct mpl {
	char	msg[512];				// message for human consumption
	struct	timespec out_ts;		// time this payload was sent
} mpl_t;

// ---------------------------------------------------------------------------
/*
	Very simple checksum over a buffer.
*/
static int sum( unsigned char* buf, int len ) {
	int sum = 0;
	int	i = 0;
	unsigned char*	last;

	last = buf + len;
	while( buf < last ) {
		sum += *(buf++) + i++;
	}

	return sum % 255;
}

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

/*
	See if my id string is in the buffer immediately after the first >.
	Return 1 if so, 0 if not.
*/
static int vet_received( char* me, char* buf ) {
	char*	ch;

	if( (ch = strchr( buf, '>' )) == NULL ) {
		return 0;
	}

	return strcmp( me, ch+1 ) == 0;
}

/*
	Create an empty route table and set an environment var for RMR to find.
	This must be called before initialising RMR.
*/
static void mk_rt( ) {
	int 	fd;
	char	fnb[128];
	char*	contents = "newrt|start\nnewrt|end\n";

	snprintf( fnb, sizeof( fnb ), "/tmp/health_check.rt" );
	fd = open( fnb, O_CREAT | O_WRONLY, 0664 );
	if( fd < 0 ) {
		fprintf( stderr, "[FAIL] could not create dummy route table: %s %s\n", fnb, strerror( errno ) );
		return;
	}

	write( fd, contents, strlen( contents ) );
	if( (close( fd ) < 0 ) ) {
		fprintf( stderr, "[FAIL] couldn't close dummy route table: %s: %s\n", fnb, strerror( errno ) );
		return;
	}

	setenv( "RMR_SEED_RT", fnb, 0 );		// set it, but don't overwrite it
}

int main( int argc, char** argv ) {
	void* mrc;      						// msg router context
	rmr_mbuf_t*		mbuf;					// message buffer
	mpl_t*	payload;						// the payload in a message
	int		ai = 1;							// arg index
	long	timeout;
	long	max_timeout = 5;				// -t to overrride
	char*	target = "localhost:4560";		// address of target to ping
	char*	listen_port;					// the port we open for "backhaul" connections (random)
	char*	tok;							// pointer at token in a buffer
	int		i;
	char	wbuf[1024];
	char	me[128];						// who I am to vet rts was actually from me
	int		rand_port = 0;					// -r sets and causes us to generate a random listen port
	int		whid;							// id of wormhole
	int		num2send = 1;					// number of messages to send
	int     ep_fd = -1;						// epoll's file des (given to epoll_wait)
	int		rcv_fd;    						// file des that NNG tickles -- give this to epoll to listen on
	int		nready;							// number of events ready for receive
	int		count = 0;
	int		errors = 0;
	int		cksum;							// computed simple checksum
	struct	timespec in_ts;					// time we got response
	struct	epoll_event events[1];			// list of events to give to epoll
	struct	epoll_event epe;				// event definition for event to listen to

	// ---- simple arg parsing ------
	while( ai < argc ) {
		if( *argv[ai] == '-' ) {
			switch( argv[ai][1] ) {
				case 'h':					// host port
					ai++;
					target = strdup( argv[ai] );
					break;

				case 'n':					// num to send
					ai++;
					num2send = atoi( argv[ai] );
					break;

				case 'r':					// generate random listen port
					rand_port = 1;
					;;

				case 't':					// timeout
					ai++;
					max_timeout = atoi( argv[ai] );
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

	if( rand_port ) {
		srand( time( NULL ) );
		snprintf( wbuf, sizeof( wbuf ), "%d", 43000 + (rand() % 1000) );			// random listen port
		listen_port = strdup( wbuf );
	} else {
		listen_port = "43086";
	}


	mk_rt();				// create a dummy route table so we don't have errors/hang

	fprintf( stderr, "[INFO] listen port: %s; sending %d messages\n", listen_port, num2send );

	if( (mrc = rmr_init( listen_port, 1400, 0 )) == NULL ) {		// start without route table listener thread
		fprintf( stderr, "[FAIL] unable to initialise RMr\n" );
		exit( 1 );
	}
	fprintf( stderr, "[INFO] RMR initialised\n" );

	if( (rcv_fd = rmr_get_rcvfd( mrc )) < 0 ) {			// if we can't get an epoll FD, then we can't timeout; abort
		fprintf( stderr, "[FAIL] unable to get an epoll FD\n" );
		exit( 1 );
	}

	if( (ep_fd = epoll_create1( 0 )) < 0 ) {
		fprintf( stderr, "[FAIL] unable to create epoll fd: %d\n", errno );
		exit( 1 );
	}
	epe.events = EPOLLIN;
	epe.data.fd = rcv_fd;

	if( epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe ) != 0 )  {
		fprintf( stderr, "[FAIL] epoll_ctl status not 0 : %s\n", strerror( errno ) );
		exit( 1 );
	}

	while( ! rmr_ready( mrc ) ) {
		sleep( 1 );
	}

	mbuf = rmr_alloc_msg( mrc, sizeof( *payload ) + 100 );		// send buffer with a bit of padding

	fprintf( stderr, "[INFO] starting session with %s, starting to send\n", target );
	whid = rmr_wh_open( mrc, target );								// open a wormhole directly to the target
	if( whid < 0 ) {
		fprintf( stderr, "[FAIL] unable to connect to %s\n", target );
		exit( 2 );
	}

	fprintf( stderr, "[INFO] connected to %s, starting to send\n", target );
	rmr_set_stimeout( mrc, 3 );									// we let rmr retry failures for up to 3 "rounds"

	gethostname( wbuf, sizeof( wbuf ) );
	snprintf( me, sizeof( me ), "%s-%d", wbuf, getpid( ) );

	errors = 0;
	while( count < num2send ) {								// we send n messages after the first message is successful
		if( !mbuf ) {
			fprintf( stderr, "[FAIL] mbuf is nil?\n" );
			exit( 1 );
		}

		payload = (mpl_t *) mbuf->payload;

		snprintf( wbuf, sizeof( payload->msg ), "%s count=%d %d", me, count, rand() );
		snprintf( mbuf->payload, 1024, "%d|%s", sum( wbuf , strlen( wbuf ) ), wbuf );

		mbuf->mtype = HEALTH_CHECK;
		mbuf->sub_id = -1;
		mbuf->len =  sizeof( *payload );
		mbuf->state = 0;

		clock_gettime( CLOCK_REALTIME, &payload->out_ts );		// mark time out
		mbuf = rmr_wh_send_msg( mrc, whid, mbuf );

		if( mbuf->state == RMR_OK ) {							// good send, wait for response
			nready = epoll_wait( ep_fd, events, 1, max_timeout * 1000 );
			if( nready > 0 ) {
				clock_gettime( CLOCK_REALTIME, &in_ts );		// mark response received time

				mbuf = rmr_rcv_msg( mrc, mbuf );
				payload = (mpl_t *) mbuf->payload;
				tok = strchr( payload->msg, '|' );				// find end of chksum
				if( tok ) {
					tok++;
					cksum = sum( tok, strlen( tok ) );
					if( cksum != atoi( payload->msg ) ) {
						fprintf( stderr, "[WRN] response to msg %d received, cksum mismatch; expected %d, got %d\n", 
							count+1, atoi( payload->msg ), cksum );
					} else {
						fprintf( stderr, "[INFO] response to msg %d received, %d mu-sec\n",  count+1, elapsed( &payload->out_ts, &in_ts ) );
					}
				}
			} else {
				fprintf( stderr, "[ERR] timeout waiting for response to message %d\n", count+1 );
				errors++;
			}
		} else {
			fprintf( stderr, "[ERR] send failed: %d\n", mbuf->state );
		}

		count++;
		sleep( 1 );
	}

	rmr_wh_close( mrc, whid );

	return errors = 0;
}


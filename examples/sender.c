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
	Mnemonic:	sender.c
	Abstract:	Very simple test sender. Sends messages with a given delay between each.
				The sender also uses epoll_wait() to ensure that any received messages
				don't clog the queue.

	Parms:		The following positional parameters are recognised on the command line:
					[listen_port [delay [stats-freq] [msg-type]]]]

					Defaults:
						listen_port 43086
						delay (mu-sec) 1000000 (1 sec)
						stats-freq 10
						msg-type 0

	Date:		1 April 2019

	CAUTION:	This example is now being pulled directly into the user documentation.
				Because of this some altered line lengths and/or parameter list breaks
				which seem strage have been applied to ensure that it formats nicely.
				All code following the 'start_example' tag below is included.
*/
// start_example

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>

#include <rmr/rmr.h>

int main( int argc, char** argv ) {
	void* mrc;							// msg router context
	struct epoll_event events[1];		// list of events to give to epoll
	struct epoll_event epe;                // event definition for event to listen to
	int     ep_fd = -1;					// epoll's file des (given to epoll_wait)
	int rcv_fd;							// file des for epoll checks
	int nready;							// number of events ready for receive
	rmr_mbuf_t*		sbuf;				// send buffer
	rmr_mbuf_t*		rbuf;				// received buffer
	int	count = 0;
	int	rcvd_count = 0;
	char*	listen_port = "43086";
	int		delay = 1000000;			// mu-sec delay between messages
	int		mtype = 0;
	int		stats_freq = 100;

	if( argc > 1 ) {					// simplistic arg picking
		listen_port = argv[1];
	}
	if( argc > 2 ) {
		delay = atoi( argv[2] );
	}
	if( argc > 3 ) {
		mtype = atoi( argv[3] );
	}

	fprintf( stderr, "<DEMO> listen port: %s; mtype: %d; delay: %d\n",
		listen_port, mtype, delay );

	if( (mrc = rmr_init( listen_port, 1400, RMRFL_NONE )) == NULL ) {
		fprintf( stderr, "<DEMO> unable to initialise RMR\n" );
		exit( 1 );
	}

	rcv_fd = rmr_get_rcvfd( mrc );  // set up epoll things, start by getting the FD from RMR
	if( rcv_fd < 0 ) {
		fprintf( stderr, "<DEMO> unable to set up polling fd\n" );
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

	sbuf = rmr_alloc_msg( mrc, 256 );	// alloc 1st send buf; subsequent bufs alloc on send
	rbuf = NULL;						// don't need to alloc receive buffer

	while( ! rmr_ready( mrc ) ) {		// must have route table
		sleep( 1 );						// wait til we get one
	}
	fprintf( stderr, "<DEMO> rmr is ready\n" );


	while( 1 ) {			// send messages until the cows come home
		snprintf( sbuf->payload, 200,
			"count=%d received= %d ts=%lld %d stand up and cheer!",	// create the payload
			count, rcvd_count, (long long) time( NULL ), rand() );

		sbuf->mtype = mtype;							// fill in the message bits
		sbuf->len =  strlen( sbuf->payload ) + 1;		// send full ascii-z string
		sbuf->state = 0;
		sbuf = rmr_send_msg( mrc, sbuf );				// send & get next buf to fill in
		while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
			sbuf = rmr_send_msg( mrc, sbuf );			// w/ simple spin that doesn't give up
		}
		count++;

		// check to see if anything was received and pull all messages in
		while( (nready = epoll_wait( ep_fd, events, 1, 0 )) > 0 ) { // 0 is non-blocking
			if( events[0].data.fd == rcv_fd ) {     // waiting on 1 thing, so [0] is ok
				errno = 0;
				rbuf = rmr_rcv_msg( mrc, rbuf );	// receive and ignore; just count
				if( rbuf ) {
					rcvd_count++;
				}
			}
		}

		if( (count % stats_freq) == 0 ) {			// occasional stats out to tty
			fprintf( stderr, "<DEMO> sent %d   received %d\n", count, rcvd_count );
		}

		usleep( delay );
	}
}


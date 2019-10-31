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
	Mnemonic:	sender.c
	Abstract:	This version of the sender will perform verification on response
				messages received back from the receiver.

				It is expected that the response messages are created with the 
				functions in the test_support module so that they can easily be
				vetted here.

				This sender is designed only to test the functionality of message
				routing, specifically the ability for a receiver to reallocate a
				message to handle a larger payload without losing the ability to
				use rmr_rts_msg(); no attempt has been made to be efficent, and
				this sender should not be used for performance tests.

	Date:		28 October 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>

#include <rmr/rmr.h>


#define HDR_SIZE 64							// the size of the header we place into the message
#define MSG_SIZE 256						// toal message size sent via RMR (hdr+data)
#define DATA_SIZE (MSG_SIZE-HDR_SIZE)		// the actual 'data' length returned in the ack msg

#ifndef DEBUG
#define DEBUG 0
#endif

#include "test_support.c"

int main( int argc, char** argv ) {
	void* mrc;      						// msg router context
	struct	epoll_event events[1];			// list of events to give to epoll
	struct	epoll_event epe;				// event definition for event to listen to
	int     ep_fd = -1;						// epoll's file des (given to epoll_wait)
	int		rcv_fd;    						// file des that NNG tickles -- give this to epoll to listen on
	int		nready;							// number of events ready for receive
	rmr_mbuf_t*		sbuf;					// send buffer
	rmr_mbuf_t*		rbuf;					// received buffer
	char*	ch;
	int		count = 0;
	int		short_count = 0;				// number of acks that didn't seem to have a bigger payload
	int		rt_count = 0;					// number of messages requiring a spin retry
	int		rcvd_count = 0;
	int		rts_ok = 0;						// number received with our tag
	int		fail_count = 0;					// # of failure sends after first successful send
	char*	listen_port = "43086";
	int		mtype = 0;
	int		stats_freq = 100;
	int		successful = 0;					// set to true after we have a successful send
	char	wbuf[DATA_SIZE];
	char	me[128];						// who I am to vet rts was actually from me
	char	trace[1024];
	long	timeout = 0;
	long	rep_timeout = 0;				// report/stats timeout
	int		delay = 100000;					// usec between send attempts
	int		nmsgs = 10;						// number of messages to send
	int		max_mt = 10;					// reset point for message type
	int		start_mt = 0;
	int		pass = 1;
	int		need;

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		delay = atoi( argv[2] );
	}
	if( argc > 3 ) {
		if( (ch = strchr( argv[3], ':' )) != NULL ) {
			max_mt = atoi( ch+1 );
			start_mt = atoi( argv[3] );
		} else {
			max_mt = atoi( argv[3] );
		}
	}
	if( argc > 4 ) {
		listen_port = argv[4];
	}

	mtype = start_mt;

	fprintf( stderr, "<VSNDR> listen port: %s; sending %d messages; delay=%d\n", listen_port, nmsgs, delay );

	if( (mrc = rmr_init( listen_port, 1400, RMRFL_NONE )) == NULL ) {
		fprintf( stderr, "<VSNDR> unable to initialise RMr\n" );
		exit( 1 );
	}

	if( (rcv_fd = rmr_get_rcvfd( mrc )) >= 0 ) {			// epoll only available from NNG -- skip receive later if not NNG
		if( rcv_fd < 0 ) {
			fprintf( stderr, "<VSNDR> unable to set up polling fd\n" );
			exit( 1 );
		}
		if( (ep_fd = epoll_create1( 0 )) < 0 ) {
			fprintf( stderr, "<VSNDR> [FAIL] unable to create epoll fd: %d\n", errno );
			exit( 1 );
		}
		epe.events = EPOLLIN;
		epe.data.fd = rcv_fd;

		if( epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe ) != 0 )  {
			fprintf( stderr, "<VSNDR> [FAIL] epoll_ctl status not 0 : %s\n", strerror( errno ) );
			exit( 1 );
		}
	} else {
		fprintf( stderr, "<VSNDR> abort: epoll not supported, can't listen for messages\n" );	
	}

	sbuf = rmr_alloc_msg( mrc, MSG_SIZE );						// alloc first send buffer; subsequent buffers allcoated on send
	rbuf = NULL;												// don't need to alloc receive buffer

	timeout = time( NULL ) + 20;		// give rmr 20s to find the route table (shouldn't need that much)
	while( ! rmr_ready( mrc ) ) {		// must have a route table before we can send; wait til RMr says it has one
		fprintf( stderr, "<VSNDR> waiting for rmr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<VSNDR> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<VSNDR> rmr is ready; starting to send\n" );

	gethostname( wbuf, sizeof( wbuf ) );
	snprintf( me, sizeof( me ), "%s-%d", wbuf, getpid( ) );

	while( count < nmsgs ) {										// we send n messages after the first message is successful
		snprintf( wbuf, DATA_SIZE, "Don't shoot the messaging library if you don't like what was in the payload (%d)", rand() );	// add random to change chksum
		need = generate_header( sbuf->payload, wbuf, 0, 0 );		// generate the header directly into the message payload
		if( need > MSG_SIZE ) 			{							// we have a static size for sending; abort if it would be busted
			fprintf( stderr, "[CRIT] abort: need for send payload size is > than allocated: %d\n", need );
			exit( 1 );
		}

		memcpy( sbuf->payload + HDR_SIZE, wbuf, DATA_SIZE );		// copy in our data (probably not the full amount of bytes
		sbuf->mtype = mtype;										// fill in the message metadata
		if( mtype < 3 ) {
			sbuf->sub_id = mtype * 10;
		} else {
			sbuf->sub_id = -1;
		}

		sbuf->len =  need;
		sbuf->state = 0;
		sbuf = rmr_send_msg( mrc, sbuf );				// send it (send returns an empty payload on success, or the original payload on fail/retry)

		switch( sbuf->state ) {
			case RMR_ERR_RETRY:
				rt_count++;
				while( time( NULL ) < timeout && sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
					sbuf = rmr_send_msg( mrc, sbuf );			// retry send until it's good (simple test; real programmes should do better)
				}
				if( sbuf->state == RMR_OK ) {
					successful = 1; 							// indicates only that we sent one successful message, not the current state
				} else {
					if( successful ) {
						fail_count++;							// count failures after first successful message
					}
					if( fail_count > 10 ) {
						fprintf( stderr, "too many failures\n" );
						exit( 1 );
					}
				}
				break;

			case RMR_OK:
				successful = 1;
				break;

			default:
				if( successful ) {
					fail_count++;							// count failures after first successful message
				}
				// some error (not connected likely), don't count this
				//sleep( 1 );
				break;
		}

		if( successful ) {				// once we have a message that was sent, start to increase things
			count++;
			mtype++;
			if( mtype >= max_mt ) {			// if large number of sends don't require infinite rt entries :)
				mtype = start_mt;
			}
		}

		if( rcv_fd >= 0 ) {
			while( (nready = epoll_wait( ep_fd, events, 1, 0 )) > 0 ) {			// if something ready to receive (non-blocking check)
				if( events[0].data.fd == rcv_fd ) {             				// we only are waiting on 1 thing, so [0] is ok
					errno = 0;
					rbuf = rmr_rcv_msg( mrc, rbuf );
					if( rbuf && rbuf->state == RMR_OK ) {
						if( rmr_payload_size( rbuf ) > HDR_SIZE+DATA_SIZE ) {		// verify that response has a larger payload than we should have sent
							rts_ok += validate_msg( rbuf->payload, rbuf->len );
						} else { 
							short_count++;
						}
						rcvd_count++;
					}
				}
			}
		}

		if( time( NULL ) > rep_timeout ) {
			fprintf( stderr, "<VSNDR> sent=%d  rcvd=%d  ok_acks=%d short_acks=%d send_fails=%d retries=%d\n", count, rcvd_count, rts_ok, short_count, fail_count, rt_count );

			rep_timeout = time( NULL ) + 5;
		}

		if( delay > 0 ) {
			usleep( delay );
		}
	}

	timeout = time( NULL ) + 2;				// allow 2 seconds for the pipe to drain from the receiver
	while( time( NULL ) < timeout ) {
		if( rcv_fd >= 0 ) {
			while( (nready = epoll_wait( ep_fd, events, 1, 100 )) > 0 ) {
				if( events[0].data.fd == rcv_fd ) {             				// we only are waiting on 1 thing, so [0] is ok
					errno = 0;
					rbuf = rmr_rcv_msg( mrc, rbuf );
					if( rbuf && rbuf->state == RMR_OK ) {
						rcvd_count++;
						if( rmr_payload_size( rbuf ) > HDR_SIZE+DATA_SIZE ) {		// verify that response has a larger payload than we should have sent
							rts_ok += validate_msg( rbuf->payload, rbuf->len );
						}

						timeout = time( NULL ) + 2;
					}
				}
			}
		}
	}

	if( rcvd_count != rts_ok || count != nmsgs ) {			// we might not receive all back if receiver didn't retry, so that is NOT a failure here
		pass = 0;
	}

	fprintf( stderr, "<VSNDR> [%s] sent=%d  rcvd=%d  rts-ok=%d failures=%d retries=%d\n", pass ? "PASS" : "FAIL",  count, rcvd_count, rts_ok, fail_count, rt_count );
	rmr_close( mrc );

	return !pass;
}


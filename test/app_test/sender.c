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
	Abstract:	This is a simple sender which will send a series of messages. 
				It is expected that the first attempt(s) will fail if the receiver
				is not up and this does not start decrementing the number to
				send until it has a good send.  

				The process will check the receive queue and list received messages
				but pass/fail is not dependent on what comes back.

				If the receiver(s) do not become connectable in 20 sec this process
				will give up and fail. 


				Message types will vary between 1 and 10, so the route table must
				be set up to support those message types.  

				Message format is:
					ck1 ck2|<msg-txt><nil>

				Ck1 is the simple check sum of the msg-text (NOT includeing <nil>)
				Ck2 is the simple check sum of the trace data which is a nil terminated
				series of bytes. 

				Parms:	argv[1] == nmsgs; argv[2] == delay; argv[3] == listen port

				Sender will send for at most 20 seconds, so if nmsgs and delay extend
				beyond that period the total number of messages sent will be less
				than n.

	Date:		18 April 2019
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

static int sum( char* str ) {
	int sum = 0;
	int	i = 0;

	while( *str ) {
		sum += *(str++) + i++;
	}

	return sum % 255;
}

int main( int argc, char** argv ) {
    void* mrc;      						// msg router context
    struct epoll_event events[1];			// list of events to give to epoll
    struct epoll_event epe;                 // event definition for event to listen to
    int     ep_fd = -1;						// epoll's file des (given to epoll_wait)
    int rcv_fd;     						// file des that NNG tickles -- give this to epoll to listen on
	int nready;								// number of events ready for receive
	rmr_mbuf_t*		sbuf;					// send buffer
	rmr_mbuf_t*		rbuf;					// received buffer
	int	count = 0;
	int	rt_count = 0;						// number of messages requiring a spin retry
	int	rcvd_count = 0;
	char*	listen_port = "43086";
	int		mtype = 0;
	int		stats_freq = 100;
	int		successful = 0;					// set to true after we have a successful send
	char	wbuf[1024];
	char	trace[1024];
	long	timeout = 0;
	int		delay = 100000;					// usec between send attempts
	int		nmsgs = 10;						// number of messages to send

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		delay = atoi( argv[2] );
	}
	if( argc > 3 ) {
		listen_port = argv[3];
	}

	fprintf( stderr, "<SNDR> listen port: %s; sending %d messages; delay=%d\n", listen_port, nmsgs, delay );

    if( (mrc = rmr_init( listen_port, 1400, RMRFL_NONE )) == NULL ) {
		fprintf( stderr, "<SNDR> unable to initialise RMr\n" );
		exit( 1 );
	}

    if( (rcv_fd = rmr_get_rcvfd( mrc )) >= 0 ) {			// epoll only available from NNG -- skip receive later if not NNG
		if( rcv_fd < 0 ) {
			fprintf( stderr, "<SNDR> unable to set up polling fd\n" );
			exit( 1 );
		}
		if( (ep_fd = epoll_create1( 0 )) < 0 ) {
			fprintf( stderr, "<SNDR> [FAIL] unable to create epoll fd: %d\n", errno );
			exit( 1 );
		}
    	epe.events = EPOLLIN;
    	epe.data.fd = rcv_fd;

    	if( epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe ) != 0 )  {
			fprintf( stderr, "<SNDR> [FAIL] epoll_ctl status not 0 : %s\n", strerror( errno ) );
			exit( 1 );
		}
	} else {
		rmr_set_rtimeout( mrc, 0 );			// for nano we must set the receive timeout to 0; non-blocking receive
	}

	sbuf = rmr_alloc_msg( mrc, 512 );	// alloc first send buffer; subsequent buffers allcoated on send
	//sbuf = rmr_tralloc_msg( mrc, 512, 11, "xxxxxxxxxx" );	// alloc first send buffer; subsequent buffers allcoated on send
	rbuf = NULL;						// don't need to alloc receive buffer

	timeout = time( NULL ) + 20;		// give rmr 20s to find the route table (shouldn't need that much)
	while( ! rmr_ready( mrc ) ) {		// must have a route table before we can send; wait til RMr says it has one
		fprintf( stderr, "<SNDR> waiting for rmr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<SNDR> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<SNDR> rmr is ready; starting to send\n" );
	
	timeout = time( NULL ) + 20;

    while( count < nmsgs ) {								// we send 10 messages after the first message is successful
		snprintf( trace, 100, "%lld", (long long) time( NULL ) );
		rmr_set_trace( sbuf, trace, strlen( trace ) + 1 );
		snprintf( wbuf, 200, "count=%d tr=%s %d stand up and cheer!", count, trace, rand() );
		snprintf( sbuf->payload, 300, "%d %d|%s", sum( wbuf ), sum( trace ), wbuf );

		sbuf->mtype = mtype;							// fill in the message bits
		sbuf->len =  strlen( sbuf->payload ) + 1;		// our receiver likely wants a nice acsii-z string
		sbuf->state = 0;
		sbuf = rmr_send_msg( mrc, sbuf );				// send it (send returns an empty payload on success, or the original payload on fail/retry)

		switch( sbuf->state ) {
			case RMR_ERR_RETRY:
				rt_count++;
				while( sbuf->state == RMR_ERR_RETRY ) {			// soft failure (device busy?) retry
					sbuf = rmr_send_msg( mrc, sbuf );			// retry send until it's good (simple test; real programmes should do better)
				}
				successful = 1;
				break;

			case RMR_OK:
				successful = 1;
				break;

			default:
				// some error (not connected likely), don't count this
				break;
		}

		if( successful ) {				// once we have a message that was sent, start to increase things
			count++;
			mtype++;
			if( mtype > 10 ) {			// if large number of sends don't require infinite rt entries :)
				mtype = 1;
			}
		}

		if( rcv_fd >= 0 ) {
			while( (nready = epoll_wait( ep_fd, events, 1, 0 )) > 0 ) {			// if something ready to receive (non-blocking check)
				if( events[0].data.fd == rcv_fd ) {             				// we only are waiting on 1 thing, so [0] is ok
					errno = 0;
					rbuf = rmr_rcv_msg( mrc, rbuf );
					if( rbuf ) {
						rcvd_count++;
					}
				}
			}
		} else {				// nano, we will only pick up one at a time.
			if(	(rbuf = rmr_rcv_msg( mrc, rbuf ) ) != NULL ) {
				if( rbuf->state == RMR_OK ) {
					rcvd_count++;
				}
			}
		}

		if( time( NULL ) > timeout ) {						// should only happen if we never connect or nmsg > what we can send in 20sec
			fprintf( stderr, "sender timeout\n" );
			break;
		}

		if( delay > 0 ) {
			usleep( delay );
		}
    }

	
	timeout = time( NULL ) + 2;				// allow 2 seconds for the pipe to drain from the receiver
	while( time( NULL ) < timeout );
		if( rcv_fd >= 0 ) {
			while( (nready = epoll_wait( ep_fd, events, 1, 100 )) > 0 ) {			// if something ready to receive (non-blocking check)
				if( events[0].data.fd == rcv_fd ) {             				// we only are waiting on 1 thing, so [0] is ok
					errno = 0;
					rbuf = rmr_rcv_msg( mrc, rbuf );
					if( rbuf ) {
						rcvd_count++;
						timeout = time( NULL ) + 2;
					}
				}
			}
		} else {				// nano, we will only pick up one at a time.
			if(	(rbuf = rmr_torcv_msg( mrc, rbuf, 100 ) ) != NULL ) {
				if( rbuf->state == RMR_OK ) {
					rcvd_count++;
				}
			}
		}

	fprintf( stderr, "<SNDR> [%s] sent %d messages   received %d acks retries=%d\n", count == nmsgs ? "PASS" : "FAIL",  count, rcvd_count, rt_count );
	rmr_close( mrc );

	return !( count == nmsgs );
}


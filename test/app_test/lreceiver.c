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
	Mnemonic:	rmr_rcvr.c
	Abstract:	This is a very simple receiver that listens for messages and
				returns each to the sender after adding a timestamp to the 
				payload.  The payload is expected to be lc_msg_t (see lcaller.c)
				and this will update the 'turn' timestamp on receipt.

				Define these environment variables to have some control:
					RMR_SEED_RT -- path to the static routing table
					RMR_RTG_SVC -- port to listen for RTG connections

	Date:		18 April 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <rmr/rmr.h>

/*
	The message type placed into the payload.
*/
typedef struct lc_msg {
	struct timespec out_ts;			// time just before call executed
	struct timespec turn_ts;		// time at the receiver,  on receipt
	struct timespec in_ts;			// time received back by the caller
	int		out_retries;			// number of retries required to send
	int		turn_retries;			// number of retries required to send
} lc_msg_t;

// ----------------------------------------------------------------------------------

static int sum( char* str ) {
	int sum = 0;
	int	i = 0;

	while( *str ) {
		sum += *(str++) + i++;
	}

	return sum % 255;
}

/*
	Split the message at the first sep and return a pointer to the first
	character after.
*/
static char* split( char* str, char sep ) {
	char*	s;

	s = strchr( str, sep );

	if( s ) {
		return s+1;
	}

	fprintf( stderr, "<RCVR> no pipe in message: (%s)\n", str );
	return NULL;
}

int main( int argc, char** argv ) {
	void* mrc;      					// msg router context
	lc_msg_t*	lmc;					// latency message type from caller
	rmr_mbuf_t* msg = NULL;				// message received
	int		i;
	int		errors = 0;
	char*	listen_port = "4560";
	long	count = 0;						// total received
	long	timeout = 0;
	char*	data;
	int		nmsgs = 10;					// number of messages to stop after (argv[1] overrides)
	int		rt_count = 0;				// retry count
	time_t	now;
	int		active;

	data = getenv( "RMR_RTG_SVC" );
	if( data == NULL ) {
		setenv( "RMR_RTG_SVC", "19289", 1 );		// set one that won't collide with the sender if on same host
	}

	if( argc > 1 ) {
		nmsgs = atoi( argv[1] );
	}
	if( argc > 2 ) {
		listen_port = argv[2];
	}


	fprintf( stderr, "<RCVR> listening on port: %s for a max of %d messages\n", listen_port, nmsgs );

	mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_MTCALL );	// start your engines!
	//mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, 0 );	// start your engines!
	if( mrc == NULL ) {
		fprintf( stderr, "<RCVR> ABORT:  unable to initialise RMr\n" );
		exit( 1 );
	}

	timeout = time( NULL ) + 20;
	while( ! rmr_ready( mrc ) ) {								// wait for RMr to load a route table
		fprintf( stderr, "<RCVR> waiting for RMr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<RCVR> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<RCVR> rmr now shows ready, listening begins\n" );

	timeout = time( NULL ) + 2;			// once we start, we assume if we go 2s w/o a message that we're done
	//while( count < nmsgs ) {
	while( 1 ) {
		active = 0;
		msg = rmr_torcv_msg( mrc, msg, 1000 );				// pop every second or so to timeout if needed

		if( msg ) {
			active = 1;
			if( msg->state == RMR_OK ) {
				lmc = (lc_msg_t *) msg->payload;
				clock_gettime( CLOCK_REALTIME, &lmc->turn_ts );		// mark time that we received it.
				count++;
				
				msg = rmr_rts_msg( mrc, msg );
				rt_count = 1000;
				while( rt_count > 0 && msg != NULL && msg->state == RMR_ERR_RETRY ) {		// to work right in nano we need this :(
					lmc->turn_retries++;
					if( count < 1 ) {										// 1st msg, so we need to connect, and we'll wait for that
						sleep( 1 );
					}
					rt_count--;
					msg = rmr_rts_msg( mrc, msg );							// we don't try to resend if this returns retry
				}
			}
		}

		now = time( NULL );
		if( now > timeout ) {
			break;
		}

		if( active ) {
			timeout = now + 2;
		}
	}

	fprintf( stderr, "<RCVR> %ld is finished got %ld messages\n", (long) getpid(), count );

	
	sleep( 3 );
	rmr_close( mrc );
	return 0;
}


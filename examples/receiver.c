// :vim ts=4 sw=4 noet:
/*
	Mnemonic:	rmr_rcvr.c
	Abstract:	This is a very simple receiver that does nothing but listen
				for messages and write stats every so often to the tty.

				Define these environment variables to have some control:
					RMR_SEED_RT -- path to the static routing table
					RMR_RTG_SVC -- host:port of the route table generator

	Parms:		Two positional parameters are recognised on the command line:
					[port [stats-freq]]

				where port is the port number to listen on and the stats frequency
				is the number of messages received which causes stats to be 
				generated.  If not supplied the listen port default is 4560
				and the stats frequency is every 10 messages.

	Date:		1 April 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <rmr/rmr.h>


int main( int argc, char** argv ) {
    void* mrc;      					// msg router context
	long long total = 0;
    rmr_mbuf_t* msg = NULL;				// message received
	int stat_freq = 10;				// write stats after reciving this many messages
	int i;
	char*	listen_port;
	long long count = 0;
	long long bad = 0;
	long long empty = 0;

	if( argc > 1 ) {
		listen_port = argv[1];
	}
	if( argc > 2 ) {
		stat_freq = atoi( argv[2] );
	}
	fprintf( stderr, "<DEMO> listening on port: %s\n", listen_port );
	fprintf( stderr, "<DEMO> stats will be reported every %d messages\n", stat_freq );

    mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_NONE );	// start your engines!
	if( mrc == NULL ) {
		fprintf( stderr, "<DEMO> ABORT:  unable to initialise RMr\n" );
		exit( 1 );
	}

	while( ! rmr_ready( mrc ) ) {								// wait for RMr to load a route table
		fprintf( stderr, "<DEMO> waiting for ready\n" );
		sleep( 3 );
	}
	fprintf( stderr, "<DEMO> rmr now shows ready\n" );

    while( 1 ) {											// forever; ctl-c, kill -15, etc to end
		msg = rmr_rcv_msg( mrc, msg );						// block until one arrives
		
		if( msg ) {
			if( msg->state == RMR_OK ) {
				count++;									// messages received for stats output
			} else {
				bad++;
			}
		} else {
			empty++;
		}

		if( (count % stat_freq) == 0  ) {
			fprintf( stderr, "<DEMO> total msg received: %lld  errors: %lld   empty: %lld\n", count, bad, empty );
		}

    }
}


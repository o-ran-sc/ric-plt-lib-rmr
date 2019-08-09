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
	Mnemonic:	msg_echo.c
	Abstract:	This is a simple message receiver which will echo the received
				message back to the sender using an RMR return to sender call.
				All of the message will be left unchanged, though the message type
				may be changed by supplying it on the command line as the first 
				positional parameter.

				Because this process uses the rts call in RMR, it does not need
				a route table. However, RMR needs to have at least an empty table
				in order to work properly. To avoid having the user make a dummy
				table, we will create an empty one in /tmp and set the needed 
				environment var so the RMR initialisation process finds it.

	Date:		9 August 2019
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

#include <rmr/rmr.h>

/*
	Create an empty route table and set an environment var for RMR to find.
	This must be called before initialising RMR.
*/
static void mk_rt( ) {
	int 	fd;
	char	fnb[128];
	char*	contents = "newrt|start\nnewrt|end\n";

	snprintf( fnb, sizeof( fnb ), "/tmp/msg_echo.rt" );
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
	void* mrc;      					// msg router context
	rmr_mbuf_t* msg = NULL;				// message received
	int i;
	int		state;
	int		errors = 0;
	char*	listen_port = "4560";
	long timeout = 0;
	char*	data;						// pointer at env data we sussed out
	char	wbuf[1024];					// we'll pull trace data into here, and use as general working buffer
	char	sbuf[128];					// short buffer
	int		mtype = -1;					// if set on command line, we'll add to msg before rts
	int		ai = 1;						// argument index

	data = getenv( "RMR_RTG_SVC" );
	if( data == NULL ) {
		setenv( "RMR_RTG_SVC", "19289", 1 );		// set one that won't collide with the sender if on same host
	}

	// ---- simple arg parsing ------
	while( ai < argc ) {
		if( *argv[ai] == '-' ) {
			switch( argv[ai][1] ) {
				case 'p':					// timeout
					ai++;
					listen_port = argv[ai];
					break;

				case 't':					// rts message type
					ai++;
					mtype = atoi( argv[ai] );
					break;

				default:
					fprintf( stderr, "[FAIL] unrecognised option: %s\n", argv[ai] );
					fprintf( stderr, "\nusage: %s [-p port] [-t msg-type]\n", argv[0] );
					exit( 1 );
			}

			ai++;
		} else {
			break;		// not an option, leave with a1 @ first positional parm
		}
	}

	fprintf( stderr, "<ECHO> listening on port: %s will return messages with type: %d\n", listen_port, mtype );
	
	mk_rt();							// make an empty rt

	mrc = rmr_init( listen_port, RMR_MAX_RCV_BYTES, RMRFL_NONE );	// start your engines!
	if( mrc == NULL ) {
		fprintf( stderr, "<ECHO> ABORT:  unable to initialise RMr\n" );
		exit( 1 );
	}

	timeout = time( NULL ) + 20;
	while( ! rmr_ready( mrc ) ) {								// wait for RMr to configure the route table
		fprintf( stderr, "<ECHO> waiting for RMr to show ready\n" );
		sleep( 1 );

		if( time( NULL ) > timeout ) {
			fprintf( stderr, "<ECHO> giving up\n" );
			exit( 1 );
		}
	}
	fprintf( stderr, "<ECHO> rmr now shows ready, listening begins\n" );

	while( 1 ) {							// listen until the cows come home, pigs fly...
		msg = rmr_rcv_msg( mrc, msg );

		if( msg && msg->state == RMR_OK ) {
			if( mtype >= 0 ) {
				msg->mtype = mtype;
				msg->sub_id = RMR_VOID_SUBID;
			}

			msg = rmr_rts_msg( mrc, msg );
		}
	}

	return  0;		// unreachable, but some compilers swak if this isn't here.
}


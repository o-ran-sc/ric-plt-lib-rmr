/*
==================================================================================
	Copyright (c) 2020 Nokia
	Copyright (c) 2020 AT&T Intellectual Property.

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
	Mnemonic:	test_common_em.c
	Abstract:	This supplies some dummy emulation functions that are common
				to any of the emulation code. This file should be inlucded
				by the emulation code, and not the test programme.

				This also includes dummy struct defs so that we can simulate
				returning messages and message buffers and some control
				functions which can be used to taylor the emulation behavour
				during the test.

	Date:		21 February 2020
	Author:		E. Scott Daniels
*/

#ifndef _em_common_c
#define _em_common_c


static int em_send_failures = 0;	// test programme can set this to emulate eagain send failures
static int em_timeout = -1;			// set by set socket option
static int em_mtc_msgs = 0;			// set to generate 'received' messages with mt-call header data
static int return_value = 0;		// functions should return this value
static int rcv_count = 0;			// receive counter for transaction id to allow test to rest
static int rcv_delay = 0;			// forced delay before call to rcvmsg starts to work

static int gates_ok = 0;
static pthread_mutex_t rcv_gate;
static int em_gen_long_hostname = 0;		// if set the emulated hostname generates a longer name (>40 char)

/*
	Simulated v1 message for receive to return. This needs to match the RMr header
	so that we can fill in length, type and xaction id things.
#define MSG_VER 1
struct em_msg {
	int32_t	mtype;						// message type  ("long" network integer)
	int32_t	plen;						// payload length
	int32_t rmr_ver;					// our internal message version number
	unsigned char xid[32];				// space for user transaction id or somesuch
	unsigned char sid[32];				// sender ID for return to sender needs
	unsigned char src[16];				// name of the sender (source)
	unsigned char meid[32];				// managed element id.
	struct timespec	ts;					// timestamp ???
};
*/

/*
	v2 message; should be able to use it for everything that is set up here as
	we don't add a payload even if setting a v1 type.
*/
#define ALT_MSG_VER 1	// alternate every so often
#define MSG_VER 3		// default version to insert
struct em_msg {
	int32_t	mtype;						// message type  ("long" network integer)
	int32_t	plen;						// payload length
	int32_t rmr_ver;					// our internal message version number
	unsigned char xid[32];				// space for user transaction id or somesuch
	unsigned char sid[32];				// sender ID for return to sender needs
	unsigned char src[64];				// name of the sender (source)
	unsigned char meid[32];				// managed element id.
	struct timespec	ts;					// timestamp ???

	                                    // V2 extension
	int32_t flags;                      // HFL_* constants
	int32_t len0;                       // length of the RMr header data
	int32_t len1;                       // length of the tracing data
	int32_t len2;                       // length of data 1 (d1)
	int32_t len3;                       // length of data 2 (d2)
	int32_t	sub_id;						// subscription id (-1 invalid)

										// V3 stuff
	unsigned char srcip[64];			// sender ID for return to sender needs
};

// --  emulation control functions ------------------------------------------------------

/*
	Test app can call this to have all emulated functions return failure instead
	of success.
*/
static void en_set_return( int rv ) {
	return_value = rv;
}

static int em_nng_foo() {
	fprintf( stderr, "emulated functions in play" );
}

/*
	Turns on/off the generation of multi-threaded call messages
*/
static int em_set_mtc_msgs( int state ) {
	em_mtc_msgs = state;
}

/*
	Returns the size of the header we inserted
*/
static int em_hdr_size() {
	if( em_mtc_msgs ) {
		return (int) sizeof( struct em_msg ) + 4;
	}

	return (int) sizeof( struct em_msg );
}

static void em_set_rcvcount( int v ) {
	rcv_count = v;
}

static void em_set_rcvdelay( int v ) {
	if( v < 0 ) {
		fprintf( stderr, "<EM>   ##ERR## attempt to set receive delay with invalid value was ignored: %d seconds\n", v );
		return;
	}
	fprintf( stderr, "<EM>   receive delay is now %d seconds\n", v );
	rcv_delay = v;
}

static void em_start() {
	if( ! gates_ok ) {
		pthread_mutex_init( &rcv_gate, NULL );
		gates_ok = 1;
	}
}


// ----------- gethostname emulation ---------------------------------------
#define gethostname  em_gethostname
static int em_gethostname( char* buf, size_t len ) {
	if( len < 1 ) {
		errno = EINVAL;
		return 1;
	}

	if( em_gen_long_hostname ) {
		snprintf( buf, len, "hostname-which-is-long-a860430b890219-dfw82" );
	} else {
		snprintf( buf, len, "em-hostname" );
	}

	return 0;
}

static int em_set_long_hostname( int v ) {
	em_gen_long_hostname = !!v;
}


// ----------- epoll emulation ---------------------------------------------

// CAUTION: sys/epoll.h must be included before these define for functions 
//			to properly compile.
//
#include <sys/epoll.h>
#define epoll_wait em_wait
#define epoll_ctl  em_ep_ctl
#define epoll_create  em_ep_create

/*
	Every other call returns 1 ready; alternate calls return 0 ready.
	Mostly for testing the timeout receive call. First call should return
	something ready and the second should return nothing ready so we can
	drive both cases.
*/
static int em_wait( int fd, void* events, int n, int to ) {
	static int ready = 0;

	ready = !ready;
	return ready;
}

static int em_ep_ctl( int epfd, int op, int fd, struct epoll_event *event ) {
	return 0;
}

static int em_ep_create( int size ) {
	return 0;
}



#endif

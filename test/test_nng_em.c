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
	Mnemonic:	test_nng_em.c
	Abstract:	A nano/NNG message emulator for testing without needing to
				actually have nanomsg, nng, or external processes.
				We also emulate the epoll_wait() function for controlled
				poll related testing.

				This module must be directly included to be used.
	Date:		11 February 2019
	Author:		E. Scott Daniels
*/

// ---------------------- emulated nng functions ---------------------------


#ifndef _em_nn
#define _em_nn

static int em_send_failures = 0;	// test programme can set this to emulate eagain send failures

// ----------- epoll emulation ---------------------------------------------

// CAUTION: sys/epoll.h must be included before this define and function will properly compile.
#define epoll_wait em_wait
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



//--------------------------------------------------------------------------
#ifdef EMULATE_NNG
struct nn_msghdr {
	int boo;
};

static int return_value = 0;

/*
	Test app can call this to have all emulated functions return failure instead
	of success.
*/
static void en_set_retur( int rv ) {
	return_value = rv;
}



static int em_nng_foo() {
	fprintf( stderr, "emulated functions in play" );
}


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
#define MSG_VER 2		// default version to insert
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

};

/*
	Receive message must allocate a new buffer and return the pointer into *m.
	Every 9 messages or so we'll simulate an old version message
*/
static int em_nng_recvmsg( nng_socket s, nng_msg ** m, int i ) {
	void* b;
	struct em_msg* msg;
	static int count = 0;			// we'll simulate a message going in by dropping an rmr-ish msg with transaction id only
	int trace_size = 0;

	//sleep( 1 );

	b = (void *) malloc( 2048 );
	if( m != NULL ) {
		memset( b, 0, 2048 );
		*m = (nng_msg *) b;
		msg = (struct em_msg *) b;
		if( count % 10  == 9 ) {
			//msg->rmr_ver = htonl( MSG_VER );
			msg->rmr_ver = ALT_MSG_VER;		// emulate the bug in RMr v1
		} else {
			msg->rmr_ver = htonl( MSG_VER );
		}
		msg->mtype = htonl( 1 );
		msg->plen = htonl( 129 );
		msg->len0 = htonl( sizeof( struct em_msg ) );
		msg->len1 = htonl( trace_size );
		snprintf( msg->xid, 32, "%015d", count++ );		// simple transaction id so we can test receive specific and ring stuff
		snprintf( msg->src, 16, "localhost:4562" );		// set src id (unrealistic) so that rts() can be tested
	}

	//fprintf( stderr, ">>> simulated received message: %s\n", msg->xid );
	return return_value;
}

static void* em_msg_body( nng_msg* msg ) {
	return (void *) msg;								// we don't manage a real msg, so body is just the buffer we allocated
}

static size_t em_msg_len( const nng_msg* msg ) {
	if( msg ) {
		return  2048;
	}

	return 0;
}


static int em_nng_pull_open(nng_socket * s ) {
	return return_value;
}
static int em_nng_pull0_open(nng_socket * s ) {
	return return_value;
}
static int em_nng_listen(nng_socket s, const char * c, nng_listener * l, int i ) {
	return return_value;
}
static int em_nng_close(nng_socket s ) {
	return return_value;
}
static int em_nng_push0_open(nng_socket * s ) {
	return return_value;
}
static int em_nng_dial(nng_socket s, const char * c, nng_dialer * d, int i ) {
	//fprintf( stderr, "<info> === simulated dialing: %s\n", c );
	return return_value;
}
static int em_nng_setopt(nng_socket s, const char * c, const void * p, size_t t ) {
	return return_value;
}
static int em_nng_sub_open(nng_socket * s ) {
	return return_value;
}
static int em_nng_sub0_open(nng_socket * s ) {
	return return_value;
}
static int em_nng_recv(nng_socket s, void * v, size_t * t, int i ) {
	return return_value;
}
static int em_nng_send( nng_socket s, void* m, int l, int f ) {
	return return_value;
}

/*
	Emulate sending a message. If the global em_send_failures is set,
	then every so often we fail with an EAGAIN to drive that part
	of the code in RMr.
*/
static int em_sendmsg( nng_socket s, nng_msg* m, int i ) {
	static int count = 0;

	if( em_send_failures && (count++ % 15 == 14) ) {
		//fprintf( stderr, ">>>> failing send\n\n" );
		return NNG_EAGAIN;
	}

	return return_value;
}

static void* em_nng_alloc( size_t len ) {
	return malloc( len );
}

static int em_nng_msg_alloc( nng_msg** mp, size_t l ) {
	void*	p;

	if( !mp || return_value != 0  ) {
		return -1;
	}

	p = (void *) malloc( sizeof( char ) * l );
	*mp = (nng_msg *) p;

	return return_value;
}

/*
	We just free the buffer here as it was a simple malloc.
*/
static void em_nng_free( void* p, size_t l ) {
	if( p ) {
		//fprintf( stderr, ">>>>> not freed: %p\n", p );
		free( p );
	}
}
static void em_nng_msg_free( void* p ) {
	if( p ) {
		//fprintf( stderr, ">>>>> not freed: %p\n", p );
		free( p );
	}
}

static int em_dialer_create( void* d, nng_socket s, char* stuff ) {
	//fprintf( stderr, ">>>> emulated dialer create\n\n" );
	return 0;
}

static int em_dialer_start( nng_dialer d, int i ) {
	//fprintf( stderr, ">>>> emulated dialer start\n\n" );
	return return_value;
}


static int em_dialer_setopt_ms( nng_dialer dialer, void* option, int ms ) {
	return return_value;
}

static int em_nng_getopt_int( nng_socket s, void* con, int* target ) {
	if( target ) {
		*target = 0;
	}
	return return_value;
}



// nng redefines some of these to point directly to various 'versions' of the function (ugg, function versions, really?)
#undef nng_recvmsg
#undef nng_free
#undef nng_pull_open
#undef nng_pull0_open
#undef nng_listen
#undef nng_close
#undef nng_getopt_int
#undef nng_push0_open
#undef nng_dial
#undef nng_setopt
#undef nng_sub_open
#undef nng_sub0_open
#undef nng_recv
#undef nng_alloc

#define nng_msg_alloc em_nng_msg_alloc
#define nng_recvmsg em_nng_recvmsg
#define nng_free em_nng_free
#define nng_free em_nng_free
#define nng_msg_free em_nng_msg_free
#define nng_pull_open em_nng_pull_open
#define nng_pull0_open em_nng_pull0_open
#define nng_listen em_nng_listen
#define nng_close em_nng_close
#define nng_getopt_int em_nng_getopt_int
#define nng_push0_open em_nng_push0_open
#define nng_dial em_nng_dial
#define nng_setopt em_nng_setopt
#define nng_sub_open em_nng_sub_open
#define nng_sub0_open em_nng_sub0_open
#define nng_recv em_nng_recv
#define nng_send em_nng_send
#define nng_sendmsg em_sendmsg
#define nng_alloc em_nng_alloc
#define nng_free em_nng_free
#define nng_dialer_setopt_ms em_dialer_setopt_ms
#define nng_dialer_start em_dialer_start
#define nng_dialer_create em_dialer_create
#define nng_msg_body em_msg_body
#define nng_msg_len em_msg_len


#else


// ----------------------- emulated nano functions --------------------------
struct em_nn_msghdr {
	int dummy;
};

static int em_nn_socket (int domain, int protocol ) {
	static int s = 1;

	return ++s;
}

static int em_nn_close (int s ) {
	return 1;
}

static int em_nn_setsockopt (int s, int level, int option, const void *optval, size_t optvallen ) {
	return 1;
}

static int em_nn_getsockopt (int s, int level, int option, void *optval, size_t *optvallen ) {
	return 1;
}

static int em_nn_bind (int s, const char *addr ) {
fprintf( stderr, ">>> ===== emulated bind called ====\n" );
	return 1;
}

static int em_nn_connect (int s, const char *addr ) {
	return 1;
}

static int em_nn_shutdown (int s, int how ) {
	return 1;
}

static int em_nn_send (int s, const void *buf, size_t len, int flags ) {
	return 1;
}

static int em_nn_recv (int s, void *buf, size_t len, int flags ) {
	return 1;
}

static int em_sendmsg (int s, const struct em_nn_msghdr *msghdr, int flags ) {
	return 1;
}

static int em_nn_recvmsg (int s, struct nn_msghdr *msghdr, int flags ) {
	return 1;
}

// nanomsg
#define nn_socket  em_nn_socket
#define nn_close  em_nn_close
#define nn_setsockopt  em_nn_setsockopt
#define nn_getsockopt  em_nn_getsockopt
#define nn_bind  em_nn_bind
#define nn_connect  em_nn_connect
#define nn_shutdown  em_nn_shutdown
#define nn_send  em_nn_send
#define nn_recv  em_nn_recv
#define nn_sendmsg  em_nn_sendmsg
#define nn_recvmsg  em_nn_recvmsg

#endif


#endif

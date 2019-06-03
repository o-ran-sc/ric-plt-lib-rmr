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


#include "rmr.h" 				// we use some of rmr defs in building dummy messages, so we need these
#include "rmr_agnostic.h"

// ---------------------- emulated nng functions ---------------------------


#ifndef _em_nn
#define _em_nn

#include <pthread.h>

static int em_send_failures = 0;	// test programme can set this to emulate eagain send failures
static int em_timeout = -1;			// set by set socket option
static int em_mtc_msgs = 0;			// set to generate 'received' messages with mt-call header data
static int return_value = 0;		// functions should return this value
static int rcv_count = 0;			// receive counter for transaction id to allow test to rest
static int rcv_delay = 0;			// forced delay before call to rcvmsg starts to work

static int gates_ok = 0;
static pthread_mutex_t rcv_gate;
static int em_gen_long_hostname = 0;		// if set the emulated hostname generates a longer name (>40 char)


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

// CAUTION: sys/epoll.h must be included before these define and function will properly compile.
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

int em_ep_ctl( int epfd, int op, int fd, struct epoll_event *event ) {
	return 0;
}

int em_ep_create( int size ) {
	return 0;
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
	unsigned char srcip[64];				// sender ID for return to sender needs
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
	rcv_delay = v;
}

static void em_start() {
	if( ! gates_ok ) {
		pthread_mutex_init( &rcv_gate, NULL );
		gates_ok = 1;
	}
}

//--------------------------------------------------------------------------
#ifdef EMULATE_NNG
struct nn_msghdr {
	int boo;
};


/*
	Receive message must allocate a new buffer and return the pointer into *m.
	Every 9 messages or so we'll simulate an old version message

	If em_mtc_msgs is set, then we add a non-zero d1 field with
	the call-id set to 2, and alternate the call flag
*/
static int em_nng_recvmsg( nng_socket s, nng_msg ** m, int i ) {
	static int call_flag = 0;

	void* b;
	struct em_msg* msg;
	int trace_size = 0;
	int d1_size = 0;
	unsigned char* d1;

	if( rcv_delay > 0 ) {
		sleep( rcv_delay );
	}

	if( em_mtc_msgs ) {
		d1_size = 4;
	}

	if( m != NULL ) {
		b = (void *) malloc( 2048 );
		memset( b, 0, 2048 );

		*m = (nng_msg *) b;
		msg = (struct em_msg *) b;
		if( ! em_mtc_msgs  &&  (rcv_count % 10) == 9 ) {
			msg->rmr_ver = ALT_MSG_VER;							// allow emulation the bug in RMr v1
		} else {
			msg->rmr_ver = htonl( MSG_VER );
		}

		msg->mtype = htonl( 1 );
		msg->plen = htonl( 220 );
		msg->len0 = htonl( sizeof( struct em_msg ) );
		msg->len1 = htonl( trace_size );
		msg->len2 = htonl( d1_size );
		msg->len3 = htonl( 0 );

		pthread_mutex_lock( &rcv_gate );	// hold lock to update counter/flag
		if( em_mtc_msgs ) {
			d1 = DATA1_ADDR( msg );
			d1[0] = 2;									// simulated msgs always on chute 2
			if( call_flag ) {
				rcv_count++;
				msg->flags |= HFL_CALL_MSG;
			}
			if( rcv_delay > 0 ) {
				fprintf( stderr, "<EM>    count=%d flag=%d %02x \n", rcv_count, call_flag, msg->flags );
			}
			call_flag = !call_flag;
		} else {
			rcv_count++;
		}
		pthread_mutex_unlock( &rcv_gate );
		snprintf( msg->xid, 32, "%015d", rcv_count );		// simple transaction id so we can test receive specific and ring stuff
		snprintf( msg->src, 64, "localhost:4562" );		// set src id (unrealistic) so that rts() can be tested
		snprintf( msg->srcip, 64, "89.2.19.19:4562" );		// set src ip for rts testing

		//fprintf( stderr, ">>> simulated received message: %s %s p=%p len0=%d\n", msg->src, msg->srcip, msg, (int) ntohl( msg->len0 ) );
	} else {
		fprintf( stderr, "<WARN> em: simulated receive no msg pointer provided\n" );
	}

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

//static int em_nn_setsockopt (int s, int level, int option, const void *optval, size_t optvallen ) {
	//return 1;
//}

static int em_nn_getsockopt (int s, int level, int option, void *optval, size_t *optvallen ) {
	return 1;
}

static int em_nn_bind (int s, const char *addr ) {
	//	fprintf( stderr, ">>> ===== emulated bind called ====\n" );
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

static int em_nn_recv (int s, void *m, size_t len, int flags ) {
	void* b;
	struct em_msg* msg;
	static int count = 0;			// we'll simulate a message going in by dropping an rmr-ish msg with transaction id only
	int trace_size = 0;
	static int counter = 0;				// if timeout value is set; we return timeout (eagain) every 3 calls
	int d1_size = 0;

	if( em_timeout > 0 ) {
		counter++;
		if( counter % 3 == 0 ) {
			return EAGAIN;
		}
	}

	if( em_mtc_msgs ) {
		d1_size = 4;
	}

	b = (void *) malloc( 2048 );
	if( m != NULL ) {						// blindly we assume this is 2k or bigger
		memset( m, 0, 2048 );
		msg = (struct em_msg *) m;
		if( count % 10  == 9 ) {
			//msg->rmr_ver = htonl( MSG_VER );
			msg->rmr_ver = ALT_MSG_VER;		// emulate the bug in RMr v1
		} else {
			msg->rmr_ver = htonl( MSG_VER );
		}
		msg->mtype = htonl( 1 );
		msg->plen = htonl( 220 );
		msg->len0 = htonl( sizeof( struct em_msg ) );
		msg->len1 = htonl( trace_size );
		msg->len2 = htonl( d1_size );
		msg->len3 = htonl( 0 );
		snprintf( msg->xid, 32, "%015d", count++ );		// simple transaction id so we can test receive specific and ring stuff
		snprintf( msg->src, 64, "localhost:4562" );		// set src id (unrealistic) so that rts() can be tested
		snprintf( msg->srcip, 64, "89.2.19.19:4562" );		// set src ip for rts testing
		//fprintf( stderr, "<EM>   returning message len=%d\n\n", ntohl( msg->plen ) );
	} else {
		fprintf( stderr, "<EM>   message was nil\n\n" );
	}

	//fprintf( stderr, ">>> simulated received message: %s %s len=%d p=%p\n", msg->src, msg->srcip, ntohl( msg->plen ), m );
	return 2048;
}

static int em_sendmsg (int s, const struct em_nn_msghdr *msghdr, int flags ) {
	return 1;
}

static int em_nn_recvmsg (int s, struct nn_msghdr *msghdr, int flags ) {
	return 1;
}

static void em_nn_freemsg( void* ptr ) {
	return;
}

/*
	Hacky implementation of set sock opt. We assume value is a pointer to int and ignore size.
*/
static int em_setsockopt( int sock, int foo, int action, int* value, int size ) {
	if( action ==  NN_RCVTIMEO ) {
		em_timeout = *value;
	}
}


// nanomsg
#define nn_socket  em_nn_socket
#define nn_close  em_nn_close
//#define nn_setsockopt  em_nn_setsockopt
#define nn_getsockopt  em_nn_getsockopt
#define nn_bind  em_nn_bind
#define nn_connect  em_nn_connect
#define nn_shutdown  em_nn_shutdown
#define nn_send  em_nn_send
#define nn_recv  em_nn_recv
#define nn_sendmsg  em_nn_sendmsg
#define nn_recvmsg  em_nn_recvmsg
#define nn_setsockopt  em_setsockopt
#define nn_freemsg  em_nn_freemsg

#endif


#endif

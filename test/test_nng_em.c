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
#include "test_common_em.c"			// things common to all emulation code

//--------------------------------------------------------------------------
#ifdef EMULATE_NNG
struct nn_msghdr {
	int boo;
};

#define SOCKET_TYPE		nng_socket		// socket representation is different in each transport

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
	free( m );					// we must ditch the message as nng does (or reuses)
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
		free( p );
	}
}
static void em_nng_msg_free( void* p ) {
	if( p ) {
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

#define SOCKET_TYPE		int		// socket representation is different in each transport


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
	free( ptr );
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

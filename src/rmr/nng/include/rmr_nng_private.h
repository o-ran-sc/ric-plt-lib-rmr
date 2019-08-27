// : vi ts=4 sw=4 noet :
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
	Mnemonic:	uta_nng_private.h
	Abstract:	Private header information for the uta nng library functions.
				These are structs which have specific NNG types; anything that
				does not can be included in the common/rmr_agnostic.h header.

	Author:		E. Scott Daniels
	Date:		31 November 2018

	Mods:		28 Feb 2019 -- moved the majority to the agnostic header.
*/

#ifndef _uta_private_h
#define _uta_private_h

/*
	Manages an endpoint. Type def for this is defined in agnostic.
*/
struct endpoint {
	char*	name;			// end point name (symtab reference)
	char*	proto;			// connection proto (should only be TCP, but future might bring others)
	char*	addr;			// address used for connection
	nng_socket	nn_sock;	// the nano-msg socket to write to for this entry
	nng_dialer	dialer;		// the connection specific information (retry timout etc)
	int		open;			// set to true if we've connected as socket cannot be checked directly)
	pthread_mutex_t	gate;	// we must serialise when we open/link to the endpoint
};

/*
	Epoll information needed for the rmr_torcv_msg() funciton
*/
typedef struct epoll_stuff {
	struct epoll_event events[1];				// wait on 10 possible events
	struct epoll_event epe;						// event definition for event to listen to
	int ep_fd;									// file des from nng
	int nng_fd;									// fd from nng
} epoll_stuff_t;

/*
	Context describing our world. Should be returned to user programme on
	call to initialise, and passed as first parm on all calls to other
	visible functions.

	The typedef is declared in the agnostic header.
*/
struct uta_ctx {
	char*	my_name;			// dns name of this host to set in sender field of a message
	char*	my_ip;				// the ip address we _think_ we are using sent in src_ip of the message for rts
	int	shutdown;				// thread notification if we need to tell them to stop
	int max_mlen;				// max message length payload+header
	int	max_plen;				// max payload length
	int	flags;					// CTXFL_ constants
	int nrtele;					// number of elements in the routing table
	int send_retries;			// number of retries send_msg() should attempt if eagain/timeout indicated by nng
	int	trace_data_len;			// number of bytes to allocate in header for trace data
	int d1_len;					// extra header data 1 length
	int d2_len;					// extra header data 2 length	(future)
	nng_socket	nn_sock;		// our general listen socket
	route_table_t* rtable;		// the active route table
	route_table_t* old_rtable;	// the previously used rt, sits here to allow for draining
	route_table_t* new_rtable;	// route table under construction
	if_addrs_t*	ip_list;		// list manager of the IP addresses that are on our known interfaces
	void*	mring;				// ring where msgs are queued while waiting for a call response msg
	chute_t*	chutes;

	char*	rtg_addr;			// addr/port of the route table generation publisher
	int		rtg_port;			// the port that the rtg listens on

	wh_mgt_t*	wormholes;		// management of user opened wormholes
	epoll_stuff_t*	eps;		// epoll information needed for the rcv with timeout call

	pthread_t	rtc_th;			// thread info for the rtc listener
	pthread_t	mtc_th;			// thread info for the multi-thread call receive process
};



/*
	Static prototypes for functions located here. All common protos are in the
	agnostic header file.
*/

// --- initialisation and housekeeping -------
static void* init(  char* uproto_port, int max_msg_size, int flags );
static void free_ctx( uta_ctx_t* ctx );

// --- rt table things ---------------------------
static int uta_link2( endpoint_t* ep );
static int rt_link2_ep( endpoint_t* ep );
static int uta_epsock_byname( route_table_t* rt, char* ep_name, nng_socket* nn_sock );
//RBF static int uta_epsock_rr( route_table_t *rt, uint64_t key, int group, int* more, nng_socket* nn_sock );
static int uta_epsock_rr( rtable_ent_t* rte, int group, int* more, nng_socket* nn_sock );
static rtable_ent_t* uta_get_rte( route_table_t *rt, int sid, int mtype, int try_alt );
static inline int xlate_nng_state( int state, int def_state );


// --- msg ---------------------------------------
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state, int trlo );
static rmr_mbuf_t* alloc_mbuf( uta_ctx_t* ctx, int state );
static void ref_tpbuf( rmr_mbuf_t* msg, size_t alen ) ;
static inline rmr_mbuf_t* clone_msg( rmr_mbuf_t* old_msg  );
static rmr_mbuf_t* rcv_msg( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );
static void* rcv_payload( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );
static inline rmr_mbuf_t* realloc_msg( rmr_mbuf_t* old_msg, int tr_len  );
static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, nng_socket nn_sock, int retries );
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg );



#endif

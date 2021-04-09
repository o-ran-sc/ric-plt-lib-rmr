//  vim: ts=4 sw=4 noet :
/*
==================================================================================
	Copyright (c) 2019-2020 Nokia
	Copyright (c) 2018-2020 AT&T Intellectual Property.

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
	Mnemonic:	uta_si_private.h
	Abstract:	Private header information for the uta nng library functions.
				These are structs which have specific NNG types; anything that
				does not can be included in the common/rmr_agnostic.h header.

	Author:		E. Scott Daniels
	Date:		31 November 2018

	Mods:		28 Feb 2019 -- moved the majority to the agnostic header.
*/

#ifndef _uta_private_h
#define _uta_private_h

// if pmode is off we don't compile in some checks in hopes of speeding things up
#ifndef PARANOID_CHECKS
#	define PARANOID_CHECKS 0
#endif


// ---- si specific things  -------------------------------------------

#define TP_HDR_LEN	50		// bytes added to each message for transport needs

							// river states
#define RS_NEW		0		// river is unitialised
#define RS_GOOD		1		// flow is in progress
#define RS_RESET	2		// flow was interrupted; reset on next receive

#define RF_NOTIFIED	0x01	// notification made about river issue
#define RF_DROP		0x02	// this message is large and being dropped

#define	TP_SZFIELD_LEN	((sizeof(uint32_t)*2)+1)	// number of bytes needed for msg size in transport header
#define	TP_SZ_MARKER	'$'							// marker indicating net byte order used


#define SI_MAX_ADDR_LEN		512
#define MAX_RIVERS			1024	// max number of directly mapped rivers

/*
	Manages a river of inbound bytes.
*/
typedef struct {
	int		state;		// RS_* constants
	char*	accum;		// bytes being accumulated
	int		nbytes;		// allocated size of accumulator
	int		ipt;		// insertion point in accumulator
	int		msg_size;	// size of the message being accumulated
	int		flags;		// RF_* constants
} river_t;


/*
	Callback context.
typedef struct {
	uta_ctx_t*	ctx;

} cbctx_t;
*/

// ---------------------------- mainline rmr things ----------------


/*
	Manages an endpoint. Type def for this is defined in agnostic.
*/
struct endpoint {
	char*	name;			// end point name (symtab reference)
	char*	proto;			// connection proto (should only be TCP, but future might bring others)
	char*	addr;			// address used for connection
	int		nn_sock;		// we'll keep calling it nn_ because it's less changes to the code
	//nng_dialer	dialer;		// the connection specific information (retry timout etc)
	int		open;			// set to true if we've connected as socket cannot be checked directly)
	pthread_mutex_t	gate;	// we must serialise when we open/link to the endpoint
	long long scounts[EPSC_SIZE];		// send counts (indexed by EPSCOUNT_* constants

							// SI specific things
	int notify;				// if we fail, we log once until a connection happens; notify if set
};

/*
	Epoll information needed for the rmr_torcv_msg() funciton
*/
typedef struct epoll_stuff {
	struct epoll_event events[1];				// wait on 1 possible events
	struct epoll_event epe;						// event definition for event to listen to
	int ep_fd;									// file des from nng
	int poll_fd;								// fd from nng
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
	int	flags;					// CFL_ constants
	int nrtele;					// number of elements in the routing table
	int send_retries;			// number of retries send_msg() should attempt if eagain/timeout indicated by nng
	int	trace_data_len;			// number of bytes to allocate in header for trace data
	int d1_len;					// extra header data 1 length
	int d2_len;					// extra header data 2 length	(future)
	int	nn_sock;				// our general listen socket
	int rtable_ready;			// set to true when rt is received or loaded
	int snarf_rt_fd;			// the file des where we save the last rt from RM
	int dcount;					// drop counter when app is slow
	char*	seed_rt_fname;		// the static/seed route table; name captured at start
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

								// added for route manager request/states
	rmr_whid_t	rtg_whid;		// wormhole id to the route manager for acks/requests
	char*		table_id;		// table ID of the route table load in progress

								// added for SI95 support
	si_ctx_t*	si_ctx;			// the socket context
	int			nrivers;		// allocated rivers
	river_t*	rivers;			// inbound flows (index is the socket fd)
	void*		river_hash;		// flows with fd values > nrivers must be mapped through the hash
	int			max_ibm;		// max size of an inbound message (river accum alloc size)
	void*		zcb_mring;		// zero copy buffer mbuf ring
	void*		fd2ep;				// the symtab mapping file des to endpoints for cleanup on disconnect
	void*		ephash;				// hash  host:port or ip:port to endpoint struct

	pthread_mutex_t	*fd2ep_gate;	// we must gate add/deletes to the fd2 symtab
	pthread_mutex_t	*rtgate;		// master gate for accessing/moving route tables
};

typedef uta_ctx_t uta_ctx;


/*
	Static prototypes for functions located here. All common protos are in the
	agnostic header file.
*/

// --- initialisation and housekeeping -------
static void* init(  char* uproto_port, int max_msg_size, int flags );
static void free_ctx( uta_ctx_t* ctx );

// --- rt table things ---------------------------
static void uta_ep_failed( endpoint_t* ep );
static int uta_link2( uta_ctx_t *ctx, endpoint_t* ep );

static int rt_link2_ep( void* vctx, endpoint_t* ep );
static rtable_ent_t* uta_get_rte( route_table_t *rt, int sid, int mtype, int try_alt );
static inline int xlate_si_state( int state, int def_state );

// --- these have changes for si
static int uta_epsock_byname( uta_ctx_t* ctx, char* ep_name, int* nn_sock, endpoint_t** uepp );
//static int uta_epsock_rr( rtable_ent_t *rte, int group, int* more, int* nn_sock, endpoint_t** uepp, si_ctx_t* si_ctx );
static int uta_epsock_rr( uta_ctx_t* ctx, rtable_ent_t *rte, int group, int* more, int* nn_sock, endpoint_t** uepp );


// --- msg ---------------------------------------
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state, int trlo );
static rmr_mbuf_t* alloc_mbuf( uta_ctx_t* ctx, int state );
static void ref_tpbuf( rmr_mbuf_t* msg, size_t alen ) ;
static inline rmr_mbuf_t* clone_msg( rmr_mbuf_t* old_msg  );
static rmr_mbuf_t* rcv_msg( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );
static inline rmr_mbuf_t* realloc_msg( rmr_mbuf_t* old_msg, int tr_len  );
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg );

static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int nn_sock, int retries );

// ---- fd to endpoint translation ------------------------------
static endpoint_t*  fd2ep_del( uta_ctx_t* ctx, int fd );
static endpoint_t*  fd2ep_get( uta_ctx_t* ctx, int fd );
static void fd2ep_init( uta_ctx_t* ctx );
static void fd2ep_add( uta_ctx_t* ctx, int fd, endpoint_t* ep );

#endif

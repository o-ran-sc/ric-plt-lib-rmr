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
	Mnemonic:	rmr_private.h
	Abstract:	Private header information for the uta library functions.
				This should contain only things which are specific to nanomsg;
				anything else is defined in the common/rmr_agnostic.h header.

	Author:		E. Scott Daniels
	Date:		27 November 2018

	Mods:		28 February 2019 - moved most of the crap here to agnosic.
*/

#ifndef _rmr_private_h
#define _rmr_private_h

/*
	Manages an endpoint. Typedef for this is defined in agnostic.h
*/
struct endpoint {
	char*	name;			// end point name (symtab reference)
	char*	proto;			// connection proto (should only be TCP, but future might bring others)
	char*	addr;			// address used for connection
	int		nn_sock;		// the nano-msg socket to write to for this entry
	int		open;			// true if we've established the connection
	pthread_mutex_t gate;		// must be able to serialise some transport level functions on the ep
};

/*
	Context describing our world. Should be returned to user programme on
	call to initialise, and passed as first parm on all calls to other
	visible functions.

	The typedef for ctx is in the agnostic header
*/
struct uta_ctx {
	char*	my_name;			// dns name of this host to set in sender field of a message
	int		shutdown;			// threads should exit if this is set
	int max_mlen;				// max message length payload+header
	int	max_plen;				// max payload length
	int	flags;					// CTXFL_ constants
	int nrtele;					// number of elements in the routing table
	int	nn_sock;				// our general listen socket
	int	trace_data_len;			// len of tracing data that sits just past header (0 means none)
	int	d1_len;					// lengths for additional post header, pre payload data areas
	int d2_len;
	int last_rto;				// last receive timeout set so that we don't bash in on every call
	route_table_t* rtable;		// the active route table
	route_table_t* old_rtable;	// the previously used rt, sits here to allow for draining
	route_table_t* new_rtable;	// route table under construction
	if_addrs_t*	ip_list;		// list manager of the IP addresses that are on our known interfaces
	void*	mring;				// ring where msgs are queued while waiting for a call response msg

	char*	rtg_addr;			// addr/port of the route table generation publisher
	int		rtg_port;			// the port that the rtg listens on

	wh_mgt_t*	wormholes;		// wormhole management
	pthread_t	rtc_th;			// thread info for the rtc listener
};


/*
	Prototypes of the functions which are defined in our static modules (nothing
	from common should be here).
*/

// ---- housekeeping and initialisation ----------
static void* init( char* usr_port, int max_mlen, int flags );
static void free_ctx( uta_ctx_t* ctx );

// --- message and context management --------
static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int nn_sock );
static void* rcv_payload( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );


// ---- route table and connection management ---------------

static int uta_link2( char* target );
static int rt_link2_ep( endpoint_t* ep );
static endpoint_t*  uta_add_ep( route_table_t* rt, rtable_ent_t* rte, char* ep_name, int group  );
static int uta_epsock_byname( route_table_t* rt, char* ep_name );
static int uta_epsock_rr( route_table_t *rt, uint64_t key, int group, int* more );

// ------ msg ------------------------------------------------
static rmr_mbuf_t* alloc_zcmsg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int size, int state, int tr_size );
static inline rmr_mbuf_t* clone_msg( rmr_mbuf_t* old_msg  );
static rmr_mbuf_t* rcv_msg( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );
static void* rcv_payload( uta_ctx_t* ctx, rmr_mbuf_t* old_msg );
static rmr_mbuf_t* send_msg( uta_ctx_t* ctx, rmr_mbuf_t* msg, int nn_sock );
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg );

static int rt_link2_ep( endpoint_t* ep );
static rmr_mbuf_t* send2ep( uta_ctx_t* ctx, endpoint_t* ep, rmr_mbuf_t* msg );

#endif

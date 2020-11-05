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
	Mnemonic:	rmr_agnostic.h
	Abstract:	Header file for things that are agnostic to the underlying transport
				mechanism.
	Author:		E. Scott Daniels
	Date:		28 February 2018
*/

#ifndef _rmr_agnostic_h
#define _rmr_agnostic_h

#include <semaphore.h>					// needed to support some structs
#include <pthread.h>

typedef struct endpoint endpoint_t;		// place holder for structs defined in nano/nng private.h
typedef struct uta_ctx  uta_ctx_t;

// allow testing to override without changing this
#ifndef DEBUG
#define DEBUG 0
#endif

#define FALSE 0
#define TRUE  1

#define QUOTE(a) #a				// allow a constant to be quoted
#define QUOTE_DEF(a) QUOTE(a)	// allow a #define value to be quoted (e.g. QUOTE(MAJOR_VERSION) )


#define RT_SIZE			10009	// primary entries in route table (prime) meids hash through this so larger than expected # meids
								// space deginations in the hash table
#define RT_MT_SPACE	0		// (integer) message type as the key
#define RT_NAME_SPACE	1		// enpoint name/address is the key
#define RT_ME_SPACE	2		// message id is the key

#define RMR_MSG_VER	3			// message version this code was designed to handle

											// environment variable names we'll suss out
#define ENV_BIND_IF		"RMR_BIND_IF"		// the interface to bind to for both normal comma and RTG (0.0.0.0 if missing)
#define ENV_RTG_PORT	"RMR_RTG_SVC"		// the port we'll listen on for rtg connections (deprecated; see RTG_SVC and CTL_PORT)
#define ENV_RTG_ADDR	"RMR_RTG_SVC"		// the address we will connect to for route manager updates
#define ENV_SEED_RT		"RMR_SEED_RT"		// where we expect to find the name of the seed route table
#define ENV_SEED_MEMAP	"RMR_SEED_MEMAP"	// where we expect to find the name of the seed route table
#define ENV_RTG_RAW		"RMR_RTG_ISRAW"		// if > 0 we expect route table gen messages as raw (not sent from an RMr application)
#define ENV_VERBOSE_FILE "RMR_VCTL_FILE"	// file where vlevel may be managed for some (non-time critical) functions
#define ENV_NAME_ONLY	"RMR_SRC_NAMEONLY"	// src in message is name only
#define ENV_WARNINGS	"RMR_WARNINGS"		// if == 1 then we write some, non-performance impacting, warnings
#define ENV_SRC_ID		"RMR_SRC_ID"		// forces this string (adding :port, max 63 ch) into the source field; host name used if not set
#define ENV_LOG_HR 		"RMR_HR_LOG"		// set to 0 to turn off human readable logging and write using some formatting
#define ENV_LOG_VLEVEL	"RMR_LOG_VLEVEL"	// set the verbosity level (0 == 0ff; 1 == crit .... 5 == debug )
#define ENV_CTL_PORT	"RMR_CTL_PORT"		// route collector will listen here for control messages (4561 default)
#define ENV_RTREQ_FREA  "RMR_RTREQ_FREQ"	// frequency we will request route table updates when we want one (1-300 inclusive)

#define NO_FLAGS	0				// no flags to pass to a function

#define FL_NOTHREAD	0x01			// do not start an additional thread (must be 'user land' to support rtg
#define UFL_MASK 		0xff		// mask applied to some flag parms passed by the user to remove any internal flags
									// internal flags, must be > than UFLAG_MASK
//#define IFL_....

#define CFL_MTC_ENABLED	0x01		// multi-threaded call is enabled
#define CFL_NO_RTACK	0x02		// no route table ack needed when end received

									// context flags
#define CTXFL_WARN		0x01		// ok to warn on stderr for some things that shouldn't happen

									// msg buffer flags
#define MFL_ZEROCOPY	0x01		// the message is an allocated zero copy message and can be sent.
#define MFL_NOALLOC		0x02		// send should NOT allocate a new buffer before returning
#define MFL_ADDSRC		0x04		// source must be added on send
#define MFL_RAW			0x08		// message is 'raw' and not from an RMr based sender (no header)
#define MFL_HUGE		0x10		// buffer was larger than applications indicated usual max; don't cache

#define MAX_EP_GROUP	32			// max number of endpoints in a group
#define MAX_RTG_MSG_SZ	2048		// max expected message size from route generator
#define MAX_CALL_ID		255			// largest call ID that is supported

//#define DEF_RTG_MSGID	""				// default to pick up all messages from rtg
#define DEF_CTL_PORT	"4561"			// default control port that rtc listens on
#define DEF_RTG_PORT	"tcp:4561"		// default port that we accept rtg connections on (deprecated)
#define DEF_COMM_PORT	"tcp:4560"		// default port we use for normal communications
#define DEF_RTG_WK_ADDR	"routemgr:4561"	// well known address for the route manager
#define DEF_TR_LEN		(-1)			// use default trace data len from context
#define DEF_RTREQ_FREQ	5				// delay between route table requests

#define UNSET_SUBID		(-1)			// initial value on msg allocation indicating not set
#define UNSET_MSGTYPE	(-1)

										// index values into the send counters for an enpoint
#define EPSC_GOOD		0				// successful send
#define EPSC_FAIL		1				// hard failurs
#define EPSC_TRANS		2				// transient/soft faiures
#define EPSC_SIZE		3				// number of counters

// -- header length/offset macros must ensure network conversion ----
#define RMR_HDR_LEN(h)		(ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1)+htonl(((uta_mhdr_t *)h)->len2)+htonl(((uta_mhdr_t *)h)->len3)) // ALL things, not just formal struct
#define RMR_TR_LEN(h) 		(ntohl(((uta_mhdr_t *)h)->len1))
#define RMR_D1_LEN(h) 		(ntohl(((uta_mhdr_t *)h)->len2))
#define RMR_D2_LEN(h) 		(ntohl(((uta_mhdr_t *)h)->len3))

// CAUTION:  if using an offset with a header pointer, the pointer MUST be cast to void* before adding the offset!
#define TRACE_OFFSET(h)		((ntohl(((uta_mhdr_t *)h)->len0)))
#define DATA1_OFFSET(h)		(ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1))
#define DATA2_OFFSET(h)		(ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1)+htonl(((uta_mhdr_t *)h)->len2))
#define PAYLOAD_OFFSET(h)	(ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1)+htonl(((uta_mhdr_t *)h)->len2)+htonl(((uta_mhdr_t *)h)->len3))

#define TRACE_ADDR(h)		(((void *)h)+ntohl(((uta_mhdr_t *)h)->len0))
#define DATA1_ADDR(h)		(((void *)h)+ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1))
#define DATA2_ADDR(h)		(((void *)h)+ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1)+htonl(((uta_mhdr_t *)h)->len2))
#define PAYLOAD_ADDR(h)		(((void *)h)+ntohl(((uta_mhdr_t *)h)->len0)+htonl(((uta_mhdr_t *)h)->len1)+htonl(((uta_mhdr_t *)h)->len2)+htonl(((uta_mhdr_t *)h)->len3))

#define SET_HDR_LEN(h)		(((uta_mhdr_t *)h)->len0=htonl((int32_t)sizeof(uta_mhdr_t)))		// convert to network byte order on insert
#define SET_HDR_TR_LEN(h,l)	(((uta_mhdr_t *)h)->len1=htonl((int32_t)l))
#define SET_HDR_D1_LEN(h,l)	(((uta_mhdr_t *)h)->len2=htonl((int32_t)l))
#define SET_HDR_D2_LEN(h,l)	(((uta_mhdr_t *)h)->len3=htonl((int32_t)l))

#define HDR_VERSION(h)	htonl((((uta_mhdr_t *)h)->rmr_ver))

							// index of things in the d1 data space
#define D1_CALLID_IDX	0	// the call-id to match on return

#define	NO_CALL_ID		0	// no call id associated with the message (normal queue)

#define V1_PAYLOAD_OFFSET(h)	(sizeof(uta_v1mhdr_t))

										// v2 header flags
#define HFL_HAS_TRACE	0x01			// Trace data is populated
#define HFL_SUBID		0x02			// subscription ID is populated
#define HFL_CALL_MSG	0x04			// msg sent via blocking call

/*
	Message header; interpreted by the other side, but never seen by
	the user application.

	DANGER: Add new fields AT THE END of the struct. Adding them any where else
			will break any code that is currently running.

	The transport layer buffer allocated will be divided this way:
		| RMr header | Trace data | data1 | data2 | User paylaod |

		Len 0 is the length of the RMr header
		Len 1 is the length of the trace data
		Len 2 and 3 are lengths of data1 and data2 and are unused at the moment

	To point at the payload, we take the address of the header and add all 4 lengths.
*/
typedef struct {
	int32_t	mtype;						// message type  ("long" network integer)
	int32_t	plen;						// payload length (sender data length in payload)
	int32_t rmr_ver;					// our internal message version number
	unsigned char xid[RMR_MAX_XID];		// space for user transaction id or somesuch
	unsigned char sid[RMR_MAX_SID];		// sender ID for return to sender needs
	unsigned char src[RMR_MAX_SRC];		// name:port of the sender (source)
	unsigned char meid[RMR_MAX_MEID];	// managed element id.
	struct timespec	ts;					// timestamp ???

										// V2 extension
	int32_t	flags;						// HFL_* constants
	int32_t	len0;						// length of the RMr header data
	int32_t	len1;						// length of the tracing data
	int32_t	len2;						// length of data 1 (d1)
	int32_t	len3;						// length of data 2 (d2)
	int32_t	sub_id;						// subscription id (-1 invalid)

										// v3 extension
	unsigned char srcip[RMR_MAX_SRC];	// ip address and port of the source
} uta_mhdr_t;


typedef struct {						// old (inflexible) v1 header
	int32_t	mtype;						// message type  ("long" network integer)
	int32_t	plen;						// payload length
	int32_t rmr_ver;					// our internal message version number
	unsigned char xid[RMR_MAX_XID];		// space for user transaction id or somesuch
	unsigned char sid[RMR_MAX_SID];		// misc sender info/data
	unsigned char src[16];				// name of the sender (source) (old size was 16)
	unsigned char meid[RMR_MAX_MEID];	// managed element id.
	struct timespec	ts;					// timestamp ???
} uta_v1mhdr_t;

/*
	Round robin group.
*/
typedef struct {
	uint16_t	ep_idx;		// next endpoint to send to
	int nused;				// number of endpoints in the list
	int nendpts;			// number allocated
	endpoint_t **epts;		// the list of endpoints that we RR over
} rrgroup_t;

/*
	Routing table entry. This is a list of endpoints that can be sent
	messages of the given mtype.  If there is more than one, we will
	round robin messags across the list.
*/
typedef struct {
	uint64_t key;			// key used to reinsert this entry into a new symtab
	int	refs;				// number of symtabs which reference the entry
	int mtype;				// the message type for this list
	int	nrrgroups;			// number of rr groups to send to (if 0, the meid in a message determines endpoint)
	rrgroup_t**	rrgroups;	// one or more set of endpoints to round robin messages to
} rtable_ent_t;

/*
	The route table.
*/
typedef struct {
	void*	hash;			// hash table for msg type and meid.
	void*	ephash;			// hash for endpoint references
	int		updates;		// counter of update records received
	int		mupdates;		// counter of meid update records received
	int		ref_count;		// num threads currently using
	pthread_mutex_t*	gate;	// lock allowing update to ref counter
} route_table_t;

/*
	A wormhole is a direct connection between two endpoints that the user app can
	send to without message type based routing.
*/
typedef struct {
	int	nalloc;				// number of ep pointers allocated
	endpoint_t** eps;		// end points directly referenced
} wh_mgt_t;


/*
	This manages an array of pointers to IP addresses that are associated with one of our interfaces.
	For now, we don't need to map the addr to a specific interface, just know that it is one of ours.
*/
typedef struct {
	char**	addrs;			// all ip addresses we found
	int		naddrs;			// num actually used
} if_addrs_t;


// --------------- ring things  -------------------------------------------------
#define RING_NONE	0			// no options
#define RING_RLOCK	0x01		// create/destroy the read lock on the ring
#define RING_WLOCK	0x02		// create/destroy the write lockk on the ring

typedef struct ring {
	uint16_t head;				// index of the head of the ring (insert point)
	uint16_t tail;				// index of the tail (extract point)
	uint16_t nelements;			// number of elements in the ring
	void**	data;				// the ring data (pointers to blobs of stuff)
	int		pfd;				// event fd for the ring for epoll
	pthread_mutex_t*	rgate;	// read lock if used
	pthread_mutex_t*	wgate;	// write lock if used
} ring_t;


// --------- multi-threaded call things -----------------------------------------
/*
	A chute provides a return path for a received message that a thread has blocked
	on.  The receive thread will set the mbuf pointer and tickler the barrier to
	signal to the call thread that data is ready.
*/
typedef struct chute {
	rmr_mbuf_t*	mbuf;						// pointer to message buffer received
	sem_t	barrier;						// semaphore that the thread is waiting on
	unsigned char	expect[RMR_MAX_XID];	// the expected transaction ID
} chute_t;


// -------------- common static prototypes --------------------------------------

//---- tools ----------------------------------
static int has_myip( char const* buf, if_addrs_t* list, char sep, int max );
static int uta_tokenise( char* buf, char** tokens, int max, char sep );
static int uta_rmip_tokenise( char* buf, if_addrs_t* iplist, char** toks, int max, char sep );
static char* uta_h2ip( char const* hname );
#ifdef RTG_PUB
// deprecated funciton -- step 1 of removal
static int uta_lookup_rtg( uta_ctx_t* ctx );
#endif
static int uta_has_str( char const* buf, char const* str, char sep, int max );
static char* get_default_ip( if_addrs_t* iplist );

// --- message ring --------------------------
static void* uta_mk_ring( int size );
static int uta_ring_config( void* vr, int options );
static void uta_ring_free( void* vr );
static inline void* uta_ring_extract( void* vr );
static inline int uta_ring_insert( void* vr, void* new_data );

// --- message and context management --------
static int ie_test( void* r, int i_factor, long inserts );


// ----- route table generic static things ---------
static inline uint64_t build_rt_key( int32_t sub_id, int32_t mtype );
static void collect_things( void* st, void* entry, char const* name, void* thing, void* vthing_list );
static void del_rte( void* st, void* entry, char const* name, void* thing, void* data );
static endpoint_t*  get_meid_owner( route_table_t *rt, char const* meid );
static char* uta_fib( char const* fname );
static route_table_t* uta_rt_init( uta_ctx_t* ctx  );
static route_table_t* uta_rt_clone( uta_ctx_t* ctx, route_table_t* srt, route_table_t* drt, int all );
static void uta_rt_drop( route_table_t* rt );
static inline route_table_t* get_rt( uta_ctx_t* ctx );
static endpoint_t*  uta_add_ep( route_table_t* rt, rtable_ent_t* rte, char* ep_name, int group  );
static rtable_ent_t* uta_add_rte( route_table_t* rt, uint64_t key, int nrrgroups );
static endpoint_t* uta_get_ep( route_table_t* rt, char const* ep_name );
static void read_static_rt( uta_ctx_t* ctx, int vlevel );
static route_table_t* prep_new_rt( uta_ctx_t* ctx, int all );
static void parse_rt_rec( uta_ctx_t* ctx,  uta_ctx_t* pctx, char* buf, int vlevel, rmr_mbuf_t* mbuf );
static rmr_mbuf_t* realloc_msg( rmr_mbuf_t* msg, int size );
static void release_rt( uta_ctx_t* ctx, route_table_t* rt );
static void* rtc( void* vctx );
static endpoint_t* rt_ensure_ep( route_table_t* rt, char const* ep_name );

// --------- route manager communications -----------------
static void send_rt_ack( uta_ctx_t* ctx, rmr_mbuf_t* mbuf, char* table_id, int state, char* reason );
static int send_update_req( uta_ctx_t* pctx, uta_ctx_t* ctx );

// -------- internal functions that can be referenced by common functions -------
static rmr_mbuf_t* mt_call( void* vctx, rmr_mbuf_t* mbuf, int call_id, int max_wait, endpoint_t* ep );


#endif

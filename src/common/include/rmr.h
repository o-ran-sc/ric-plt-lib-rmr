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
	Mnemonic:	rmr.h
	Abstract:	General (public) header file for the uta message routing library
	Author:		E. Scott Daniels
	Date:		27 November 2018
*/

#ifndef _rmr_h
#define _rmr_h

#include <sys/epoll.h>		// broken on mac

#ifdef __cplusplus
extern "C" {
#endif


#define RMR_MAX_XID			32		// space in header reserved for user xaction id
#define RMR_MAX_SID			32		// spece in header reserved for sender id
#define RMR_MAX_MEID		32		// spece in header reserved for managed element id
#define RMR_MAX_SRC			64		// max length of hostname (which could be IPv6 addr with [...]:port so more than the 39 bytes of a plain addr
#define RMR_MAX_RCV_BYTES	4096	// max bytes we support in a receive message

									// various flags for function calls
#define RMRFL_NONE			0x00	// no flags
#define RMRFL_AUTO_ALLOC	0x01	// send auto allocates a zerocopy buffer

#define RMR_DEF_SIZE		0		// pass as size to have msg allocation use the default msg size


#define RMR_OK				0		// state is good
#define RMR_ERR_BADARG		1		// argument passd to function was unusable
#define RMR_ERR_NOENDPT		2		// send/call could not find an endpoint based on msg type
#define RMR_ERR_EMPTY		3		// msg received had no payload; attempt to send an empty message
#define RMR_ERR_NOHDR		4		// message didn't contain a valid header
#define RMR_ERR_SENDFAILED	5		// send failed; errno has nano reason
#define RMR_ERR_CALLFAILED	6		// unable to send call() message
#define RMR_ERR_NOWHOPEN	7		// no wormholes are open
#define RMR_ERR_WHID		8		// wormhole id was invalid
#define RMR_ERR_OVERFLOW	9		// operation would have busted through a buffer/field size
#define RMR_ERR_RETRY		10		// request (send/call/rts) failed, but caller should retry (EAGAIN for wrappers)
#define RMR_ERR_RCVFAILED	11		// receive failed (hard error)
#define RMR_ERR_TIMEOUT		12		// message processing call timed out
#define RMR_ERR_UNSET		13		// the message hasn't been populated with a transport buffer
#define	RMR_ERR_TRUNC		14		// received message likely truncated
#define RMR_ERR_INITFAILED	15		// initialisation of something (probably message) failed

#define RMR_WH_CONNECTED(a) (a>=0)	// for now whid is integer; it could be pointer at some future date

/*
	General message buffer. Passed to send and returned by receive.

	(All fields are exposed such that if a wrapper needs to dup the storage as it passes
	into or out of their environment they dup it all, not just what we choose to expose.)
*/
typedef struct {
	int		state;				// state of processing
	int	mtype;					// message type
	int	len;					// length of data in the payload (send or received)
	unsigned char* payload;		// transported data
	unsigned char* xaction;		// pointer to fixed length transaction id bytes

								// these things are off limits to the user application
	void*	tp_buf;				// underlying transport allocated pointer (e.g. nng message)	
	void*	header;				// internal message header (whole buffer: header+payload)
	unsigned char* id;			// if we need an ID in the message separate from the xaction id
	int		flags;				// various MFL_ (private) flags as needed
	int		alloc_len;			// the length of the allocated space (hdr+payload)
} rmr_mbuf_t;


typedef int rmr_whid_t;			// wormhole identifier returned by rmr_wh_open(), passed to rmr_wh_send_msg()


// ---- library message specific prototypes ------------------------------------------------------------
extern rmr_mbuf_t* rmr_alloc_msg( void* vctx, int size );
extern rmr_mbuf_t* rmr_call( void* vctx, rmr_mbuf_t* msg );
extern void rmr_close( void* vctx );
extern void* rmr_init( char* proto_port, int max_msg_size, int flags );
extern int rmr_init_trace( void* vctx, int size );
extern int rmr_payload_size( rmr_mbuf_t* msg );
extern rmr_mbuf_t* rmr_send_msg( void* vctx, rmr_mbuf_t* msg );
extern rmr_mbuf_t* rmr_mtosend_msg( void* vctx, rmr_mbuf_t* msg, int max_to );
extern rmr_mbuf_t* rmr_rcv_msg( void* vctx, rmr_mbuf_t* old_msg );
extern rmr_mbuf_t* rmr_rcv_specific( void* uctx, rmr_mbuf_t* msg, char* expect, int allow2queue );
extern rmr_mbuf_t*  rmr_rts_msg( void* vctx, rmr_mbuf_t* msg );
extern int rmr_ready( void* vctx );
extern int rmr_set_rtimeout( void* vctx, int time );
extern int rmr_set_stimeout( void* vctx, int time );
extern int rmr_get_rcvfd( void* vctx );								// only supported with nng
extern rmr_mbuf_t* rmr_torcv_msg( void* vctx, rmr_mbuf_t* old_msg, int ms_to );
extern rmr_mbuf_t*  rmr_tralloc_msg( void* context, int msize, int trsize, unsigned const char* data );
extern rmr_whid_t rmr_wh_open( void* vctx, char const* target );
extern rmr_mbuf_t* rmr_wh_send_msg( void* vctx, rmr_whid_t whid, rmr_mbuf_t* msg );
extern void rmr_wh_close( void* vctx, int whid );


// ----- msg buffer operations (no context needed) ------------------------------------------------------
extern int rmr_bytes2meid( rmr_mbuf_t* mbuf, unsigned char const* src, int len );
extern void rmr_bytes2payload( rmr_mbuf_t* mbuf, unsigned char const* src, int len );
extern int rmr_bytes2xact( rmr_mbuf_t* mbuf, unsigned char const* src, int len );
extern void rmr_free_msg( rmr_mbuf_t* mbuf );
extern unsigned char*  rmr_get_meid( rmr_mbuf_t* mbuf, unsigned char* dest );
extern rmr_mbuf_t* rmr_realloc_msg( rmr_mbuf_t* mbuf, int new_tr_size );
extern int rmr_str2meid( rmr_mbuf_t* mbuf, unsigned char const* str );
extern void rmr_str2payload( rmr_mbuf_t* mbuf, unsigned char const* str );
extern void rmr_str2payload( rmr_mbuf_t* mbuf, unsigned char const* str );
extern int rmr_str2xact( rmr_mbuf_t* mbuf, unsigned char const* str );

extern int rmr_get_trlen( rmr_mbuf_t* msg );
extern int rmr_get_trace( rmr_mbuf_t* msg, unsigned char* dest, int size );
extern int rmr_set_trace( rmr_mbuf_t* msg, unsigned const char* data, int size );

extern int rmr_rcv_to( void* vctx, int time );		// DEPRECATED -- replaced with set_rtimeout
extern int rmr_send_to( void* vctx, int time );		// DEPRECATED -- replaced with set_stimeout


// --- uta compatability defs if needed user should define UTA_COMPAT  ----------------------------------
#ifdef UTA_COMPAT
#pragma message( "use of UTA_COMPAT is deprecated and soon to be removed" )

#define UTA_MAX_XID RMR_MAX_XID 
#define UTA_MAX_SID	RMR_MAX_SID 
#define UTA_MAX_SRC RMR_MAX_SRC 
#define UTA_MAX_RCV_BYTES RMR_MAX_RCV_BYTES 

#define UTAFL_NONE		RMRFL_NONE 
#define UTAFL_AUTO_ALLOC RMRFL_AUTO_ALLOC 

#define UTA_DEF_SIZE	RMRFL_AUTO_ALLOC 

#define UTA_OK			 RMR_OK 
#define UTA_ERR_BADARG	RMR_ERR_BADARG 
#define UTA_ERR_NOENDPT RMR_ERR_NOENDPT 	
#define UTA_ERR_EMPTY	RMR_ERR_EMPTY
#define UTA_ERR_NOHDR	RMR_ERR_NOHDR 
#define UTA_ERR_SENDFAILED RMR_ERR_SENDFAILED 
#define UTA_ERR_CALLFAILED RMR_ERR_CALLFAILED 

#define uta_mbuf_t rmr_mbuf_t

#define uta_alloc_msg  rmr_alloc_msg
#define uta_call rmr_call
#define uta_free_msg rmr_free_msg
#define uta_init rmr_init
#define uta_payload_size  rmr_payload_size
#define uta_send_msg  rmr_send_msg
#define uta_rcv_msg rmr_rcv_msg
#define uta_rcv_specific rmr_rcv_specific
#define uta_rcv_to rmr_rcv_to
#define uta_rts_msg rmr_rts_msg
#define uta_ready rmr_ready
#define uta_send_to rmr_send_to
#endif		// uta compat


#ifdef __cplusplus
 }
#endif

#endif		// dup include prevention

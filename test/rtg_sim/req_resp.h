// :vi ts=4 sw=4 noet:
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

#ifndef _req_resp_h
#define _req_resp_h

/*
	A message buffer that can be used for either req/resp context setup
	or for publishing in a pub/sub setup.
*/
typedef struct rr_mbuf {
	int	used;			// bytes actually used
	int size;			// allocated size of payload
	char*	payload;
} rr_mbuf_t;


/*
	A 'context' to interface with nano; so far very simple, but could
	expand, so we use the struct.
*/
typedef struct rr_ctx {
	int	nn_sock;
} rr_ctx_t;


// ---- prototypes for the rr library ----------------------
// vctx is the pointer returned by the connect or start listening functions
// and is passed to nearly all other functions.

extern void* rr_connect( char* host, char* port );
extern void* rr_start_listening( char* port );
extern rr_mbuf_t* rr_new_buffer(  rr_mbuf_t* mb, int len );
extern void rr_close( void* vctx );
extern void rr_free( void* vctx );
extern void rr_free_mbuf( rr_mbuf_t* mbuf );
extern void*  open_publisher( char*  port );
extern int rr_rcv_to( void* vctx, int time );
extern int rr_send_to( void* vctx, int time );
extern rr_mbuf_t*  rr_receive( void* vctx, rr_mbuf_t* mbuf, int len );
extern rr_mbuf_t* rr_send( void* vctx, rr_mbuf_t* mbuf, int alloc_buf );

#endif

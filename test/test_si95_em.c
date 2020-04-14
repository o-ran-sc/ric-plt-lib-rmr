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
	Mnemonic:	test_si95_em.c
	Abstract:	This supplies a bunch of dummy SI95 functions which emulate
				the sending/receiving of data such that modules can be tested
				without the acutal backing of a network.

				This module must be directly included to be used.
	Date:		20 February 2020
	Author:		E. Scott Daniels
*/


#include "rmr.h" 				// we use some of rmr defs in building dummy messages, so we need these
#include "rmr_agnostic.h"

// ---------------------- emulated SI95 functions ---------------------------


#ifndef _em_si		// this is the same define as the nng emulation code uses to give warning if both included
#define _em_si

#include <arpa/inet.h>
#include <pthread.h>

#include "test_common_em.c"			// common emulation needed for all (epoll, gethostname...)

// --- some globals --------------------------------------------------------
int em_reset_call_flag = 0;			// allows a send to turn off the call flag (see em_disable_call_flg())

// ------------- emulated message header -----------------------------------

/*
	This is a copy from agnostic.h. we need to reset flags in some situations
	so we have to have this, under a different name to avoid disaster.
*/
typedef struct {
    int32_t mtype;                      // message type  ("long" network integer)
    int32_t plen;                       // payload length (sender data length in payload)
    int32_t rmr_ver;                    // our internal message version number
    unsigned char xid[RMR_MAX_XID];     // space for user transaction id or somesuch
    unsigned char sid[RMR_MAX_SID];     // sender ID for return to sender needs
    unsigned char src[RMR_MAX_SRC];     // name:port of the sender (source)
    unsigned char meid[RMR_MAX_MEID];   // managed element id.
    struct timespec ts;                 // timestamp ???

                                        // V2 extension
    int32_t flags;                      // HFL_* constants
    int32_t len0;                       // length of the RMr header data
    int32_t len1;                       // length of the tracing data
    int32_t len2;                       // length of data 1 (d1)
    int32_t len3;                       // length of data 2 (d2)
    int32_t sub_id;                     // subscription id (-1 invalid)

                                        // v3 extension
    unsigned char srcip[RMR_MAX_SRC];   // ip address and port of the source
} em_mhdr_t;

//--------------------------------------------------------------------------
/*
	These are the current references in the RMR code; all others are internal
	to the SI portion of the library

	SIcbreg( ctx->si_ctx, 
	SIwait( ctx->si_ctx );
	SIinitialise( SI_OPT_FG );		// FIX ME: si needs to streamline and drop fork/bg stuff
	SIlistener( ctx->si_ctx, TCP_DEVICE, bind_info )) < 0 ) {
	SItp_stats( ctx->si_ctx );			// dump some interesting stats
	SIclose( ctx->nn_sock );
	SIset_tflags( ctx->si_ctx, SI_TF_FASTACK );
	SIconnect( si_ctx, conn_info )) < 0 ) {
	SIsendt( ctx->si_ctx, nn_sock, msg->tp_buf, tot_len )) != SI_OK ) {
*/

#define SIEM_BLOCKED	18
#define SIEM_ERROR		(-1)
#define SIEM_OK			0

#define SOCKET_TYPE		int		// socket representation is different in each transport

struct ginfo_blk;				// defined in SI things, but must exist here

#include "si95/socket_if.h"		// need to have the si context more than anything else


static void *em_sinew( int type ) {
	return NULL;
}

static char *em_sigetname( int sid ) {
	return "somename";
}

static int em_siaddress( void *src, void **dest, int type ) {
	return 0;
}

static void em_sibldpoll( struct ginfo_blk* gptr ) {
	return;
}

static struct tp_blk *em_siconn_prep( struct ginfo_blk *gptr, int type, char *abuf, int family ) {
	return NULL;
}

/*
	Caller passing a callback funciton for SI to drive; nothing to do.
*/
void *em_cb_data = NULL;
static void em_sicbreg( struct ginfo_blk *gptr, int type, int ((*fptr)()), void * dptr ) {
	if( em_cb_data == NULL ) {
		fprintf( stderr, "<SIEM> calldback dptr %p saved for type %d\n", dptr, type );
		em_cb_data = dptr;
	}
	return;
}

static void em_sicbstat( struct ginfo_blk *gptr, int status, int type ) {
	return;
}

static int em_siclose( struct ginfo_blk *gptr, int fd ) {
	return 0;
}

/*
	If em_send_failures is true, this will fail a small part of the time
	to simualte connection failures.
*/
static int em_next_fd = 0;
static int em_siconnect( struct ginfo_blk *gptr, char *abuf ) {
	static int count = 0;

	if( em_send_failures && (count++ % 15 == 14) ) {
		//fprintf( stderr, "<SIEM> siem is failing connect attempt\n\n" );
		return -1;
	}

	fprintf( stderr, "<SIEM> siem is emulating connect to (%s) attempt return fd=%d\n", abuf, em_next_fd );
	if( em_next_fd < 50 ) {
		em_next_fd++;
	}
	return em_next_fd-1;
}

static struct tp_blk *em_siestablish( int type, char *abuf, int family ) {
	return NULL;
}

static int em_sigenaddr( char *target, int proto, int family, int socktype, struct sockaddr **rap ) {
	return 0;
}

static int em_sigetaddr( struct ginfo_blk *gptr, char *buf ) {
	return 0;
}

static struct tp_blk *em_silisten_prep( struct ginfo_blk *gptr, int type, char* abuf, int family ) {
	return NULL;
}

/*
	Called to open a listen port; returns the port fd or -1 on error.
*/
static int em_silistener( struct ginfo_blk *gptr, int type, char *abuf ) {
	return 100;
}

static void em_simap_fd( struct ginfo_blk *gptr, int fd, struct tp_blk* tpptr ) {
	return;
}

static int em_sinewsession( struct ginfo_blk *gptr, struct tp_blk *tpptr ) {
	return 0;
}

static int em_sipoll( struct ginfo_blk *gptr, int msdelay ) {
	return 0;
}

static int em_sircv( struct ginfo_blk *gptr, int sid, char *buf, int buflen, char *abuf, int delay ) {
	return 0;
}

static void em_sisend( struct ginfo_blk *gptr, struct tp_blk *tpptr ) {
	return;
}

/*
	Calling this function causes the send emulation to turn off the call
	flag in the RMR header. Turning that flag off makes the arriving message
	look as though it might be a response to a call rather than a call itself.
	This is needed since we loop back messages.
*/
static void em_disable_call_flg() {
	em_reset_call_flag = 1;
	fprintf( stderr, "<SIEM> reset call flag setting is: %d\n", em_reset_call_flag );
}

/*
	Opposite of disable_call_flg; the flag is not touched.
*/
static void em_allow_call_flg() {
	em_reset_call_flag = 0;
	fprintf( stderr, "<SIEM> reset call flag setting is: %d\n", em_reset_call_flag );
}

//  callback prototype to drive to simulate 'receive'
static int mt_data_cb( void* datap, int fd, char* buf, int buflen );

/*
	Emulate sending a message. If the global em_send_failures is set,
	then every so often we fail with an EAGAIN to drive that part
	of the code in RMr.

	"Send a message" by passing it to the callback if we have a non-nil cb data pointer.
	We'll divide the data into two to test the concatination of the receiver.
*/
static int em_sisendt( struct ginfo_blk *gptr, int fd, char *ubuf, int ulen ) {
	static int count = 0;
	static int uss = -1;		// ultra short send done

	if( em_send_failures && ((count++ % 15) == 14) ) {
		//fprintf( stderr, "<SIEM> sendt is failing send with blocked/again\n\n" );
		errno = EAGAIN;
		return SIEM_BLOCKED;
	}

	if( em_reset_call_flag ) {		// for call testing we need to flip the flag off to see it "return"
		em_mhdr_t*	hdr;

		hdr = (em_mhdr_t *) (ubuf+50);		// past the transport header bytes
		hdr->flags &= ~HFL_CALL_MSG;		// flip off the call flag
	}

	uss++;

	if( em_cb_data != NULL ) {
		if( uss == 1 ) {					// drive the reconstruction where a split in the msg length happens
			fprintf( stderr, "<SIEM> sendt is queuing ultra short first packet of a two packet sendh len=%d\n", ulen );
			mt_data_cb( em_cb_data, 0, ubuf, 3 );
			mt_data_cb( em_cb_data, 0, ubuf+3, ulen - 3 );
		} else {
			if( ulen > 100 ) {
				fprintf( stderr, "<SIEM> sendt is queuing two packets with the callback len=%d\n", ulen );
				mt_data_cb( em_cb_data, 0, ubuf, 100 );
				mt_data_cb( em_cb_data, 0, ubuf+100, ulen - 100 );
			} else {
				fprintf( stderr, "<SIEM> sendt is queuing one packet with the callback len=%d\n", ulen );
				mt_data_cb( em_cb_data, 0, ubuf, ulen );
			}
		}
	}

	errno = 0;
	//fprintf( stderr, "<SIEM> sendt is returning default reeturn status: %d\n", return_value );
	return return_value;
}

/*
	Sets flags; ignore.
*/
static void em_siset_tflags( struct ginfo_blk *gp, int flags ) {
	return;
}

static int em_sishow_version( ) {
	return 0;
}

static void em_sishutdown( struct ginfo_blk *gptr ) {
	return;
}

/*
	This prints some SI stats -- ignore.
*/
static void em_sitp_stats( void *vgp ) {
	return;
}

static void em_siterm( struct ginfo_blk* gptr, struct tp_blk *tpptr ) {
	return;
}

static void em_sitrash( int type, void *bp ) {
	return;
}

/*
	This will be tricky.  Wait receives raw packets from the network
	and drives the callback(s) which are registered.  We'll need to 
	simulate driving the callback with data that spans multiple FDs
	and has messages split across buffers, but does not block foreaver
	so the test can continue.
	It won't be pretty.

	For now We'll hard code the RMR callback functions and not 
	try to use what it passes via the register function lest we
	need to implement all of SI just to run unit tests.


	Thinking: use rmr's alloc function to alloc a message buffer
	which we fill in.  We can cheat and add the length as RMR does
	on send, and then split the buffer as we feed it back to the 
	callback function.
*/
static int em_siwait( struct ginfo_blk *gptr ) {
	return 0;
}

/*
	The emulation doesn't use the global info stuff, so alloc something
	to generate a pointer.
*/
static struct ginfo_blk *em_siinitialise( int opts ) {
	void *p;

	p = malloc( 1024 );
	return p;
}


// redefine all SI calls to reference functions here.
#define SInew em_sinew
#define sigetname em_sigetname
#define SIaddress em_siaddress
#define SIbldpoll em_sibldpoll
#define SIconn_prep em_siconn_prep
#define SIcbreg em_sicbreg
#define SIcbstat em_sicbstat
//#define SIclose em_siclose
#define SIconnect em_siconnect
#define SIestablish em_siestablish
#define SIgenaddr em_sigenaddr
#define SIgetaddr em_sigetaddr
#define SIlisten_prep em_silisten_prep
#define SIlistener em_silistener
#define SImap_fd em_simap_fd
#define SInewsession em_sinewsession
#define SIpoll em_sipoll
#define SIrcv em_sircv
#define SIsend em_sisend
#define SIsendt em_sisendt
#define SIset_tflags em_siset_tflags
#define SIshow_version em_sishow_version
#define SIshutdown em_sishutdown
#define SItp_stats em_sitp_stats
#define SIterm em_siterm
#define SItrash em_sitrash
#define SIwait em_siwait
#define SIinitialise em_siinitialise


#endif

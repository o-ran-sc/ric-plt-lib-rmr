// :vi sw=4 ts=4 noet:
/*
==================================================================================
	Copyright (c) 2020-2021 Nokia
	Copyright (c) 2020-2021 AT&T Intellectual Property.

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
	Mmemonic:	si95_test.c
	Abstract:	This is the main driver to test the si95 core functions
				(within rmr/src/si/src/si95).

	Author:		E. Scott Daniels
	Date:		6 March 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <strings.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <semaphore.h>


#include <netdb.h>		// these four needed for si address tests
#include <stdio.h>
#include <ctype.h>
#include <netinet/in.h>

#define DEBUG 1

											// specific test tools in this directory
#undef NNG_UNDER_TEST						// NNG is NOT under test so undefine if set
#define NO_EMULATION 1						// no emulation of transport functions
#define NO_PRIVATE_HEADERS 1				// no rmr_si or rmr_nng headers
#define NO_DUMMY_RMR 1						// no msg things

#include "test_support.c"					// things like fail_if()
#include "test_transport_em.c"				// system/transport emulation (open, close, connect, etc)

#include <rmr_logging.h>
#include <logging.c>


// ------------- dummy functions to force edge cases when we can ---------------------------------------

#define SYSTEM_UNDER_TEST	1				// for conditional code

/*
	These are global so they can be reset for individual tests.
*/
static int good_mallocs = 0;		// number of initial good malocs before failurs
static int bad_mallocs = 1;			// number of failed mallocs (consecutive)

static void* test_malloc( size_t n ) {

	fprintf( stderr, ">>>> test malloc: %d %d\n", good_mallocs, bad_mallocs );
	if( good_mallocs ) {
		good_mallocs--;
		return malloc( n );
	}

	if( bad_mallocs ) {
		bad_mallocs--;
		errno = ENOMEM;
		return NULL;
	}

	return malloc( n );
}

// -----------------------------------------------------------------------------------------------------

#include <si95/siaddress.c>
//#include <si95/sialloc.c>
#include <si95/sibldpoll.c>
#include <si95/sicbreg.c>
#include <si95/sicbstat.c>
#include <si95/siclose.c>
#include <si95/siconnect.c>
#include <si95/siestablish.c>
#include <si95/sigetadd.c>
#include <si95/sigetname.c>
#include <si95/siinit.c>
#include <si95/silisten.c>
#include <si95/sinew.c>
#include <si95/sinewses.c>
#include <si95/sipoll.c>
//#include <si95/sircv.c>
#include <si95/sisend.c>
#include <si95/sisendt.c>
#include <si95/sishutdown.c>
#include <si95/siterm.c>
#include <si95/sitrash.c>
#define malloc test_malloc
#include <si95/siwait.c>
#undef malloc

// ---------------------------------------------------------------------

void*	si_ctx = NULL;			// a global context might be useful

// ---------------------------------------------------------------------

/*
	Fake callback to register.
*/
static int test_cb( void* data ) {
	return 0;
}

/*
	Returns error for coverage testing of CB calls
*/
static int test_cb_err( void* data ) {
	return -1;
}

/*
	Memory allocation/free related tests
*/
static int memory( ) {
	int		errors = 0;
	void*	ptr;
	void*	iptr;

	// ---- SInew ----------------
	ptr = SInew( 100 );				// invalid block type should return nil
	errors += fail_not_nil( ptr, "memory: sinew did not return nil when given a valid struct type" );
	SItrash( 100, NULL );				// drive trash for coverage

	iptr = SInew( IOQ_BLK );
	errors += fail_if_nil( iptr, "memory: sinew returned nil when given ioq request" );
	SItrash(  IOQ_BLK, iptr );

	ptr = SInew( TP_BLK );
	errors += fail_if_nil( ptr, "memory: sinew returned nil when given tpblk request" );
	if( ptr ) {
		iptr = SInew( IOQ_BLK );
		((struct tp_blk *)ptr)->squeue = iptr;
		SItrash(  TP_BLK, ptr );
	}

	ptr = SInew( GI_BLK );
	errors += fail_if_nil( ptr, "memory: sinew returned nil when given giblk request" );
	SItrash(  GI_BLK, ptr );		// GI block cannot be trashed, ensure this (valgind will complain about a leak)
	free( ptr );					// we can free GI block only in tests

	fprintf( stderr, "<INFO> memory module finished with %d errors\n", errors );
	return errors;
}

/*
	Test initialisation related things
*/
static int init() {
	int		errors = 0;

	si_ctx = SIinitialise( 0 );
	errors += fail_if_nil( si_ctx, "init: siinit returned a nil pointer" );

	SIclr_tflags( si_ctx, 0x00 );		// drive for coverage; no return value from these
	SIset_tflags( si_ctx, 0x03 );

	fprintf( stderr, "<INFO> init  module finished with %d errors\n", errors );
	return errors;
}

static int cleanup() {
	int errors = 0;

	if( ! si_ctx ) {
		fprintf( stderr, "<INFO> cleanup has no context to use\n" );
		return 0;
	}

	fprintf( stderr, "<INFO> cleanup running\n" );
	SIcbstat( si_ctx, SI_RET_UNREG, SI_CB_SECURITY );
	SIcbstat( si_ctx, SI_RET_QUIT, SI_CB_SECURITY );

	SItp_stats( si_ctx );		// drive for coverage only
	SItp_stats( NULL );

	SIconnect( si_ctx, "127.0.0.1:43086" );	// ensure context has a tp block to free on shutdown
	SIshutdown( NULL );
	SIabort( si_ctx );

	// cleaning up the remaining global resources
	struct ginfo_blk *gptr = (struct ginfo_blk*)si_ctx;
	SItrash( TP_BLK, gptr->tplist );
	free( gptr->tp_map );
	free( gptr->rbuf );
	free( gptr->cbtab );
	free( si_ctx );

	fprintf( stderr, "<INFO> cleanup  module finished with %d errors\n", errors );
	return errors;
}

/*
	Address related tests.
*/
static int addr() {
	int errors = 0;
	int l;
	char buf1[4096];			// space to build buffers for xlation
	char*	hr_addr;			// human readable address returned
	void* net_addr;				// a network address block of some type

/*
	struct sockaddr* addr;
	addr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );

	l = SIgenaddr( "    [ff02::4]:4567", PF_INET6, IPPROTO_TCP, SOCK_STREAM, &addr );

	SIgenaddr( "    [ff02::4]:4567", PF_INET6, IPPROTO_TCP, SOCK_STREAM, &addr );
*/

	l = SIaddress( NULL, NULL, 0 );
	errors += fail_if_true( l != 0, "SIaddress given two null pointers didn't return 0 len" );
	l = SIaddress( buf1, NULL, 0 );
	errors += fail_if_true( l != 0, "SIaddress given null dest pointer didn't return 0 len" );
	l = SIaddress( NULL, (void *) &buf1, 0 );
	errors += fail_if_true( l != 0, "SIaddress given null src pointer didn't return 0 len" );

	net_addr = NULL;
	snprintf( buf1, sizeof( buf1 ), "   [ff02::5:4001" );		// invalid address, drive leading space eater too
	l = SIaddress( buf1, (void **)  &net_addr, AC_TOADDR6 );
	errors += fail_if_true( l > 0, "to addr6 with bad addr convdersion returned valid len" );
	free( net_addr );

	snprintf( buf1, sizeof( buf1 ), "[ff02::5]:4002" );		// v6 might not be supported so failure is OK here; driving for coverage
	l = SIaddress( buf1, &net_addr, AC_TOADDR6 );
	if( l > 0 ) {
		l = SIaddress( net_addr, (void *) &hr_addr, AC_TODOT );						// convert the address back to hr string
		errors += fail_if_true( l < 1, "v6 to dot conversion failed" );
		errors += fail_if_nil( hr_addr, "v6 to dot conversion yields a nil pointer" );
		free( net_addr );
		free( hr_addr );
	}

	snprintf( buf1, sizeof( buf1 ), "localhost:43086" );
	l = SIaddress( buf1, (void **) &net_addr, AC_TOADDR );
	errors += fail_if_true( l < 1, "v4 to addr conversion failed" );

	l = SIaddress( net_addr, (void *) &hr_addr, AC_TODOT );						// convert the address back to hr string
	errors += fail_if_true( l < 1, "to dot convdersion failed" );
	errors += fail_if_nil( hr_addr, "v4 to dot conversion yields a nil pointer" );
	free( net_addr );
	free( hr_addr );

	fprintf( stderr, "<INFO> addr module finished with %d errors\n", errors );
	return errors;
}

/*
	Prep related tests. These mostly drive cases that aren't driven by "normal"
	connect, send, receive tests (e.g. UDP branches).
*/
static int prep() {
	int		errors = 0;
	void*	thing;					// the thing that should be returned

	thing = SIlisten_prep( UDP_DEVICE, "localhost:1234", AF_INET );
	errors += fail_if_nil( thing, "listen prep udp returned nil block" );
	SItrash( TP_BLK, thing );

	thing = SIlisten_prep( UDP_DEVICE, "localhost:1234", 84306 );		// this should fail
	errors += fail_not_nil( thing, "listen prep udp returned valid block ptr for bogus family" );

	thing = SIconn_prep( si_ctx, UDP_DEVICE, "localhost:1234", 84306 );		// again, expect to fail; bogus family
	errors += fail_not_nil( thing, "conn prep udp returned valid block ptr for bogus family" );

	return errors;
}

/*
	Polling/waiting tests.  These are difficult at best because of the blocking
	nature of things, not to mention needing to have real ports open etc.
*/
static int poll() {
	int errors  = 0;
	int status;
	struct ginfo_blk* dummy;


	dummy = SIinitialise( 0 );				// get one to fiddle to drive edge cases
	dummy->flags |= GIF_SHUTDOWN;			// shutdown edge condition
	SIpoll( dummy, 1 );

	free( dummy->tp_map );
	free( dummy->rbuf );
	free( dummy->cbtab );

	memset( dummy, 0, sizeof( *dummy ) );	// force bad cookie check code to drive
	SIpoll( dummy, 1 );

	free (dummy );

	status = SIpoll( si_ctx, 1 );
	errors += fail_if_true( status != 0, "poll failed" );

	return errors;
}


/*
	Connection oriented tests.
*/
static int conn( ) {
	int errors = 0;
	int state;
	int cfd = 3;					// fd for close
	char*	buf;

	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state < 0, "connect to low port failed" );

	state = SIconnect( si_ctx, "localhost:43086" );		// drive save connect with good return code
	errors += fail_if_true( state < 0, "connect to high port failed" );

	tpem_set_addr_dup_state( 1 );						// force get sockket name emulation to return a duplicate address
	state = SIconnect( si_ctx, "localhost:43086" );		// drive save connect with good return code
	errors += fail_if_true( state >= 0, "forced dup connect did not return error" );

	tpem_set_addr_dup_state( 0 );						// back to normal
	tpem_set_conn_state( -1 );
	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state >= 0, "connect to low port successful when failure expected" );
	tpem_set_conn_state( 3 );

	tpem_set_sock_state( 1 );							// make scoket calls fail
	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state >= 0, "connect to low port successful when socket based failure expected" );

	tpem_set_sock_state( 0 );

	state = SIlistener( si_ctx, TCP_DEVICE, "0.0.0.0:4567" );
	errors += fail_if_true( state < 0, "listen failed" );

	tpem_set_bind_state( 1 );
	state = SIlistener( si_ctx, TCP_DEVICE, "0.0.0.0:4567" );
	errors += fail_if_true( state >= 0, "listen successful when bind error set" );
	tpem_set_bind_state( 0 );

	SIbldpoll( si_ctx );		// for coverage. no return value and nothing we can check

	state = SIclose( NULL, 0 );			//coverage
	errors += fail_if_true( state != SI_ERROR, "close given nil context returned success" );

	state = SIclose( si_ctx, cfd );
	errors += fail_if_true( state == SI_ERROR, "close given good context and good fd returned error" );

	state = SIclose( si_ctx, 5000 );						// out of range fd
	errors += fail_if_true( state != SI_ERROR, "close given good context and bad fd returned success" );

	state = SIclose( si_ctx, TCP_LISTEN_PORT );				// close listener
	errors += fail_if_true( state == SI_ERROR, "close given good context and listener fd returned error" );

	state = SIclose( si_ctx, UDP_PORT );					// close first open udp port (should not be there)
	errors += fail_if_true( state != SI_ERROR, "close given good context and udp generic fd returned error" );

	buf = SIgetname( 3 );
	if( fail_if_true( buf == NULL, "get name failed to return a buffer" ) ) {
		errors++;
	} else {
		errors += fail_if_true( buf[0] == 0, "get name returned buf with emtpy string" );
		free( buf );
	}

	buf = SIgetname( -1 );			// invalid fd
	errors += fail_not_nil( buf, "get name returned buf with non-emtpy string when given bad fd" );

	fprintf( stderr, "<INFO> conn module finished with %d errors\n", errors );
	return errors;
}

/*
	Misc tests that just don't fit in another bucket.
*/
static int misc( ) {
	int errors = 0;
	char	buf[1024];

	SIcbreg( NULL, SI_CB_SECURITY, test_cb, NULL );		// coverage only, no return value no verification
	SIcbreg( si_ctx, SI_CB_SECURITY, test_cb, NULL );

	buf[0] = 0;
	SIgetaddr( si_ctx, buf );
	errors += fail_if_true( buf[0] == 0, "get address failed" );
	fprintf( stderr, "<INFO> get address returns (%s)\n", buf );

	fprintf( stderr, "<INFO> misc module finished with %d errors\n", errors );
	return errors;
}


/*
	New session (accept) testing.
*/
static int new_sess( ) {
	int errors = 0;
	char	buf[1024];
	struct tp_blk *tpptr;
	int		status;

	tpptr = SInew( TP_BLK );
	tpptr->fd = 3;
	tpptr->flags |= TPF_LISTENFD;

	tpem_set_accept_fd( -1 );									// accept will "fail" for coverage
	status = SInewsession( si_ctx, tpptr );
	errors += fail_if_true( status != SI_ERROR, "newsession did not fail when accept fails" );

	tpem_set_accept_fd( 5 );									// accept will return a good fd
	SIcbreg( si_ctx, SI_CB_SECURITY, test_cb_err, NULL );		// register error and drive new session for error coverage
	status = SInewsession( si_ctx, tpptr );
	errors += fail_if_true( status >= 0, "newsession did failed when accept was good" );

	tpem_set_accept_fd( 6 );									// accept will return a good fd
	SIset_tflags( si_ctx, SI_TF_NODELAY | SI_TF_FASTACK );		// flip options for coverage in new sess
	SIcbreg( si_ctx, SI_CB_CONN, test_cb, NULL );				// drive connection for coverage
	SIcbreg( si_ctx, SI_CB_SECURITY, test_cb, NULL );
	status = SInewsession( si_ctx, tpptr );
	errors += fail_if_true( status < 0, "newsession did failed when accept was good" );

	free( tpptr );

	fprintf( stderr, "<INFO> new_sess module finished with %d errors\n", errors );
	return errors;
}

/*
	Send tests
*/
static int send_tests( ) {
	int		errors = 0;
	char	buf[1024];
	int		len;
	int		state;

	len = snprintf( buf, 100, "Heaven knows I'm miserable now!" );

	state = SIsendt( si_ctx, 9999, buf, len );
	errors += fail_if_true( state >= 0, "send given fd out of range did not fail" );

	state = SIsendt( si_ctx, -1, buf, len );
	errors += fail_if_true( state >= 0, "send given neg fd did not fail" );

	SIsendt( si_ctx, 6, buf, len );

	tpem_set_send_err( 99 );
	SIsendt( si_ctx, 6, buf, len );

	tpem_set_send_err( 0 );
	tpem_set_sel_blk( 1 );
	SIsendt( si_ctx, 6, buf, len );

	tpem_set_sel_blk( 0 );
	tpem_set_selef_fd( 6 );						// will cause send to fail and fd6 to close
	SIsendt( si_ctx, 6, buf, len );

	return errors;
}


/*
	Wait testing.  This is tricky because we don't have any sessions and thus it's difficult
	to drive much of SIwait().
*/
static int wait_tests() {
	int errors = 0;
	struct ginfo_blk* dummy;


	dummy = SIinitialise( 0 );				// get one to fiddle to drive edge cases
	SIwait( dummy );						// malloc should "fail"

	dummy->flags |= GIF_SHUTDOWN;
	SIwait( dummy );

	free( dummy->tp_map );
	free( dummy->rbuf );
	free( dummy->cbtab );

	memset( dummy, 0, sizeof( *dummy ) );	// force bad cookie check code to drive
	SIwait( dummy );

	free( dummy );


	SIwait( si_ctx );						// should drive once through the loop

	return errors;
}

// ----------------------------------------------------------------------------------------

/*
	Drive tests...
*/
int main() {
	int errors = 0;

	rmr_set_vlevel( 5 );			// enable all debugging

	fprintf( stderr, "\n<INFO> starting SI95 tests\n" );

	errors += init();
	errors += memory();
	errors += addr();
	errors += prep();
	errors += conn();
	errors += misc();

	errors += new_sess();		// should leave a "connected" session at fd == 6
	errors += send_tests();

	errors += poll();
	errors += wait_tests();

	errors += cleanup();

	test_summary( errors, "SI95 tests" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in SI95 core code\n\n", errors );
	}

	return !!errors;
}

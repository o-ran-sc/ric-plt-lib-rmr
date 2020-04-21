// :vi sw=4 ts=4 noet:
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
#include <errno.h>
#include <pthread.h>
#include <ctype.h>

#include <netdb.h>		// these four needed for si address tests
#include <stdio.h>
#include <ctype.h>
#include <netinet/in.h>



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <semaphore.h>

#define DEBUG 1

											// specific test tools in this directory
#undef NNG_UNDER_TEST						// NNG is NOT under test so undefine if set
#define NO_EMULATION 1						// no emulation of transport functions
#define NO_PRIVATE_HEADERS 1				// no rmr_si or rmr_nng headers
#define NO_DUMMY_RMR 1						// no msg things

#include "test_support.c"					// things like fail_if()
#include "test_transport_em.c"				// system/transport emulation (open, close, connect, etc)

/*
#include "rmr.h"					// things the users see
#include "rmr_symtab.h"
#include "rmr_agnostic.h"			// transport agnostic header
*/
#include <rmr_logging.h>
#include <logging.c>

#include <si95/siaddress.c>
//#include <si95/sialloc.c>
//#include <si95/sibldpoll.c>
//#include <si95/sicbreg.c>
//#include <si95/sicbstat.c>
//#include <si95/siclose.c>
#include <si95/siconnect.c>
#include <si95/siestablish.c>
//#include <si95/sigetadd.c>
//#include <si95/sigetname.c>
#include <si95/siinit.c>
#include <si95/silisten.c>
#include <si95/sinew.c>
//#include <si95/sinewses.c>
//#include <si95/sipoll.c>
//#include <si95/sircv.c>
//#include <si95/sisend.c>
//#include <si95/sisendt.c>
#include <si95/sishutdown.c>
#include <si95/siterm.c>
#include <si95/sitrash.c>
//#include <si95/siwait.c>

// ---------------------------------------------------------------------

void*	si_ctx = NULL;			// a global context might be useful

// ---------------------------------------------------------------------

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
		return 0;
	}

	SItp_stats( si_ctx );		// drive for coverage only
	SItp_stats( NULL );

	SIconnect( si_ctx, "localhost:43086" );	// ensure context has a tp block to free on shutdown
	SIshutdown( NULL );
	SIabort( si_ctx );


	fprintf( stderr, "<INFO> cleanup  module finished with %d errors\n", errors );
	return errors;
}

/*
	Address related tests.
*/
static int addr() {
	int errors = 0;
	int l;
	struct sockaddr* addr;
	char buf1[4096];
	char buf2[4096];
	char* dest;

	addr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
/*
	l = SIgenaddr( "    [ff02::4]:4567", PF_INET6, IPPROTO_TCP, SOCK_STREAM, &addr );

	SIgenaddr( "    [ff02::4]:4567", PF_INET6, IPPROTO_TCP, SOCK_STREAM, &addr );
*/

	dest = NULL;
	snprintf( buf1, sizeof( buf1 ), "   [ff02::5:4001" );		// invalid address, drive leading space eater too
	l = SIaddress( buf1, (void **)  &dest, AC_TOADDR6 );
	errors += fail_if_true( l > 0, "to addr6 with bad addr convdersion returned valid len" );

	snprintf( buf1, sizeof( buf1 ), "[ff02::5]:4002" );		// v6 might not be supported so failure is OK here; driving for coverage
	l=SIaddress( buf1, (void **) &dest, AC_TOADDR6 );

	snprintf( buf1, sizeof( buf1 ), "localhost:43086" );
	l = SIaddress( buf1, (void **) &dest, AC_TOADDR );
	errors += fail_if_true( l < 1, "to addr convdersion failed" );

	snprintf( buf1, sizeof( buf1 ), "localhost:4004" );
	l = SIaddress( buf1, (void **) &dest, AC_TODOT );
	errors += fail_if_true( l < 1, "to dot convdersion failed" );

	fprintf( stderr, "<INFO> addr module finished with %d errors\n", errors );
	return errors;

}


/*
	Connection oriented tests.
*/
static int conn( ) {
	int errors = 0;
	int state;

	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state < 0, "connect to low port failed" );

	state = SIconnect( si_ctx, "localhost:43086" );		// drive save connect with good return code
	errors += fail_if_true( state < 0, "connect to high port failed" );

	tpem_set_addr_dup_state( 1 );				// force get sockket name emulation to return a duplicate address
	state = SIconnect( si_ctx, "localhost:43086" );		// drive save connect with good return code
	errors += fail_if_true( state >= 0, "forced dup connect did not return error" );

	tpem_set_addr_dup_state( 0 );				// force get sockket name emulation to return a duplicate address
	tpem_set_conn_state( 1 );
	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state >= 0, "connect to low port successful when failure expected" );

	tpem_set_sock_state( 1 );		// make scoket calls fail
	state = SIconnect( si_ctx, "localhost:4567" );		// driver regular connect
	errors += fail_if_true( state >= 0, "connect to low port successful when socket based failure expected" );

	tpem_set_sock_state( 0 );

	state = SIlistener( si_ctx, TCP_DEVICE, "0.0.0.0:4567" );
	errors += fail_if_true( state < 0, "listen failed" );

	tpem_set_bind_state( 1 );
	state = SIlistener( si_ctx, TCP_DEVICE, "0.0.0.0:4567" );
	errors += fail_if_true( state >= 0, "listen successful when bind error set" );
	tpem_set_bind_state( 0 );


	fprintf( stderr, "<INFO> conn module finished with %d errors\n", errors );
	return errors;
}

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
	errors += conn();
	errors += cleanup();

	fprintf( stderr, "<INFO> testing finished\n" );
	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in SI95 core code\n\n", errors );
	}

	return !!errors;
}

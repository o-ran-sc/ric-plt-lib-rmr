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
#undef NNG_UNDER_TEST 						// NNG is NOT under test so undefine if set
#define NO_EMULATION 1						// no emulation of transport functions
#define NO_PRIVATE_HEADERS 1				// no rmr_si or rmr_nng headers
#define NO_DUMMY_RMR 1						// no msg things
#include "test_support.c"					// things like fail_if()


/*
#include "rmr.h"					// things the users see
#include "rmr_symtab.h"
#include "rmr_agnostic.h"			// transport agnostic header
*/
#include <rmr_logging.h>
#include <logging.c>

//#include <si95/siaddress.c>
//#include <si95/sialloc.c>
//#include <si95/sibldpoll.c>
//#include <si95/sicbreg.c>
//#include <si95/sicbstat.c>
//#include <si95/siclose.c>
//#include <si95/siconnect.c>
//#include <si95/siestablish.c>
//#include <si95/sigetadd.c>
//#include <si95/sigetname.c>
#include <si95/siinit.c>
//#include <si95/silisten.c>
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
	iptr = SInew( IOQ_BLK );
	SItrash(  IOQ_BLK, iptr );

	ptr = SInew( TP_BLK );
	errors += fail_if_nil( ptr, "memory: sinew returned nil when given tpblk request" );
	if( ptr ) {
		((struct tp_blk *)ptr)->squeue = iptr;
		SItrash(  TP_BLK, ptr );
	}
	
	ptr = SInew( GI_BLK );
	errors += fail_if_nil( ptr, "memory: sinew returned nil when given giblk request" );
	SItrash(  GI_BLK, ptr );


	return errors;
}

void*	si_ctx = NULL;			// a global context might be useful

/*
	Test initialisation related things
*/
static int init() {
	int		errors = 0;

	si_ctx = SIinitialise( 0 );
	errors += fail_if_nil( si_ctx, "init: siinit returned a nil pointer" );

	SIclr_tflags( si_ctx, 0x00 );		// drive for coverage; no return value from these
	SIset_tflags( si_ctx, 0x03 );

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

	if( errors == 0 ) {
		fprintf( stderr, "<PASS> all tests were OK\n\n" );
	} else {
		fprintf( stderr, "<FAIL> %d errors in SI95 core code\n\n", errors );
	}

	return !!errors;
}

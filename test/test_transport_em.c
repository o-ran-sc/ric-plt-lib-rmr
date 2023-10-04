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
	Mnemonic:	test_transport.c
	Abstract:	This supplies a bunch of dummy system functions which emulate
				things like connect() bind() etc.

				This module must be directly included to be used.
	Date:		17 April 2020
	Author:		E. Scott Daniels
*/

#ifndef _test_transport_c
#define _sitransport_h			// prevent the transport defs when including SI95


char	tpem_last_addr[1024];		// last address to simulate connection to ourself
int		tpem_last_len = 0;

int tpem_addr_dup = 0;			// getsockname duplicates last addr if true
int tpem_conn_state = 0;		// states returned by emulated functions allowing failures to be driven
int tpem_sock_state = 0;		// if sock state 0, then socket call returns good fd
int tpem_listen_state = 0;
int tpem_bind_state = 0;
int tpem_accept_fd = 5;			// file desc returned by accept
int tpem_sel_ef = -1;			// select sets this fd's error if >= 0
int tpem_sel_block = 0;			// set if select call inidcates would block
int	tpem_send_err = 0;			// set to cause send to return error

// ------------ emulation control -------------------------------------------

/*
	Allow test prog to set various things
*/
static void tpem_set_conn_state( int s ) {
	tpem_conn_state = s;
}

static void tpem_set_addr_dup_state( int s ) {
	tpem_addr_dup = s;
}

static void tpem_set_sock_state( int s ) {
	tpem_sock_state = s;
}

static void tpem_set_bind_state( int s ) {
	tpem_bind_state = s;
}

static void tpem_set_accept_fd( int s ) {
	tpem_accept_fd = s;
}

static void tpem_set_selef_fd( int s ) {
	tpem_sel_ef = s;
}

static void tpem_set_sel_blk( int s ) {
	tpem_sel_block = s;
}

static void tpem_set_send_err( int s ) {
	tpem_send_err = s;
}

// ---- emulated functions ---------------------------------------------------

static int tpem_bind( int socket, struct sockaddr* addr, socklen_t alen ) {
	return tpem_bind_state;
}

static int tpem_connect( int socket, struct sockaddr* addr, socklen_t alen ) {
	memcpy( tpem_last_addr, addr, alen );
	tpem_last_len = alen;
	fprintf( stderr, "<SYSEM> connection simulated rc=%d\n", tpem_conn_state );
	return tpem_conn_state;
}

/*
	This gets the last address connected to if dup is true; else returns 0s
	which should be enough to test that connection didn't loop back to us.
*/
static int tpem_getsockname( int socket, struct sockaddr* address, socklen_t* alen ) {
	int clen;		// copy len

	if( tpem_last_len > 0 ) {
		clen = tpem_last_len > *alen ? *alen : tpem_last_len;
		if( tpem_addr_dup ) {
			memcpy( address, tpem_last_addr, clen );
		} else {
			memset( address, 0, clen );
		}
		*alen = clen;
	} else {
		memset( address, 0, *alen );
	}

	return 0;
}

static int tpem_listen( int socket, int backlog ) {
	return tpem_listen_state;
}

static int tpem_socket( int domain, int type, int protocol ) {
	static int fd = 1;

	if( tpem_sock_state == 0 ) {
		if( ++fd > 10 ) {
			fd = 3;				// ensure we don't stomp on std* descriptors
		}

		return fd;
	}

	return -1;
}

static int tpem_accept( int socket, struct sockaddr *restrict address, socklen_t *restrict address_len) {
	return tpem_accept_fd;
}

/*
	Emulate a select. If tpem_sel_ef is set, then the error fd set for the fd is set to true.
	If sel_woudl_block is set, then the select returns blocking
*/
static int tpem_select( int fd_count, fd_set* rf, fd_set* wf, fd_set* ef, void* time ) {
	fprintf( stderr, "<SYSTEM> select returns %d (1==no-block)\n", tpem_sel_block ? -1 : 1  );

	if( tpem_sel_block ) {
		return -1;
	}

	if( tpem_sel_ef >= 0 ) {
		FD_SET( tpem_sel_ef, ef );
	} else {
		FD_ZERO( ef );
	}

	return 1;
}

/*
	If tpem_send_err is set, we return less than count;
*/
static int tpem_send( int fd, void* buf, int count, int flags ) {
	errno = tpem_send_err;

	fprintf( stderr, "<SYSTEM> send on fd=%d for %d bytes ret=%d\n", fd, count, tpem_send_err ? -1 : count );
	return tpem_send_err ? -1 : count;
}


// ---------------------------------------------------------------------------------------

/*
	redefine all system calls to reference functions here. There are two defs
	SI functions should use the capitalised verision so that sliding ff under
	it is possible. There might be instances wehre the actual system call is
	needed, so we also define the lowercase value.
*/
#define BIND tpem_bind
#define bind tpem_bind
#define CONNECT tpem_connect
#define connect tpem_connect
#define getsockname tpem_getsockname
#define SOCKET tpem_socket
#define socket tpem_socket
#define LISTEN tpem_listen
#define listen tpem_listen
#define accept tpem_accept
#define ACCEPT tpem_accept
#define SEND	tpem_send
#define SELECT	tpem_select
#define select	tpem_select

/*
	these are defined in SI so that we can use the system stack or FFstack
	they must exist and reference system calls if not defined above.
*/
#define CLOSE		close
#define SHUTDOWN	shutdown
#define	GETSOCKOPT	getsockopt
#define SETSOCKOPT	setsockopt
#define READ		read
#define WRITE		write
#define SENDTO		sendto
#define RECV		recv
#define RECVFROM	recvfrom
#define RECVMSG		recvmsg



#endif


// :vi sw=4 ts=4 noet:
/*
==================================================================================
	Copyright (c) 2019-2021 Nokia
	Copyright (c) 2018-2021 AT&T Intellectual Property.

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
	Mnemonic:	tools_static.c
	Abstract:	A small set of very simple tools to support Uta == RMR.
					uta_tokenise -- simple string tokeniser
					uta_rmip_tokenise -- tokenise and remove ip addresses from the list
					uta_h2ip	-- look up host name and return an ip address
					uta_lookup_rtg	-- looks in env for rtg host:port
					uta_has_str	-- searches buffer of tokens for a string

					uta_link2	-- establish a nanomsg connection to a host

					uta_dump_env -- dump the environment variables to stdout that are
								of importance to RMR.

	Author:		E. Scott Daniels
	Date:		30 November 2018
*/

#ifndef _tools_static_c
#define _tools_static_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>

#include <sys/types.h>		// these are needed to suss out ip addresses from interfaces
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

// --- some protos needed for better organisation --------
static int is_this_myip( if_addrs_t* l, char* addr );


// ----------------------------------------------------------------------------------


/*
	A strncpy() replacement that ensures the resulting dest buffer has
	a zero (nil) terminator even if the source is longer than the dest
	length.
	A max of len-1 bytes are copied from src to dest. The copy stops when
	a zero (nil) is encountered in the src, or len-1 bytes are copied.
	The string is always nil terminated.
	The string length is returned.

	It is the responsiblity of the caller to ensure that dest is at
	least len bytes in length.

	If either src/dest is invalid (nil) a value of -1 is returned.
*/
static inline int zt_buf_fill( char* dest, char const* src, int len ) {
	char*		dp;
	char const*	sp;
	int			n;		// num moved

	if( dest == NULL || src == NULL ) {
		return -1;
	}

	dp = dest;
	sp = src;
	n = 0;
	while(  *sp && n < len-1 ) {
		*(dp++) = *(sp++);
		n++;
	}

	*dp = 0;
	return n;
}

/*
	Simple tokeniser. Split a null terminated string into tokens recording the
	pointers in the tokens array provided.  Tokens MUST be large enough. Max is
	the max number of tokens to split into.  Returns the actual number of tokens
	recorded in the pointer array.

	CAUTION: this modifies the string passed in!!
*/
static int uta_tokenise( char* buf, char** tokens, int max, char sep ) {
	char* end;					// end of token
	int	n = 0;

	if( !buf || ! tokens || !(*buf) ) {
		return 0;
	}

	tokens[n++] = buf;
	end = buf;
	while( n < max && *end && (end = strchr( end, sep )) != NULL ) {
		*end = 0;
		tokens[n++] = ++end;
	}

	return n;
}

/*
	Tokenise and remove matches.
	Given a buffer of 'sep' separated tokens, and a list of things,
	return up to max tokens with any tokens that matched things in
	the list. Toks is the user supplied array of char* which we will
	fill in (up to max) with pointers to tokens from buf.  This
	damages buf, so the caller must dup the string if it must be
	preserved for later, original, use.  The pointers returned in
	toks will reference portions of bufs.

	Returns the number of tokens referenced by toks.
*/
static int uta_rmip_tokenise( char* buf, if_addrs_t* iplist, char** toks, int max, char sep ) {
	int 	ntoks = 0;			// total toks in the original buffer
	int		pcount = 0;			// count after prune
	char**	all_toks;
	int i;
	int j;


	all_toks = malloc( sizeof( char * ) * max );					// refernce to all tokens; we'll prune
	pcount = ntoks = uta_tokenise( buf, all_toks, max, sep );		// split them up
	j = 0;
	if( ntoks > 0 ) {
		for( i = 0; i < ntoks; i++ ) {
			if( is_this_myip( iplist, all_toks[i] ) ) {
				pcount--;									// ours, prune
			} else {
				toks[j++] = all_toks[i];					// not one of ours, keep it
			}
		}
	}

	free( all_toks );
	return pcount;
}

/*
	Xlate hostname (expected to be name:port) to an IP address that nano will tolerate.
	We'll use the first address from the list to keep it simple. If the first character
	of the name is a digit, we assume it's really an IP address and just return that.

	Return is a string which the caller must free. Even if the string passed in is already
	an IP address, a duplicate will be returend so that it can always be freed.
	On error a nil pointer is returned.
*/
static char* uta_h2ip( char const* hname ) {
	char			buf[120];
	struct hostent* hent;
	unsigned int	octs[4];
	unsigned int	a;
	int				i;
	char*			tok;
	char*			dname;		// duplicated name for distruction

	dname = strdup( hname );

	if( isdigit( *dname ) || *dname == '[' ) {		// hostnames can't start with digit, or ipv6 [; assume ip address
		return dname;
	}

	if( (tok = strchr( dname, ':' )) != NULL ) {
		*(tok++) = 0;
	}

	hent = gethostbyname( dname );			// valgrind will complain that this leaks, but we cannot free it!
	if( hent == NULL || hent->h_addr_list == NULL ) {
		//rmr_vlog( RMR_VL_WARN, "h2ip: dns lookup failed for: %s\n", dname );
		free( dname );
		return NULL;
	}

	a = ntohl( *((unsigned int *)hent->h_addr_list[0]) );
	for( i = 3; i >= 0; i-- ) {
		octs[i] = a & 0xff;
		a = a >> 8;
	}

	if( tok ) {							// if :port was given, smash it back on
		snprintf( buf, sizeof( buf ), "%d.%d.%d.%d:%s", octs[0], octs[1], octs[2], octs[3], tok );
	} else {
		snprintf( buf, sizeof( buf ), "%d.%d.%d.%d", octs[0], octs[1], octs[2], octs[3] );
	}

	free( dname );
	return strdup( buf );
}


#ifdef RTG_PUB
/*
	Looks for the environment variable RMR_RTG_SVC which we assume to be name[:port], and
	does a dns lookup on the name. If the env does not have such a variable, we default to
	"rtg" and a port of 5656.

	Returns true (1) if lookup found something;

	CAUTION:  this is ONLY used if the RTG is a pub and we are using pub/sub to get updates.
			There are issues with some underlying transport pub/sub implementations so this
			is likley NOT needed/used.
*/
static int uta_lookup_rtg( uta_ctx_t* ctx ) {
	char*	ev;					// pointer to the env value
	char*	def_port = "5656";
	char*	port = NULL;
	char*	dstr = NULL;

	if( ctx == NULL ) {
		return 0;
	}


	if( ctx->rtg_addr ) {
		free( ctx->rtg_addr );
	}

	if( (ev = getenv( "RMR_RTG_SVC" )) == NULL ) {
		ev = "rtg";
		port = def_port;
	} else {
		dstr = strdup( ev );			// copy so we can trash it
		if( (port = strchr( dstr, ':' )) == NULL ) {
			port = def_port;
		} else {
			*port = 0;
			port++;						// point at the first digit
		}
		ev = dstr;						// all references below assume ev
	}

	ctx->rtg_addr = uta_h2ip( ev );		// convert name to IP addr
	ctx->rtg_port = atoi( port );
	if( dstr ) {
		free( dstr );
	}

	return ctx->rtg_addr != NULL;
}
#endif


/*
	Expects a buffer of 'sep' separated tokens and looks to see if
	the given string is one of those tokens. Returns the token
	index (0 - n-1) if the string is found; -1 otherwise. The max
	parameter supplies the maximum number of tokens to search in
	the buffer.

	On failure (-1) errno will be set in cases where memory cannot
	be alocated (is this even possible any more?). If errno is 0
	and failure is returned, then the caller should assume that
	the token isn't in the list, or the list had no elements.
*/
static int uta_has_str( char const* buf, char const* str, char sep, int max ) {
	char*	dbuf;			// duplicated buf so we can trash
	char** tokens;			// pointer to tokens from the string
	int		ntokens;		// number of tokens buf split into
	int		i;
	int		rc;				// return code

	if( max < 2 ) {
		return -1;
	}

	dbuf = strdup( buf );
	if( dbuf == NULL  ) {
		errno = ENOMEM;
		return -1;
	}

	if( (tokens = (char **) malloc( sizeof( char * ) * max )) == NULL ) {
		errno = ENOMEM;
		free( dbuf );
		return -1;
	}

	ntokens = uta_tokenise( dbuf, tokens, max, sep );
	errno = 0;
	rc = -1;
	for( i = 0; rc < 0 && i < ntokens; i++ ) {
		if( tokens[i] ) {
			if( strcmp( tokens[i], str ) == 0 ) {
				rc = i;
			}
		}
	}

	free( dbuf );
	free( tokens );
	return rc;
}

/*
	Generate a list of all IP address associated with the interfaces available.
	For now we capture them all, but we may need to limit. The port is smashed
	onto each IP we find so that we can do a direct compare against the addr
	that could be in the route table.

	If the environment variable which limits the binding of our listen port
	to a single interface (ENV_BIND_IF) then ONLY that interface/address is added
	to the list so that we don't pick up entries from the rtable that are for other
	processes listening on different interfaces.

	The ENV_BIN_IF environment variable may be either an IP address (v6 must be in
	square braces), or an interface name (e.g. eth0).
*/
static if_addrs_t*  mk_ip_list( char* port ) {
	if_addrs_t* l;
	struct	ifaddrs *ifs;		// pointer to head
	struct	ifaddrs *ele;		// pointer into the list
	char	octs[NI_MAXHOST+1];
	char	wbuf[NI_MAXHOST+128];
	char*	fmt = NULL;			// address format (v4 or v6)
	char*	envp;				// at the environment var if there
	char*	target_if = NULL;	// target interface supplied by ENV_BIND_IF
	char*	tok;


	if( (l = (if_addrs_t *) malloc( sizeof( if_addrs_t ) )) == NULL ) {
		return NULL;
	}
	memset( l, 0, sizeof( if_addrs_t ) );
	l->addrs = (char **) malloc( sizeof( char* ) * 128 );
	if( l->addrs == NULL ) {
		free( l );
		return NULL;
	}

	if( (envp = getenv( ENV_BIND_IF )) != NULL ) {
		if( isdigit( *envp ) || *envp == '[' ) {					// ip address given and not device name
			snprintf( wbuf, sizeof( wbuf ), "%s:%s", envp, port );		// smash port onto the addr as is
			l->addrs[l->naddrs] = strdup( wbuf );
			l->naddrs++;
			if( DEBUG ) rmr_vlog( RMR_VL_INFO, "rmr: using only specific bind interface when searching specific RT entries: %s\n", wbuf );
			return l;
		}

		target_if = envp;		// device name given, suss it out below
	}

	getifaddrs( &ifs );
	for( ele = ifs; ele; ele = ele->ifa_next ) {
		memset( octs, 0, sizeof( octs ) );
		if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "checking interface: %s\n", ele->ifa_name );
		if( ele && strcmp( ele->ifa_name, "lo" ) &&									// do NOT capture the loopback interface address
			(target_if == NULL || strcmp( ele->ifa_name, target_if ) == 0 ) ) {		// no target, or matches ENV_BIND_IF target

			if( ele->ifa_addr != NULL ) {						// possible for some interfaces to not have an address
				if( ele->ifa_addr->sa_family == AF_INET ) {
					getnameinfo( ele->ifa_addr, sizeof( struct sockaddr_in ),  octs, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
					fmt = "%s:%s";
				} else {
					if( ele->ifa_addr->sa_family == AF_INET6 ) {
						getnameinfo( ele->ifa_addr, sizeof( struct sockaddr_in6 ),  octs, NI_MAXHOST, NULL, 0, NI_NUMERICHOST );
						fmt = "[%s]:%s";
					} else {
						if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "unrecognised IF family (not v4 or v6)\n" );
						continue;
					}
				}

				if( *octs  && fmt != NULL ) {				// possible that we didn't recognise the format (v4 or v6), don't try if we didn't
					if( (tok = strchr( octs, '%' )) != NULL ) {			// for unknown reasons some ip6 addrs have %if-name appended; truncate
						*tok = 0;
					}
					if( l->naddrs < 128 ) {
						if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "capture address: %s: %s\n", ele->ifa_name, octs );

						snprintf( wbuf, sizeof( wbuf ), fmt, octs, port );		// smash port onto the addr
						l->addrs[l->naddrs] = strdup( wbuf );
						l->naddrs++;
					}
				} else {
					if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "unrecognised or no octets octs=%x fmt=%p\n", *octs, fmt );
				}
			} else {
				if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "no ip address for interface: %s\n", ele->ifa_name );
			}
		}
	}

	if( ifs ) {
		freeifaddrs( ifs );
	}

	return l;
}

/*
	Check the address:port passed in and return true if it matches
	one of the addresses we saw when we built the list. Right now
	this isn't a speed intensive part of our processing, so we just
	do a straight search through the list. We don't expect this to
	ever be a higly driven functions so not bothering to optimise.
*/
static int is_this_myip( if_addrs_t* l, char* addr ) {
	int i;

	if( l == NULL ) {
		return 0;
	}

	if( addr == NULL ) {
		return 0;
	}

	for( i = 0; i < l->naddrs; i++ ) {
		if( l->addrs[i] != NULL  &&  strcmp( addr, l->addrs[i] ) == 0 ) {
			return 1;
		}
	}

	return 0;
}

/*
	Expects a buffer containing "sep" separated tokens, and a list of
	IP addresses anchored by ip_list.  Searches the tokens to see if
	any are an ip address:port which is in the ip list.  Returns true
	(1) if a token is in the list, false otherwise.
*/
static int has_myip( char const* buf, if_addrs_t* list, char sep, int max ) {
	char*	dbuf;			// duplicated buf so we can trash
	char** tokens;			// pointer to tokens from the string
	int		ntokens;		// number of tokens buf split into
	int		i;
	int		rc = 0;			// return code

	if( max < 2 ) {
		return 0;
	}

	if( buf == NULL ) {
		return 0;
	}

	if( list == NULL ) {
		return 0;
	}


	dbuf = strdup( buf );			// get a copy we can mess with
	if( dbuf == NULL  ) {
		errno = ENOMEM;
		return 0;
	}

	if( (tokens = (char **) malloc( sizeof( char * ) * max )) == NULL ) {
		errno = ENOMEM;
		free( dbuf );
		return 0;
	}

	ntokens = uta_tokenise( dbuf, tokens, max, sep );
	errno = 0;
	rc = 0;
	for( i = 0; ! rc  && i < ntokens; i++ ) {
		if( tokens[i] ) {
			if( is_this_myip( list, tokens[i] ) ) {
				rc = 1;
				break;
			}
		}
	}

	free( dbuf );
	free( tokens );
	return rc;
}

/*
	Given a list manager block, return the default IP address.
	For now, that is just the first address on the list which
	easily could be non-deterministic and change with each restart
	of the application if a specific interface is not provided via
	the environment variable (ENV_BIND_IF) and if there is more than
	one device available on the container/physical host.
*/
static char* get_default_ip( if_addrs_t* iplist ) {

	if( iplist != NULL  &&  iplist->naddrs > 0  &&  iplist->addrs != NULL ) {
		return strdup( iplist->addrs[0] );
	}

	return NULL;
}

/*
	Write all environment variables that we consider to be important to stderr.
*/
static void uta_dump_env( ) {
	char* token;
	char* elist[] = {
			ENV_BIND_IF,
			ENV_RTG_PORT,
			ENV_RTG_ADDR,
			ENV_SEED_RT,
			ENV_SEED_MEMAP,
			ENV_RTG_RAW,
			ENV_VERBOSE_FILE,
			ENV_NAME_ONLY,
			ENV_WARNINGS,
			ENV_SRC_ID,
			ENV_LOG_HR,
			ENV_LOG_VLEVEL,
			ENV_CTL_PORT,
			ENV_RTREQ_FREA
	};
	int i;

	for( i = 0; i < sizeof( elist ) / sizeof( char *); i ++ ) {
		token = getenv( elist[i] );
		if( token != NULL ) {
			rmr_vlog( RMR_VL_INFO, "dump_env: %s = '%s'\n", elist[i], token );
		} else {
			rmr_vlog( RMR_VL_INFO, "dump_env: %s = <unset>\n", elist[i] );
		}
	}
}

#endif

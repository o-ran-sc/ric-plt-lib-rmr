// :vi sw=4 ts=4 noet:
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
	Mnemonic:	rt_generic_static.c
	Abstract:	These are route table functions which are not specific to the
				underlying protocol.  rtable_static, and rtable_nng_static
				have transport provider specific code.

				This file must be included before the nng/nano specific file as
				it defines types.

	Author:		E. Scott Daniels
	Date:		5  February 2019
*/

#ifndef rt_generic_static_c
#define rt_generic_static_c

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>


/*
	Passed to a symtab foreach callback to construct a list of pointers from
	a current symtab.
*/
typedef struct thing_list {
	int nalloc;
	int nused;
	void** things;
} thing_list_t;

// ------------------------------------------------------------------------------------------------


/*
	Little diddy to trim whitespace and trailing comments. Like shell, trailing comments
	must be at the start of a word (i.e. must be immediatly preceeded by whitespace).
*/
static char* clip( char* buf ) {
	char* 	tok;

	while( *buf && isspace( *buf ) ) {							// skip leading whitespace
		buf++;
	}

	if( (tok = strchr( buf, '#' )) != NULL ) {
		if( tok == buf ) {
			return buf;					// just push back; leading comment sym handled there
		}

		if( isspace( *(tok-1) ) ) {
			*tok = 0;
		}
	}

	for( tok = buf + (strlen( buf ) - 1); tok > buf && isspace( *tok ); tok-- );	// trim trailing spaces too
	*(tok+1) = 0;

	return buf;
}


/*
	Given a message type create a route table entry and add to the hash keyed on the
	message type.  Once in the hash, endpoints can be added with uta_add_ep. Size
	is the number of group slots to allocate in the entry.
*/
static rtable_ent_t* uta_add_rte( route_table_t* rt, uint64_t key, int nrrgroups ) {
	rtable_ent_t* rte;

	if( rt == NULL ) {
		return NULL;
	}

	if( (rte = (rtable_ent_t *) malloc( sizeof( *rte ) )) == NULL ) {
		fprintf( stderr, "rmr_add_rte: malloc failed for entry\n" );
		return NULL;
	}
	memset( rte, 0, sizeof( *rte ) );

	if( nrrgroups <= 0 ) {
		nrrgroups = 10;
	}

	if( (rte->rrgroups = (rrgroup_t **) malloc( sizeof( rrgroup_t * ) * nrrgroups )) == NULL ) {
		fprintf( stderr, "rmr_add_rte: malloc failed for rrgroup array\n" );
		free( rte );
		return NULL;
	}
	memset( rte->rrgroups, 0, sizeof( rrgroup_t *) * nrrgroups );
	rte->nrrgroups = nrrgroups;

	rmr_sym_map( rt->hash, key, rte );							// add to hash using numeric mtype as key

	if( DEBUG ) fprintf( stderr, "[DBUG] route table entry created: k=%lu groups=%d\n", key, nrrgroups );
	return rte;
}

/*
	This accepts partially parsed information from a record sent by route manager or read from
	a file such that:
		ts_field is the msg-type,sender field
		subid is the integer subscription id
		rr_field is the endpoint information for round robening message over

	If all goes well, this will add an RTE to the table under construction.

	The ts_field is checked to see if we should ingest this record. We ingest if one of
	these is true:
		there is no sender info (a generic entry for all)
		there is sender and our host:port matches one of the senders
		the sender info is an IP address that matches one of our IP addresses
*/
static void build_entry( uta_ctx_t* ctx, char* ts_field, uint32_t subid, char* rr_field, int vlevel ) {
	rtable_ent_t*	rte;		// route table entry added
	char*	tok;
	int		ntoks;
	uint64_t key = 0;			// the symtab key will be mtype or sub_id+mtype
	char* 	tokens[128];
	char* 	gtokens[64];
	int		i;
	int		ngtoks;				// number of tokens in the group list
	int		grp;				// index into group list

	ts_field = clip( ts_field );				// ditch extra whitespace and trailing comments
	rr_field = clip( rr_field );

	if( ((tok = strchr( ts_field, ',' )) == NULL ) || 					// no sender names (generic entry for all)
		(uta_has_str( ts_field,  ctx->my_name, ',', 127) >= 0) ||		// our name is in the list
		has_myip( ts_field, ctx->ip_list, ',', 127 ) ) {				// the list has one of our IP addresses

			key = build_rt_key( subid, atoi( ts_field ) );

			if( DEBUG > 1 || (vlevel > 1) ) fprintf( stderr, "[DBUG] create rte for mtype=%s subid=%d key=%lu\n", ts_field, subid, key );

			if( (ngtoks = uta_tokenise( rr_field, gtokens, 64, ';' )) > 0 ) {					// split round robin groups
				rte = uta_add_rte( ctx->new_rtable, key, ngtoks );								// get/create entry for this key

				for( grp = 0; grp < ngtoks; grp++ ) {
					if( (ntoks = uta_tokenise( gtokens[grp], tokens, 64, ',' )) > 0 ) {
						for( i = 0; i < ntoks; i++ ) {
							if( DEBUG > 1  || (vlevel > 1)) fprintf( stderr, "[DBUG]    add endpoint  %s\n", ts_field );
							uta_add_ep( ctx->new_rtable, rte, tokens[i], grp );
						}
					}
				}
			}
		} else {
			if( DEBUG || (vlevel > 2) )
				fprintf( stderr, "entry not included, sender not matched: %s\n", tokens[1] );
		}
}

/*
	Parse a single record recevied from the route table generator, or read
	from a static route table file.  Start records cause a new table to
	be started (if a partial table was received it is discarded. Table
	entry records are added to the currenly 'in progress' table, and an
	end record causes the in progress table to be finalised and the
	currently active table is replaced.

	We expect one of several types:
		newrt|{start|end}
		rte|<mtype>[,sender]|<endpoint-grp>[;<endpoint-grp>,...]
		mse|<mtype>[,sender]|<sub-id>|<endpoint-grp>[;<endpoint-grp>,...]
*/
static void parse_rt_rec( uta_ctx_t* ctx, char* buf, int vlevel ) {
	int i;
	int ntoks;							// number of tokens found in something
	int ngtoks;
	int	grp;							// group number
	rtable_ent_t*	rte;				// route table entry added
	char*	tokens[128];
	char*	gtokens[64];				// groups
	char*	tok;						// pointer into a token or string

	if( ! buf ) {
		return;
	}

	while( *buf && isspace( *buf ) ) {							// skip leading whitespace
		buf++;
	}
	for( tok = buf + (strlen( buf ) - 1); tok > buf && isspace( *tok ); tok-- );	// trim trailing spaces too
	*(tok+1) = 0;

	if( (ntoks = uta_tokenise( buf, tokens, 128, '|' )) > 0 ) {
		switch( *(tokens[0]) ) {
			case 0:													// ignore blanks
				// fallthrough
			case '#':												// and comment lines
				break;

			case 'n':												// newrt|{start|end}
				tokens[1] = clip( tokens[1] );
				if( strcmp( tokens[1], "end" ) == 0 ) {				// wrap up the table we were building
					if( ctx->new_rtable ) {
						uta_rt_drop( ctx->old_rtable );				// time to drop one that was previously replaced
						ctx->old_rtable = ctx->rtable;				// currently active becomes old and allowed to 'drain'
						ctx->rtable = ctx->new_rtable;				// one we've been adding to becomes active
						ctx->new_rtable = NULL;
						if( DEBUG > 1 || (vlevel > 1) ) fprintf( stderr, "[DBUG] end of route table noticed\n" );
					} else {
						if( DEBUG > 1 ) fprintf( stderr, "[DBUG] end of route table noticed, but one was not started!\n" );
						ctx->new_rtable = NULL;
					}
				} else {											// start a new table.
					if( ctx->new_rtable != NULL ) {					// one in progress?  this forces it out
						if( DEBUG > 1 || (vlevel > 1) ) fprintf( stderr, "[DBUG] new table; dropping incomplete table\n" );
						uta_rt_drop( ctx->new_rtable );
					}

					if( ctx->rtable )  {
						ctx->new_rtable = uta_rt_clone( ctx->rtable );	// create by cloning endpoint entries from active table
					} else {
						ctx->new_rtable = uta_rt_init(  );				// don't have one yet, just crate empty
					}
					if( DEBUG > 1 || (vlevel > 1)  ) fprintf( stderr, "[DBUG] start of route table noticed\n" );
				}
				break;

			case 'm':					// assume mse entry
				if( ! ctx->new_rtable ) {			// bad sequence, or malloc issue earlier; ignore siliently
					break;
				}

				if( ntoks < 4 ) {
					if( DEBUG ) fprintf( stderr, "[WRN] rmr_rtc: mse record had too few fields: %d instead of 4\n", ntoks );
					break;
				}

				build_entry( ctx, tokens[1], atoi( tokens[2] ), tokens[3], vlevel );
				break;

			case 'r':					// assume rt entry
				if( ! ctx->new_rtable ) {			// bad sequence, or malloc issue earlier; ignore siliently
					break;
				}

				if( ntoks > 3 ) {													// assume new entry with subid last
					build_entry( ctx, tokens[1], atoi( tokens[3] ), tokens[2], vlevel );
				} else {
					build_entry( ctx, tokens[1], UNSET_SUBID, tokens[2], vlevel );			// old school entry has no sub id
				}
				break;

			default:
				if( DEBUG ) fprintf( stderr, "[WRN] rmr_rtc: unrecognised request: %s\n", tokens[0] );
				break;
		}
	}
}

/*
	This function attempts to open a static route table in order to create a 'seed'
	table during initialisation.  The environment variable RMR_SEED_RT is expected
	to contain the necessary path to the file. If missing, or if the file is empty,
	no route table will be available until one is received from the generator.

	This function is probably most useful for testing situations, or extreme
	cases where the routes are static.
*/
static void read_static_rt( uta_ctx_t* ctx, int vlevel ) {
	int		i;
	char*	fname;
	char*	fbuf;				// buffer with file contents
	char*	rec;				// start of the record
	char*	eor;				// end of the record
	int		rcount = 0;			// record count for debug

	if( (fname = getenv( ENV_SEED_RT )) == NULL ) {
		return;
	}

	if( (fbuf = uta_fib( fname ) ) == NULL ) {			// read file into a single buffer
		fprintf( stderr, "[WRN] seed route table could not be opened: %s: %s\n", fname, strerror( errno ) );
		return;
	}

	if( DEBUG ) fprintf( stderr, "[INFO] seed route table successfully opened: %s\n", fname );
	for( rec = fbuf; rec && *rec; rec = eor+1 ) {
		rcount++;
		if( (eor = strchr( rec, '\n' )) != NULL ) {
			*eor = 0;
		}

		parse_rt_rec( ctx, rec, vlevel );
	}

	if( DEBUG ) fprintf( stderr, "[INFO] seed route table successfully parsed: %d records\n", rcount );
	free( fbuf );
}

/*
	Callback driven for each named thing in a symtab. We collect the pointers to those
	things for later use (cloning).
*/
static void collect_things( void* st, void* entry, char const* name, void* thing, void* vthing_list ) {
	thing_list_t*	tl;

	if( (tl = (thing_list_t *) vthing_list) == NULL ) {
		return;
	}

	if( thing == NULL ) {
		return;
	}

	tl->things[tl->nused++] = thing;		// save a reference to the thing
}

/*
	Called to delete a route table entry struct. We delete the array of endpoint
	pointers, but NOT the endpoints referenced as those are referenced from
	multiple entries.
*/
static void del_rte( void* st, void* entry, char const* name, void* thing, void* data ) {
	rtable_ent_t*	rte;
	int i;

	if( (rte = (rtable_ent_t *) thing) == NULL ) {
		return;
	}

	if( rte->rrgroups ) {									// clean up the round robin groups
		for( i = 0; i < rte->nrrgroups; i++ ) {
			if( rte->rrgroups[i] ) {
				free( rte->rrgroups[i]->epts );			// ditch list of endpoint pointers (end points are reused; don't trash them)
			}
		}

		free( rte->rrgroups );
	}

	free( rte );											// finally, drop the potato
}

/*
	Read an entire file into a buffer. We assume for route table files
	they will be smallish and so this won't be a problem.
	Returns a pointer to the buffer, or nil. Caller must free.
	Terminates the buffer with a nil character for string processing.

	If we cannot stat the file, we assume it's empty or missing and return
	an empty buffer, as opposed to a nil, so the caller can generate defaults
	or error if an empty/missing file isn't tolerated.
*/
static char* uta_fib( char* fname ) {
	struct stat	stats;
	off_t		fsize = 8192;	// size of the file
	off_t		nread;			// number of bytes read
	int			fd;
	char*		buf;			// input buffer

	if( (fd = open( fname, O_RDONLY )) >= 0 ) {
		if( fstat( fd, &stats ) >= 0 ) {
			if( stats.st_size <= 0 ) {					// empty file
				close( fd );
				fd = -1;
			} else {
				fsize = stats.st_size;						// stat ok, save the file size
			}
		} else {
			fsize = 8192; 								// stat failed, we'll leave the file open and try to read a default max of 8k
		}
	}

	if( fd < 0 ) {											// didn't open or empty
		if( (buf = (char *) malloc( sizeof( char ) * 1 )) == NULL ) {
			return NULL;
		}

		*buf = 0;
		return buf;
	}

	// add a size limit check here

	if( (buf = (char *) malloc( sizeof( char ) * fsize + 2 )) == NULL ) {		// enough to add nil char to make string
		close( fd );
		errno = ENOMEM;
		return NULL;
	}

	nread = read( fd, buf, fsize );
	if( nread < 0 || nread > fsize ) {							// failure of some kind
		free( buf );
		errno = EFBIG;											// likely too much to handle
		close( fd );
		return NULL;
	}

	buf[nread] = 0;

	close( fd );
	return buf;
}

/*
	Create and initialise a route table; Returns a pointer to the table struct.
*/
static route_table_t* uta_rt_init( ) {
	route_table_t*	rt;

	if( (rt = (route_table_t *) malloc( sizeof( route_table_t ) )) == NULL ) {
		return NULL;
	}

	if( (rt->hash = rmr_sym_alloc( 509 )) == NULL ) {		// modest size, prime
		free( rt );
		return NULL;
	}

	return rt;
}

/*
	Clone (sort of) an existing route table.  This is done to preserve the endpoint
	names referenced in a table (and thus existing sessions) when a new set
	of message type to endpoint name mappings is received.  A new route table
	with only endpoint name references is returned based on the active table in
	the context.
*/
static route_table_t* uta_rt_clone( route_table_t* srt ) {
	endpoint_t*		ep;		// an endpoint
	route_table_t*	nrt;	// new route table
	route_table_t*	art;	// active route table
	void*	sst;			// source symtab
	void*	nst;			// new symtab
	thing_list_t things;
	int i;

	if( srt == NULL ) {
		return NULL;
	}

	if( (nrt = (route_table_t *) malloc( sizeof( *nrt ) )) == NULL ) {
		return NULL;
	}

	if( (nrt->hash = rmr_sym_alloc( 509 )) == NULL ) {		// modest size, prime
		free( nrt );
		return NULL;
	}

	things.nalloc = 2048;
	things.nused = 0;
	things.things = (void **) malloc( sizeof( void * ) * things.nalloc );
	if( things.things == NULL ) {
		free( nrt->hash );
		free( nrt );
		return NULL;
	}

	sst = srt->hash;											// convenience pointers (src symtab)
	nst = nrt->hash;

	rmr_sym_foreach_class( sst, 1, collect_things, &things );		// collect the named endpoints in the active table

	for( i = 0; i < things.nused; i++ ) {
		ep = (endpoint_t *) things.things[i];
		rmr_sym_put( nst, ep->name, 1, ep );						// slam this one into the new table
	}

	free( things.things );
	return nrt;
}

/*
	Given a name, find the endpoint struct in the provided route table.
*/
static endpoint_t* uta_get_ep( route_table_t* rt, char const* ep_name ) {

	if( rt == NULL || rt->hash == NULL || ep_name == NULL || *ep_name == 0 ) {
		return NULL;
	}

	return rmr_sym_get( rt->hash, ep_name, 1 );
}

/*
	Drop the given route table. Purge all type 0 entries, then drop the symtab itself.
*/
static void uta_rt_drop( route_table_t* rt ) {
	if( rt == NULL ) {
		return;
	}

	rmr_sym_foreach_class( rt->hash, 0, del_rte, NULL );		// free each rte referenced by the hash, but NOT the endpoints
	rmr_sym_free( rt->hash );									// free all of the hash related data
	free( rt );
}

/*
	Look up and return the pointer to the endpoint stuct matching the given name.
	If not in the hash, a new endpoint is created, added to the hash. Should always
	return a pointer.
*/
static endpoint_t* rt_ensure_ep( route_table_t* rt, char const* ep_name ) {
	endpoint_t*	ep;

	if( !rt || !ep_name || ! *ep_name ) {
		fprintf( stderr, "[WARN] rmr: rt_ensure:  internal mishap, something undefined rt=%p ep_name=%p\n", rt, ep_name );
		errno = EINVAL;
		return NULL;
	}

	if( (ep = uta_get_ep( rt, ep_name )) == NULL ) { 					// not there yet, make
		if( (ep = (endpoint_t *) malloc( sizeof( *ep ) )) == NULL ) {
			fprintf( stderr, "[WARN] rmr: rt_ensure:  malloc failed for endpoint creation: %s\n", ep_name );
			errno = ENOMEM;
			return NULL;
		}

		ep->open = 0;					// not connected
		ep->addr = uta_h2ip( ep_name );
		ep->name = strdup( ep_name );

		rmr_sym_put( rt->hash, ep_name, 1, ep );
	}

	return ep;
}


/*
	Given a session id and message type build a key that can be used to look up the rte in the route
	table hash. Sub_id is expected to be -1 if there is no session id associated with the entry.
*/
static inline uint64_t build_rt_key( int32_t sub_id, int32_t mtype ) {
	uint64_t key;

	if( sub_id == UNSET_SUBID ) {
		key = 0xffffffff00000000 | mtype;
	} else {
		key = (((uint64_t) sub_id) << 32) | (mtype & 0xffffffff);
	}

	return key;
}


#endif

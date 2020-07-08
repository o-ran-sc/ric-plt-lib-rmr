// :vi sw=4 ts=4 noet:
/*
==================================================================================
	Copyright (c) 2019-2020 Nokia
	Copyright (c) 2018-2020 AT&T Intellectual Property.

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

#include <RIC_message_types.h>		// needed for route manager messages


/*
	Passed to a symtab foreach callback to construct a list of pointers from
	a current symtab.
*/
typedef struct thing_list {
	int nalloc;
	int nused;
	void** things;
	const char** names;
} thing_list_t;

// ---- debugging/testing -------------------------------------------------------------------------

/*
	Dump some stats for an endpoint in the RT. This is generally called to
	verify endpoints after a table load/change.
*/
static void ep_stats( void* st, void* entry, char const* name, void* thing, void* vcounter ) {
	int*	counter;
	endpoint_t* ep;

	if( (ep = (endpoint_t *) thing) == NULL ) {
		return;
	}

	if( (counter = (int *) vcounter) != NULL ) {
		(*counter)++;
	}

	rmr_vlog_force( RMR_VL_DEBUG, "rt endpoint: target=%s open=%d\n", ep->name, ep->open );
}

/*
	Called to count meid entries in the table. The meid points to an 'owning' endpoint
	so we can list what we find
*/
static void meid_stats( void* st, void* entry, char const* name, void* thing, void* vcounter ) {
	int*	counter;
	endpoint_t* ep;

	if( (ep = (endpoint_t *) thing) == NULL ) {
		return;
	}

	if( (counter = (int *) vcounter) != NULL ) {
		(*counter)++;
	}

	rmr_vlog_force( RMR_VL_DEBUG, "meid=%s owner=%s open=%d\n", name, ep->name, ep->open );
}

/*
	Dump counts for an endpoint in the RT. The vid parm is assumed to point to
	the 'source' information and is added to each message.
*/
static void ep_counts( void* st, void* entry, char const* name, void* thing, void* vid ) {
	endpoint_t* ep;
	char*	id;

	if( (ep = (endpoint_t *) thing) == NULL ) {
		return;
	}

	if( (id = (char *) vid) == NULL ) {
		id = "missing";
	}

	rmr_vlog_force( RMR_VL_INFO, "sends: ts=%lld src=%s target=%s open=%d succ=%lld fail=%lld (hard=%lld soft=%lld)\n",
		(long long) time( NULL ),
		id,
		ep->name,
		ep->open,
		ep->scounts[EPSC_GOOD],
		ep->scounts[EPSC_FAIL] + ep->scounts[EPSC_TRANS],
		ep->scounts[EPSC_FAIL],
		ep->scounts[EPSC_TRANS]   );
}

/*
	Dump stats for a route entry in the table.
*/
static void rte_stats( void* st, void* entry, char const* name, void* thing, void* vcounter ) {
	int*	counter;
	rtable_ent_t* rte;			// thing is really an rte
	int		mtype;
	int		sid;

	if( (rte = (rtable_ent_t *) thing) == NULL ) {
		return;
	}

	if( (counter = (int *) vcounter) != NULL ) {
		(*counter)++;
	}

	mtype = rte->key & 0xffff;
	sid = (int) (rte->key >> 32);

	rmr_vlog_force( RMR_VL_DEBUG, "rte: key=%016lx mtype=%4d sid=%4d nrrg=%2d refs=%d\n", rte->key, mtype, sid, rte->nrrgroups, rte->refs );
}

/*
	Given a route table, cause some stats to be spit out.
*/
static void  rt_stats( route_table_t* rt ) {
	int* counter;

	if( rt == NULL ) {
		rmr_vlog_force( RMR_VL_DEBUG, "rtstats: nil table\n" );
		return;
	}

	counter = (int *) malloc( sizeof( int ) );
	*counter = 0;
	rmr_vlog_force( RMR_VL_DEBUG, "route table stats:\n" );
	rmr_vlog_force( RMR_VL_DEBUG, "route table endpoints:\n" );
	rmr_sym_foreach_class( rt->hash, RT_NAME_SPACE, ep_stats, counter );		// run endpoints (names) in the active table
	rmr_vlog_force( RMR_VL_DEBUG, "rtable: %d known endpoints\n", *counter );

	rmr_vlog_force( RMR_VL_DEBUG, "route table entries:\n" );
	*counter = 0;
	rmr_sym_foreach_class( rt->hash, RT_MT_SPACE, rte_stats, counter );			// run message type entries
	rmr_vlog_force( RMR_VL_DEBUG, "rtable: %d mt entries in table\n", *counter );

	rmr_vlog_force( RMR_VL_DEBUG, "route table meid map:\n" );
	*counter = 0;
	rmr_sym_foreach_class( rt->hash, RT_ME_SPACE, meid_stats, counter );		// run meid space
	rmr_vlog_force( RMR_VL_DEBUG, "rtable: %d meids in map\n", *counter );

	free( counter );
}

/*
	Given a route table, cause endpoint counters to be written to stderr. The id
	parm is written as the "source" in the output.
*/
static void  rt_epcounts( route_table_t* rt, char* id ) {
	if( rt == NULL ) {
		rmr_vlog_force( RMR_VL_INFO, "endpoint: no counts: empty table\n" );
		return;
	}

	rmr_sym_foreach_class( rt->hash, 1, ep_counts, id );		// run endpoints in the active table
}

// ------------ route manager communication -------------------------------------------------
/*
	Send a request for a table update to the route manager. Updates come in
	async, so send and go.

	pctx is the private context for the thread; ctx is the application context
	that we need to be able to send the application ID in case rt mgr needs to
	use it to idenfity us.

	Returns 0 if we were not able to send a request.
*/
static int send_update_req( uta_ctx_t* pctx, uta_ctx_t* ctx ) {
	rmr_mbuf_t*	smsg;
	int	state = 0;

	if( ctx->rtg_whid < 0 ) {
		return state;
	}

	smsg = rmr_alloc_msg( pctx, 1024 );
	if( smsg != NULL ) {
		smsg->mtype = RMRRM_REQ_TABLE;
		smsg->sub_id = 0;
		snprintf( smsg->payload, 1024, "%s ts=%ld\n", ctx->my_name, (long) time( NULL ) );
		rmr_vlog( RMR_VL_INFO, "rmr_rtc: requesting table: (%s) whid=%d\n", smsg->payload, ctx->rtg_whid );
		smsg->len = strlen( smsg->payload ) + 1;
	
		smsg = rmr_wh_send_msg( pctx, ctx->rtg_whid, smsg );
		if( (state = smsg->state) != RMR_OK ) {
			rmr_vlog( RMR_VL_INFO, "rmr_rtc: send failed: %d whid=%d\n", smsg->state, ctx->rtg_whid );
			rmr_wh_close( ctx, ctx->rtg_whid );					// send failed, assume connection lost
			ctx->rtg_whid = -1;
		}

		rmr_free_msg( smsg );
	}

	return state;
}

/*
	Send an ack to the route table manager for a table ID that we are
	processing.	 State is 1 for OK, and 0 for failed. Reason might 
	be populated if we know why there was a failure.

	Context should be the PRIVATE context that we use for messages
	to route manger and NOT the user's context.

	If a message buffere is passed we use that and use return to sender
	assuming that this might be a response to a call and that is needed
	to send back to the proper calling thread. If msg is nil, we allocate
	and use it.
*/
static void send_rt_ack( uta_ctx_t* ctx, rmr_mbuf_t* smsg, char* table_id, int state, char* reason ) {
	int		use_rts = 1;
	int		payload_size = 1024;
	
	if( ctx == NULL || ctx->rtg_whid < 0 ) {
		return;
	}

	if( ctx->flags & CFL_NO_RTACK ) {		// don't ack if reading from file etc
		return;
	}

	if( smsg != NULL ) {
		smsg = rmr_realloc_payload( smsg, payload_size, FALSE, FALSE );		// ensure it's large enough to send a response
	} else {
		use_rts = 0;
		smsg = rmr_alloc_msg( ctx, payload_size );
	}

	if( smsg != NULL ) {
		smsg->mtype = RMRRM_TABLE_STATE;
		smsg->sub_id = -1;
		snprintf( smsg->payload, payload_size-1, "%s %s %s\n", state == RMR_OK ? "OK" : "ERR", 
			table_id == NULL ? "<id-missing>" : table_id, reason == NULL ? "" : reason );

		smsg->len = strlen( smsg->payload ) + 1;
	
		rmr_vlog( RMR_VL_INFO, "rmr_rtc: sending table state: (%s) state=%d whid=%d\n", smsg->payload, smsg->state, ctx->rtg_whid );
		if( use_rts ) {
			smsg = rmr_rts_msg( ctx, smsg );
		} else {
			smsg = rmr_wh_send_msg( ctx, ctx->rtg_whid, smsg );
		}
		if( (state = smsg->state) != RMR_OK ) {
			rmr_vlog( RMR_VL_WARN, "unable to send table state: %d\n", smsg->state );
			rmr_wh_close( ctx, ctx->rtg_whid );					// send failed, assume connection lost
			ctx->rtg_whid = -1;
		}

		if( ! use_rts ) {
			rmr_free_msg( smsg );			// if not our message we must free the leftovers
		}
	}
}

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
	This accepts a pointer to a nil terminated string, and ensures that there is a
	newline as the last character. If there is not, a new buffer is allocated and
	the newline is added.  If a new buffer is allocated, the buffer passed in is
	freed.  The function returns a pointer which the caller should use, and must
	free.  In the event of an error, a nil pointer is returned.
*/
static char* ensure_nlterm( char* buf ) {
	char*	nb = NULL;
	int		len = 1;

	nb = buf;
	if( buf == NULL || (len = strlen( buf )) < 2 ) {
		if( (nb = (char *) malloc( sizeof( char ) * 2 )) != NULL ) {
			*nb = '\n';
			*(nb+1) = 0;
		}
	} else {
		if( buf[len-1] != '\n' ) {
			rmr_vlog( RMR_VL_WARN, "rmr buf_check: input buffer was not newline terminated (file missing final \\n?)\n" );
			if( (nb = (char *) malloc( sizeof( char ) * (len + 2) )) != NULL ) {
				memcpy( nb, buf, len );
				*(nb+len) = '\n';			// insert \n and nil into the two extra bytes we allocated
				*(nb+len+1) = 0;
			}

			free( buf );
		}
	}

	return nb;
}

/*
	Given a message type create a route table entry and add to the hash keyed on the
	message type.  Once in the hash, endpoints can be added with uta_add_ep. Size
	is the number of group slots to allocate in the entry.
*/
static rtable_ent_t* uta_add_rte( route_table_t* rt, uint64_t key, int nrrgroups ) {
	rtable_ent_t* rte;
	rtable_ent_t* old_rte;		// entry which was already in the table for the key

	if( rt == NULL ) {
		return NULL;
	}

	if( (rte = (rtable_ent_t *) malloc( sizeof( *rte ) )) == NULL ) {
		rmr_vlog( RMR_VL_ERR, "rmr_add_rte: malloc failed for entry\n" );
		return NULL;
	}
	memset( rte, 0, sizeof( *rte ) );
	rte->refs = 1;
	rte->key = key;

	if( nrrgroups < 0 ) {		// zero is allowed as %meid entries have no groups
		nrrgroups = 10;
	}

	if( nrrgroups ) {
		if( (rte->rrgroups = (rrgroup_t **) malloc( sizeof( rrgroup_t * ) * nrrgroups )) == NULL ) {
			free( rte );
			return NULL;
		}
		memset( rte->rrgroups, 0, sizeof( rrgroup_t *) * nrrgroups );
	} else {
		rte->rrgroups = NULL;
	}

	rte->nrrgroups = nrrgroups;

	if( (old_rte = rmr_sym_pull( rt->hash, key )) != NULL ) {
		del_rte( NULL, NULL, NULL, old_rte, NULL );				// dec the ref counter and trash if unreferenced
	}

	rmr_sym_map( rt->hash, key, rte );							// add to hash using numeric mtype as key

	if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "route table entry created: k=%llx groups=%d\n", (long long) key, nrrgroups );
	return rte;
}

/*
	This accepts partially parsed information from an rte or mse record sent by route manager or read from
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

		if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "create rte for mtype=%s subid=%d key=%lx\n", ts_field, subid, key );

		if( (ngtoks = uta_tokenise( rr_field, gtokens, 64, ';' )) > 0 ) {					// split round robin groups
			if( strcmp( gtokens[0], "%meid" ) == 0 ) {
				ngtoks = 0;																	// special indicator that uses meid to find endpoint, no rrobin
			}
			rte = uta_add_rte( ctx->new_rtable, key, ngtoks );								// get/create entry for this key
			rte->mtype = atoi( ts_field );													// capture mtype for debugging

			for( grp = 0; grp < ngtoks; grp++ ) {
				if( (ntoks = uta_rmip_tokenise( gtokens[grp], ctx->ip_list, tokens, 64, ',' )) > 0 ) {		// remove any referneces to our ip addrs
					for( i = 0; i < ntoks; i++ ) {
						if( strcmp( tokens[i], ctx->my_name ) != 0 ) {					// don't add if it is us -- cannot send to ourself
							if( DEBUG > 1  || (vlevel > 1)) rmr_vlog_force( RMR_VL_DEBUG, "add endpoint  ts=%s %s\n", ts_field, tokens[i] );
							uta_add_ep( ctx->new_rtable, rte, tokens[i], grp );
						}
					}
				}
			}
		}
	} else {
		if( DEBUG || (vlevel > 2) ) {
			rmr_vlog_force( RMR_VL_DEBUG, "entry not included, sender not matched: %s\n", tokens[1] );
		}
	}
}

/*
	Trash_entry takes a partially parsed record from the input and
	will delete the entry if the sender,mtype matches us or it's a
	generic mtype. The refernce in the new table is removed and the
	refcounter for the actual rte is decreased. If that ref count is
	0 then the memory is freed (handled byh the del_rte call).
*/
static void trash_entry( uta_ctx_t* ctx, char* ts_field, uint32_t subid, int vlevel ) {
	rtable_ent_t*	rte;		// route table entry to be 'deleted'
	char*	tok;
	int		ntoks;
	uint64_t key = 0;			// the symtab key will be mtype or sub_id+mtype
	char* 	tokens[128];

	if( ctx == NULL || ctx->new_rtable == NULL || ctx->new_rtable->hash == NULL ) {
		return;
	}

	ts_field = clip( ts_field );				// ditch extra whitespace and trailing comments

	if( ((tok = strchr( ts_field, ',' )) == NULL ) || 					// no sender names (generic entry for all)
		(uta_has_str( ts_field,  ctx->my_name, ',', 127) >= 0) ||		// our name is in the list
		has_myip( ts_field, ctx->ip_list, ',', 127 ) ) {				// the list has one of our IP addresses

		key = build_rt_key( subid, atoi( ts_field ) );
		rte = rmr_sym_pull( ctx->new_rtable->hash, key );			// get it
		if( rte != NULL ) {
			if( DEBUG || (vlevel > 1) ) {
				 rmr_vlog_force( RMR_VL_DEBUG, "delete rte for mtype=%s subid=%d key=%08lx\n", ts_field, subid, key );
			}
			rmr_sym_ndel( ctx->new_rtable->hash, key );			// clear from the new table
			del_rte( NULL, NULL, NULL, rte, NULL );				// clean up the memory: reduce ref and free if ref == 0
		} else {
			if( DEBUG || (vlevel > 1) ) {
				rmr_vlog_force( RMR_VL_DEBUG, "delete could not find rte for mtype=%s subid=%d key=%lx\n", ts_field, subid, key );
			}
		}
	} else {
		if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "delete rte skipped: %s\n", ts_field );
	}
}

/*
	Given the tokens from an mme_ar (meid add/replace) entry, add the entries.
	the 'owner' which should be the dns name or IP address of an enpoint
	the meid_list is a space separated list of me IDs

	This function assumes the caller has vetted the pointers as needed.

	For each meid in the list, an entry is pushed into the hash which references the owner
	endpoint such that when the meid is used to route a message it references the endpoint
	to send messages to.
*/
static void parse_meid_ar( route_table_t* rtab, char* owner, char* meid_list, int vlevel ) {
	char*	tok;
	int		ntoks;
	char* 	tokens[128];
	int		i;
	int		state;
	endpoint_t*	ep;						// endpoint struct for the owner

	owner = clip( owner );				// ditch extra whitespace and trailing comments
	meid_list = clip( meid_list );

	ntoks = uta_tokenise( meid_list, tokens, 128, ' ' );
	for( i = 0; i < ntoks; i++ ) {
		if( (ep = rt_ensure_ep( rtab, owner )) != NULL ) {
			state = rmr_sym_put( rtab->hash, tokens[i], RT_ME_SPACE, ep );						// slam this one in if new; replace if there
			if( DEBUG || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "parse_meid_ar: add/replace meid: %s owned by: %s state=%d\n", tokens[i], owner, state );
		} else {
			rmr_vlog( RMR_VL_WARN, "rmr parse_meid_ar: unable to create an endpoint for owner: %s", owner );
		}
	}
}

/*
	Given the tokens from an mme_del, delete the listed meid entries from the new
	table. The list is a space separated list of meids.

	The meids in the hash reference endpoints which are never deleted and so
	the only thing that we need to do here is to remove the meid from the hash.

	This function assumes the caller has vetted the pointers as needed.
*/
static void parse_meid_del( route_table_t* rtab, char* meid_list, int vlevel ) {
	char*	tok;
	int		ntoks;
	char* 	tokens[128];
	int		i;

	if( rtab->hash == NULL ) {
		return;
	}

	meid_list = clip( meid_list );

	ntoks = uta_tokenise( meid_list, tokens, 128, ' ' );
	for( i = 0; i < ntoks; i++ ) {
		rmr_sym_del( rtab->hash, tokens[i], RT_ME_SPACE );						// and it only took my little finger to blow it away!
		if( DEBUG || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "parse_meid_del: meid deleted: %s\n", tokens[i] );
	}
}

/*
	Parse a partially parsed meid record. Tokens[0] should be one of:
		meid_map, mme_ar, mme_del.

	pctx is the private context needed to return an ack/nack using the provided
	message buffer with the route managers address info.
*/
static void meid_parser( uta_ctx_t* ctx, uta_ctx_t* pctx, rmr_mbuf_t* mbuf, char** tokens, int ntoks, int vlevel ) {
	char wbuf[1024];

	if( tokens == NULL || ntoks < 1 ) {
		return;							// silent but should never happen
	}

	if( ntoks < 2 ) {					// must have at least two for any valid request record
		rmr_vlog( RMR_VL_ERR, "meid_parse: not enough tokens on %s record\n", tokens[0] );
		return;
	}

	if( strcmp( tokens[0], "meid_map" ) == 0 ) {					// start or end of the meid map update
		tokens[1] = clip( tokens[1] );
		if( *(tokens[1]) == 's' ) {
			if( ctx->new_rtable != NULL ) {					// one in progress?  this forces it out
				if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "meid map start: dropping incomplete table\n" );
				uta_rt_drop( ctx->new_rtable );
				send_rt_ack( pctx, mbuf, ctx->table_id, !RMR_OK, "table not complete" );	// nack the one that was pending as end never made it
			}

			if( ctx->table_id != NULL ) {
				free( ctx->table_id );
			}
			if( ntoks >2 ) {
				ctx->table_id = strdup( clip( tokens[2] ) );
			} else {
				ctx->table_id = NULL;
			}
			ctx->new_rtable = uta_rt_clone_all( ctx->rtable );		// start with a clone of everything (mtype, endpoint refs and meid)
			ctx->new_rtable->mupdates = 0;
			if( DEBUG || (vlevel > 1)  ) rmr_vlog_force( RMR_VL_DEBUG, "meid_parse: meid map start found\n" );
		} else {
			if( strcmp( tokens[1], "end" ) == 0 ) {								// wrap up the table we were building
				if( ntoks > 2 ) {												// meid_map | end | <count> |??? given
					if( ctx->new_rtable->mupdates != atoi( tokens[2] ) ) {		// count they added didn't match what we received
						rmr_vlog( RMR_VL_ERR, "meid_parse: meid map update had wrong number of records: received %d expected %s\n", ctx->new_rtable->mupdates, tokens[2] );
						snprintf( wbuf, sizeof( wbuf ), "missing table records: expected %s got %d\n", tokens[2], ctx->new_rtable->updates );
						send_rt_ack( pctx, mbuf, ctx->table_id, !RMR_OK, wbuf );
						uta_rt_drop( ctx->new_rtable );
						ctx->new_rtable = NULL;
						return;
					}

					if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "meid_parse: meid map update ended; found expected number of entries: %s\n", tokens[2] );
				}

				if( ctx->new_rtable ) {
					uta_rt_drop( ctx->old_rtable );				// time to drop one that was previously replaced
					ctx->old_rtable = ctx->rtable;				// currently active becomes old and allowed to 'drain'
					ctx->rtable = ctx->new_rtable;				// one we've been adding to becomes active
					ctx->new_rtable = NULL;
					if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "end of meid map noticed\n" );
					send_rt_ack( pctx, mbuf, ctx->table_id, RMR_OK, NULL );

					if( vlevel > 0 ) {
						rmr_vlog_force( RMR_VL_DEBUG, "old route table:\n" );
						rt_stats( ctx->old_rtable );
						rmr_vlog_force( RMR_VL_DEBUG, "new route table:\n" );
						rt_stats( ctx->rtable );
					}
				} else {
					if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "end of meid map noticed, but one was not started!\n" );
					ctx->new_rtable = NULL;
				}
			}
		}

		return;
	}	

	if( ! ctx->new_rtable ) { 			// for any other mmap entries, there must be a table in progress or we punt
		if( DEBUG ) rmr_vlog( RMR_VL_DEBUG, "meid update/delte (%s) encountered, but table update not started\n", tokens[0] );
		return;
	}

	if( strcmp( tokens[0], "mme_ar" ) == 0 ) {
		if( ntoks < 3  || tokens[1] == NULL || tokens[2] == NULL ) {
			rmr_vlog( RMR_VL_ERR, "meid_parse: mme_ar record didn't have enough tokens found %d\n", ntoks );
			return;
		}
		parse_meid_ar( ctx->new_rtable,  tokens[1], tokens[2], vlevel );
		ctx->new_rtable->mupdates++;
	}

	if( strcmp( tokens[0], "mme_del" ) == 0 ) {
		if( ntoks < 2 ) {
			rmr_vlog( RMR_VL_ERR, "meid_parse: mme_del record didn't have enough tokens\n" );
			return;
		}
		parse_meid_del( ctx->new_rtable,  tokens[1], vlevel );
		ctx->new_rtable->mupdates++;
	}
}

/*
	Parse a single record recevied from the route table generator, or read
	from a static route table file.  Start records cause a new table to
	be started (if a partial table was received it is discarded. Table
	entry records are added to the currenly 'in progress' table, and an
	end record causes the in progress table to be finalised and the
	currently active table is replaced.

	The updated table will be activated when the *|end record is encountered.
	However, to allow for a "double" update, where both the meid map and the
	route table must be updated at the same time, the end indication on a
	route table (new or update) may specifiy "hold" which indicates that meid
	map entries are to follow and the updated route table should be held as
	pending until the end of the meid map is received and validated.

	CAUTION:  we are assuming that there is a single route/meid map generator
		and as such only one type of update is received at a time; in other
		words, the sender cannot mix update records and if there is more than
		one sender process they must synchronise to avoid issues.


	For a RT update, we expect:
		newrt | start | <table-id>
		newrt | end | <count>
		rte|<mtype>[,sender]|<endpoint-grp>[;<endpoint-grp>,...]
		mse|<mtype>[,sender]|<sub-id>|<endpoint-grp>[;<endpoint-grp>,...]
		mse| <mtype>[,sender] | <sub-id> | %meid


	For a meid map update we expect:
		meid_map | start | <table-id>
		meid_map | end | <count> | <md5-hash>
		mme_ar | <e2term-id> | <meid0> <meid1>...<meidn>
		mme_del | <meid0> <meid1>...<meidn>


	The pctx is our private context that must be used to send acks/status
	messages back to the route manager.  The regular ctx is the ctx that
	the user has been given and thus that's where we have to hang the route
	table we're working with.

	If mbuf is given, and we need to ack, then we ack using the mbuf and a
	return to sender call (allows route manager to use wh_call() to send
	an update and rts is required to get that back to the right thread).
	If mbuf is nil, then one will be allocated (in ack) and a normal wh_send
	will be used.
*/
static void parse_rt_rec( uta_ctx_t* ctx,  uta_ctx_t* pctx, char* buf, int vlevel, rmr_mbuf_t* mbuf ) {
	int i;
	int ntoks;							// number of tokens found in something
	int ngtoks;
	int	grp;							// group number
	rtable_ent_t*	rte;				// route table entry added
	char*	tokens[128];
	char*	tok;						// pointer into a token or string
	char	wbuf[1024];

	if( ! buf ) {
		return;
	}

	while( *buf && isspace( *buf ) ) {							// skip leading whitespace
		buf++;
	}
	for( tok = buf + (strlen( buf ) - 1); tok > buf && isspace( *tok ); tok-- );	// trim trailing spaces too
	*(tok+1) = 0;

	if( (ntoks = uta_tokenise( buf, tokens, 128, '|' )) > 0 ) {
		tokens[0] = clip( tokens[0] );
		switch( *(tokens[0]) ) {
			case 0:													// ignore blanks
				// fallthrough
			case '#':												// and comment lines
				break;

			case 'd':												// del | [sender,]mtype | sub-id
				if( ! ctx->new_rtable ) {			// bad sequence, or malloc issue earlier; ignore siliently
					break;
				}

				if( ntoks < 3 ) {
					if( DEBUG ) rmr_vlog( RMR_VL_WARN, "rmr_rtc: del record had too few fields: %d instead of 3\n", ntoks );
					break;
				}

				trash_entry( ctx, tokens[1], atoi( tokens[2] ), vlevel );
				ctx->new_rtable->updates++;
				break;

			case 'n':												// newrt|{start|end}
				tokens[1] = clip( tokens[1] );
				if( strcmp( tokens[1], "end" ) == 0 ) {				// wrap up the table we were building
					if( ntoks >2 ) {
						if( ctx->new_rtable->updates != atoi( tokens[2] ) ) {	// count they added didn't match what we received
							rmr_vlog( RMR_VL_ERR, "rmr_rtc: RT update had wrong number of records: received %d expected %s\n",
								ctx->new_rtable->updates, tokens[2] );
							snprintf( wbuf, sizeof( wbuf ), "missing table records: expected %s got %d\n", tokens[2], ctx->new_rtable->updates );
							send_rt_ack( pctx, mbuf, ctx->table_id, !RMR_OK, wbuf );
							uta_rt_drop( ctx->new_rtable );
							ctx->new_rtable = NULL;
							break;
						}
					}

					if( ctx->new_rtable ) {
						uta_rt_drop( ctx->old_rtable );				// time to drop one that was previously replaced
						ctx->old_rtable = ctx->rtable;				// currently active becomes old and allowed to 'drain'
						ctx->rtable = ctx->new_rtable;				// one we've been adding to becomes active
						ctx->new_rtable = NULL;
						if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog( RMR_VL_DEBUG, "end of route table noticed\n" );

						if( vlevel > 0 ) {
							rmr_vlog_force( RMR_VL_DEBUG, "old route table:\n" );
							rt_stats( ctx->old_rtable );
							rmr_vlog_force( RMR_VL_DEBUG, "new route table:\n" );
							rt_stats( ctx->rtable );
						}

						send_rt_ack( pctx, mbuf, ctx->table_id, RMR_OK, NULL );
						ctx->rtable_ready = 1;							// route based sends can now happen
					} else {
						if( DEBUG > 1 ) rmr_vlog_force( RMR_VL_DEBUG, "end of route table noticed, but one was not started!\n" );
						ctx->new_rtable = NULL;
					}
				} else {															// start a new table.
					if( ctx->new_rtable != NULL ) {									// one in progress?  this forces it out
						send_rt_ack( pctx, mbuf, ctx->table_id, !RMR_OK, "table not complete" );			// nack the one that was pending as end never made it

						if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "new table; dropping incomplete table\n" );
						uta_rt_drop( ctx->new_rtable );
					}

					if( ctx->table_id != NULL ) {
						free( ctx->table_id );
					}
					if( ntoks >2 ) {
						ctx->table_id = strdup( clip( tokens[2] ) );
					} else {
						ctx->table_id = NULL;
					}

					ctx->new_rtable = NULL;
					ctx->new_rtable = uta_rt_clone( ctx->rtable );	// create by cloning endpoint and meidtentries from active table
					ctx->new_rtable->updates = 0;						// init count of entries received
					if( DEBUG > 1 || (vlevel > 1)  ) rmr_vlog_force( RMR_VL_DEBUG, "start of route table noticed\n" );
				}
				break;

			case 'm':									// mse entry or one of the meid_ records
				if( strcmp( tokens[0], "mse" ) == 0 ) {
					if( ! ctx->new_rtable ) {			// bad sequence, or malloc issue earlier; ignore siliently
						break;
					}

					if( ntoks < 4 ) {
						if( DEBUG ) rmr_vlog( RMR_VL_WARN, "rmr_rtc: mse record had too few fields: %d instead of 4\n", ntoks );
						break;
					}

					build_entry( ctx, tokens[1], atoi( tokens[2] ), tokens[3], vlevel );
					ctx->new_rtable->updates++;
				} else {
					meid_parser( ctx, pctx, mbuf, tokens, ntoks, vlevel );
				}
				break;

			case 'r':					// assume rt entry
				if( ! ctx->new_rtable ) {			// bad sequence, or malloc issue earlier; ignore siliently
					break;
				}

				ctx->new_rtable->updates++;
				if( ntoks > 3 ) {													// assume new entry with subid last
					build_entry( ctx, tokens[1], atoi( tokens[3] ), tokens[2], vlevel );
				} else {
					build_entry( ctx, tokens[1], UNSET_SUBID, tokens[2], vlevel );			// old school entry has no sub id
				}
				break;

			case 'u':												// update current table, not a total replacement
				tokens[1] = clip( tokens[1] );
				if( strcmp( tokens[1], "end" ) == 0 ) {				// wrap up the table we were building
					if( ctx->new_rtable == NULL ) {					// update table not in progress
						break;
					}

					if( ntoks >2 ) {
						if( ctx->new_rtable->updates != atoi( tokens[2] ) ) {	// count they added didn't match what we received
							rmr_vlog( RMR_VL_ERR, "rmr_rtc: RT update had wrong number of records: received %d expected %s\n",
								ctx->new_rtable->updates, tokens[2] );
							uta_rt_drop( ctx->new_rtable );
							ctx->new_rtable = NULL;
							break;
						}
					}

					if( ctx->new_rtable ) {
						uta_rt_drop( ctx->old_rtable );				// time to drop one that was previously replaced
						ctx->old_rtable = ctx->rtable;				// currently active becomes old and allowed to 'drain'
						ctx->rtable = ctx->new_rtable;				// one we've been adding to becomes active
						ctx->new_rtable = NULL;
						if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "end of rt update noticed\n" );

						if( vlevel > 0 ) {
							rmr_vlog_force( RMR_VL_DEBUG, "old route table:\n" );
							rt_stats( ctx->old_rtable );
							rmr_vlog_force( RMR_VL_DEBUG, "updated route table:\n" );
							rt_stats( ctx->rtable );
						}
					} else {
						if( DEBUG > 1 ) rmr_vlog_force( RMR_VL_DEBUG, "end of rt update noticed, but one was not started!\n" );
						ctx->new_rtable = NULL;
					}
				} else {											// start a new table.
					if( ctx->new_rtable != NULL ) {					// one in progress?  this forces it out
						if( DEBUG > 1 || (vlevel > 1) ) rmr_vlog_force( RMR_VL_DEBUG, "new table; dropping incomplete table\n" );
						uta_rt_drop( ctx->new_rtable );
					}

					if( ntoks >2 ) {
						if( ctx->table_id != NULL ) {
							free( ctx->table_id );
						}
						ctx->table_id = strdup( clip( tokens[2] ) );
					}

					ctx->new_rtable = uta_rt_clone_all( ctx->rtable );	// start with a clone of everything (endpts and entries)
					ctx->new_rtable->updates = 0;						// init count of updates received
					if( DEBUG > 1 || (vlevel > 1)  ) rmr_vlog_force( RMR_VL_DEBUG, "start of rt update noticed\n" );
				}
				break;

			default:
				if( DEBUG ) rmr_vlog( RMR_VL_WARN, "rmr_rtc: unrecognised request: %s\n", tokens[0] );
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

	if( (fbuf = ensure_nlterm( uta_fib( fname ) ) ) == NULL ) {			// read file into a single buffer (nil terminated string)
		rmr_vlog( RMR_VL_WARN, "rmr read_static: seed route table could not be opened: %s: %s\n", fname, strerror( errno ) );
		return;
	}

	if( DEBUG ) rmr_vlog_force( RMR_VL_DEBUG, "seed route table successfully opened: %s\n", fname );
	for( eor = fbuf; *eor; eor++ ) {					// fix broken systems that use \r or \r\n to terminate records
		if( *eor == '\r' ) {
			*eor = '\n';								// will look like a blank line which is ok
		}
	}

	rec = fbuf;
	while( rec && *rec ) {
		rcount++;
		if( (eor = strchr( rec, '\n' )) != NULL ) {
			*eor = 0;
		} else {
			rmr_vlog( RMR_VL_WARN, "rmr read_static: seed route table had malformed records (missing newline): %s\n", fname );
			rmr_vlog( RMR_VL_WARN, "rmr read_static: seed route table not used: %s\n", fname );
			free( fbuf );
			return;
		}

		parse_rt_rec( ctx, NULL, rec, vlevel, NULL );		// no pvt context as we can't ack

		rec = eor+1;
	}

	if( DEBUG ) rmr_vlog_force( RMR_VL_DEBUG, "rmr_read_static:  seed route table successfully parsed: %d records\n", rcount );
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

	tl->names[tl->nused] = name;			// the name/key
	tl->things[tl->nused++] = thing;		// save a reference to the thing
}

/*
	Called to delete a route table entry struct. We delete the array of endpoint
	pointers, but NOT the endpoints referenced as those are referenced from
	multiple entries.

	Route table entries can be concurrently referenced by multiple symtabs, so
	the actual delete happens only if decrementing the rte's ref count takes it
	to 0. Thus, it is safe to call this function across a symtab when cleaning up
	the symtab, or overlaying an entry.

	This function uses ONLY the pointer to the rte (thing) and ignores the other
	information that symtab foreach function passes (st, entry, and data) which
	means that it _can_ safetly be used outside of the foreach setting. If
	the function is changed to depend on any of these three, then a stand-alone
	rte_cleanup() function should be added and referenced by this, and refererences
	to this outside of the foreach world should be changed.
*/
static void del_rte( void* st, void* entry, char const* name, void* thing, void* data ) {
	rtable_ent_t*	rte;
	int i;

	if( (rte = (rtable_ent_t *) thing) == NULL ) {
		return;
	}

	rte->refs--;
	if( rte->refs > 0 ) {			// something still referencing, so it lives
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

	if( (rt->hash = rmr_sym_alloc( RT_SIZE )) == NULL ) {
		free( rt );
		return NULL;
	}

	return rt;
}

/*
	Clones one of the spaces in the given table.
	Srt is the source route table, Nrt is the new route table; if nil, we allocate it.
	Space is the space in the old table to copy. Space 0 uses an integer key and
	references rte structs. All other spaces use a string key and reference endpoints.
*/
static route_table_t* rt_clone_space( route_table_t* srt, route_table_t* nrt, int space ) {
	endpoint_t*		ep;		// an endpoint
	rtable_ent_t*	rte;	// a route table entry
	void*	sst;			// source symtab
	void*	nst;			// new symtab
	thing_list_t things;	// things from the space to copy
	int		i;
	int		free_on_err = 0;

	if( nrt == NULL ) {				// make a new table if needed
		free_on_err = 1;
		nrt = uta_rt_init();
	}

	if( srt == NULL ) {		// source was nil, just give back the new table
		return nrt;
	}

	things.nalloc = 2048;
	things.nused = 0;
	things.things = (void **) malloc( sizeof( void * ) * things.nalloc );
	things.names = (const char **) malloc( sizeof( char * ) * things.nalloc );
	if( things.things == NULL ) {
		if( free_on_err ) {
			free( nrt->hash );
			free( nrt );
			nrt = NULL;
		}

		return nrt;
	}

	sst = srt->hash;											// convenience pointers (src symtab)
	nst = nrt->hash;

	rmr_sym_foreach_class( sst, space, collect_things, &things );		// collect things from this space

	if( DEBUG ) rmr_vlog_force( RMR_VL_DEBUG, "clone space cloned %d things in space %d\n",  things.nused, space );
	for( i = 0; i < things.nused; i++ ) {
		if( space ) {												// string key, epoint reference
			ep = (endpoint_t *) things.things[i];
			rmr_sym_put( nst, things.names[i], space, ep );					// slam this one into the new table
		} else {
			rte = (rtable_ent_t *) things.things[i];
			rte->refs++;											// rtes can be removed, so we track references
			rmr_sym_map( nst, rte->key, rte );						// add to hash using numeric mtype/sub-id as key (default to space 0)
		}
	}

	free( things.things );
	free( (void *) things.names );
	return nrt;
}

/*
	Creates a new route table and then clones the parts of the table which we must keep with each newrt|start.
	The endpoint and meid entries in the hash must be preserved.
*/
static route_table_t* uta_rt_clone( route_table_t* srt ) {
	endpoint_t*		ep;				// an endpoint
	rtable_ent_t*	rte;			// a route table entry
	route_table_t*	nrt = NULL;		// new route table
	int i;

	if( srt == NULL ) {
		return uta_rt_init();		// no source to clone, just return an empty table
	}

	nrt = rt_clone_space( srt, nrt, RT_NAME_SPACE );		// allocate a new one, add endpoint refs
	rt_clone_space( srt, nrt, RT_ME_SPACE );				// add meid refs to new

	return nrt;
}

/*
	Creates a new route table and then clones  _all_ of the given route table (references 
	both endpoints AND the route table entries. Needed to support a partial update where 
	some route table entries will not be deleted if not explicitly in the update and when 
	we are adding/replacing meid references.
*/
static route_table_t* uta_rt_clone_all( route_table_t* srt ) {
	endpoint_t*		ep;				// an endpoint
	rtable_ent_t*	rte;			// a route table entry
	route_table_t*	nrt = NULL;		// new route table
	int i;

	if( srt == NULL ) {
		return uta_rt_init();		// no source to clone, just return an empty table
	}

	nrt = rt_clone_space( srt, nrt, RT_MT_SPACE );			// create new, clone all spaces to it
	rt_clone_space( srt, nrt, RT_NAME_SPACE );
	rt_clone_space( srt, nrt, RT_ME_SPACE );

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
		rmr_vlog( RMR_VL_WARN, "rt_ensure:  internal mishap, something undefined rt=%p ep_name=%p\n", rt, ep_name );
		errno = EINVAL;
		return NULL;
	}

	if( (ep = uta_get_ep( rt, ep_name )) == NULL ) { 					// not there yet, make
		if( (ep = (endpoint_t *) malloc( sizeof( *ep ) )) == NULL ) {
			rmr_vlog( RMR_VL_WARN, "rt_ensure:  malloc failed for endpoint creation: %s\n", ep_name );
			errno = ENOMEM;
			return NULL;
		}

		ep->notify = 1;								// show notification on first connection failure
		ep->open = 0;								// not connected
		ep->addr = uta_h2ip( ep_name );
		ep->name = strdup( ep_name );
		pthread_mutex_init( &ep->gate, NULL );		// init with default attrs
		memset( &ep->scounts[0], 0, sizeof( ep->scounts ) );

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

/*
	Given a route table and meid string, find the owner (if known). Returns a pointer to
	the endpoint struct or nil.
*/
static inline endpoint_t*  get_meid_owner( route_table_t *rt, char* meid ) {
	endpoint_t* ep;		// the ep we found in the hash

	if( rt == NULL || rt->hash == NULL || meid == NULL || *meid == 0 ) {
		return NULL;
	}

	return (endpoint_t *) rmr_sym_get( rt->hash, meid, RT_ME_SPACE ); 
}

#endif

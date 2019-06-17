// :vim ts=4 sw=4 noet:
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
	Mnemonic:	rtm_sim.c
	Abstract:	This is a simple route manager simulation which provides the ability
				to push a route table into one or more xAPPs. Designed just to
				drive the internal RMr route table collector outside of the static
				file allowing for testing of port definition in some containerised
				environments.

				This application does not persist; it generates a set of tables based
				on the config file, connects to all applications listed, distributes
				the table, and exits.  If periodic delivery of one or more different
				configurations needs to be executed, use a shell script to wrap this
				application in a loop.

	Date:		14 June 2019
	Author:		E. Scott Daniels
*/

/*
config file format:
	# 	comment and blank lines allowed
	#	trailing comments allowed

	# port is used for any app listed in send2 which does not have a trailing :port
	# it may be supplied as a different value before each table, and if not
	# redefined applies to all subsequent tables.
	#

	# A table consists of a send2 list (app[:port]) which are the applications that will
	# receive the table. Each table may contain one or more entries.  Entries define
	# the message type and subscription ID, along with one or more round robin groups.
	# A rrgroup is one or more app:port "endpoints" which RMr will use when sending
	# messages of the indicated type/subid.  Port on a rrgroup is rquired and is the
	# port that the application uses for app to app communications.
	#

	port:	xapp-rtg-listen-port	# 4561 default
	table:
		send2: app1:port app2:port ... appn:port
		entry:
			mtype:	n
			subid:	n
			rrgroup:	app:port... app:port

		entry:
			mtype: n
			subid: n
			rrgroup: app:port ... app:port
*/


#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include  <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "req_resp.c"		// simple nano interface for request/response connections


#define CONNECTED	1		// we've established a shunt connection to the app

#define TRUE		1
#define FALSE		0

#define ALLOC_NEW	1		// rrsend should allocate a new buffer

#define MAX_TABLES	16		// total tables we support
#define MAX_SEND2	64		// max number of apps that a table can be sent to
#define MAX_APPS	1024	// max total apps defined (tables * send2)
#define MAX_GROUPS	64		// max num of round robin groups per entry
#define MAX_RRG_SIZE 64		// max number of apps in a group
#define MAX_ENTRIES	256		// max entries in a table
#define MAX_TOKENS	512		// max tokens we'll break a buffer into


// ---------------------------------------------------------------------------------------

/*
	Things we need to track for an application.
*/
typedef struct app {
	void*	shunt;			// the rr context to shunt directly to an app
	int		state;			// connected or not
	char*	name;			// IP address or DNS name and port for connecting
	char*	port;			// rr wants two strings as it builds it's own NN string
} app_t;



// ----- table stuff (very staticly sized, but this isn't for prod) ----------------------
/*
	A round robin group in a table entry
*/
typedef struct rrgroup {
	int napps;						// number of apps
	char*	apps[MAX_RRG_SIZE];
} rrgroup_t;

/*
	A single table entry.
*/
typedef struct entry {
	int		mtype;					// entry message type
	int		subid;					// entry sub id
	int		ngroups;
	rrgroup_t	groups[MAX_GROUPS];	// the entry's groups
} entry_t;

/*
	Defines a table which we will distribute.
*/
typedef struct table {
	int		napps;						// number of apps this table is sent to
	int		first_app;					// first app in minfo that we send to
	int		nentries;					// number of entries
	entry_t	entries[MAX_ENTRIES];
} table_t;

/*
	Master set of contextual information.
*/
typedef struct master {
	int		napps;				// number in use (next insert point)
	int		ntables;
	int		port;				// the port applications open by default for our connections
	app_t	apps[MAX_APPS];
	table_t	tables[MAX_TABLES];
} master_t;

/*
	Record buffer; file in memory which can be iterated over a record at a time.
*/
typedef struct rbuffer {
	char*	buffer;				// stuff read from file
	char*	rec;				// next record
	int		at_end;				// true if end was reached
} rbuffer_t;

/*
	Set of tokens.
*/
typedef struct tokens {
	char*	buffer;					// buffer that tokens points into
	int		ntoks;					// number of tokens in tokens
	char*	tokens[MAX_TOKENS];		// pointers into buffer at the start of each token 0..ntokens-1
} tokens_t;


// ----- token utilities ------------------------------------------------------------

/*
	Frees a token manager.
*/
static void free_tokens( tokens_t* t ) {
	if( t == NULL ) {
		return;
	}

	free( t );
}

/*
	Simple tokeniser.  If sep is whitespace, then leading whitespace (all, not just
	sep) is ignored; if sep is not whitespace, then leadign whitespace is included
	in the first token.  If sep is whitespace, consecutive instances of whitespace
	are treated as a single seperator:
		if sep given as space, then
		"bug boo" and "bug     boo"  both generate two tokens: (bug) (boo)

		if sep given as pipe (|), then
		"bug||boo"   generates three tokens:  (bug), (), (boo)

	Each token is a zero terminated string.

*/
static tokens_t* tokenise( char* buf, char sep ) {
	tokens_t*	t;
	int			i;
	char		end_sep;		// if quoted endsep will be the quote mark

	if( !buf || !(*buf) ) {
		return NULL;
	}

	t = (tokens_t *) malloc( sizeof( *t ) );
	memset( t, 0, sizeof( *t ) );

	t->buffer = strdup( buf );
	buf = t->buffer;					// convenience

	if( isspace( sep ) ) {										// if sep is in whitespace class
		while( buf != NULL && *buf && isspace( *buf ) ) {		// pass over any leading whitespace
			buf++;
		}

		for( i = 0; i < strlen( buf ); i++ ) {
			if( buf[i] == '\t' ) {
				buf[i] = ' ';
			}
		}
	}

	while( buf != NULL && t->ntoks < MAX_TOKENS && *buf  ) {
		if( *buf == '"' ) {
			end_sep = '"';
			buf++;

		} else {
			end_sep = sep;
		}

		t->tokens[t->ntoks++] = buf;						// capture token start

		if( (buf = strchr( buf, end_sep )) != NULL ) {			// find token end
			*(buf++) = 0;

			if( end_sep != sep ) {
				buf++;
			}

			if( isspace( sep ) ) {				// treat consec seperators as one if sep is whitespace
				while( *buf == sep ) {
					buf++;
				}
			}
		}
	}

	return t;
}

// ----- file/record management utilities ------------------------------------------------------------

/*
	Read an entire file into a single buffer.
*/
static char* f2b( char* fname ) {
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
	Read a file into a buffer, and set a record buffer to manage it.
*/
static rbuffer_t* f2r( char* fname ) {
	char*		raw; 			// raw buffer
	rbuffer_t*	r;

	if( (raw = f2b( fname )) == NULL ) {
		return NULL;
	}

	r = (rbuffer_t *) malloc( sizeof( *r ) );
	memset( r, 0, sizeof( *r ) );
	r->buffer = raw;
	r->rec = raw;							// point at first (only) record
}

/*
	Return a pointer to the next record in the buffer, or nil if at
	end of buffer.
*/
static char* next_rec( rbuffer_t* r ) {
	char*	rec;

	if( !r || r->at_end ) {
		return NULL;
	}

	rec = r->rec;
	r->rec = strchr( r->rec, '\n' );
	if( r->rec ) {
		*r->rec = 0;
		r->rec++;
		if( *r->rec == 0 ) {
			r->at_end = TRUE;
		}
	} else {
		r->at_end = TRUE;				// mark for next call
	}

	return rec;
}

static void free_rbuf( rbuffer_t* r ) {
	if( r != NULL ) {
		if( r->buffer != NULL ) {
			free( r->buffer );
		}

		free( r );
	}
}


// ----- table management ------------------------------------------------------------

/*
	Run the app list and attempt to open a shunt to any unconnected application.
	Returns the number of applications that could not be connected to.
*/
static int connect2all( master_t* mi ) {
	int	errors = 0;
	int i;


	if( mi == NULL ) {
		return 1;
	}

	for( i = 0; i < mi->napps; i++ ) {
		if( mi->apps[i].state != CONNECTED ) {
			fprintf( stderr, "[INF] opening shunt to: %s:%s\n", mi->apps[i].name, mi->apps[i].port );
			mi->apps[i].shunt = rr_connect( mi->apps[i].name, mi->apps[i].port );
			if( mi->apps[i].shunt == NULL) {
				errors++;
			} else {
				fprintf( stderr, "[INFO] shunt created to: %s:%s\n", mi->apps[i].name, mi->apps[i].port );
				mi->apps[i].state = CONNECTED;
			}
		}
	}

	return errors;
}

/*
	Add an application to the current table.
*/
static void add_app( master_t* mi, char* app_name ) {
	char	wbuf[256];
	char*	app_port;
	char*	ch;

	if( mi == NULL || app_name == NULL ) {
		return;
	}

	if( mi->napps > MAX_APPS ) {
		fprintf( stderr, "[WARN] too many applications, ignoring: %s\n", app_name );
	}

	if( (ch = strchr( app_name, ':' )) == NULL ) {		// assume we are using the default rm listen port
		snprintf( wbuf, sizeof( wbuf ), "%d", mi->port );
		app_port = wbuf;
	} else {
		*(ch++) = 0;									// name port given, split and point at port
		app_port = ch;
	}

	mi->apps[mi->napps].name = strdup( app_name );
	mi->apps[mi->napps].port = strdup( app_port );
	mi->apps[mi->napps].state = !CONNECTED;
	mi->napps++;
}


/*
	Initialise things; returns a master info context.
*/
static master_t* init( char* cfname ) {
	master_t*	mi;
	char		wbuf[128];
	rbuffer_t*	rb;							// record manager for reading config file
	char*		rec;
	tokens_t*	tokens;
	int			i;
	int			errors;
	char*		tok;
	table_t*	table = NULL;
	rrgroup_t*	rrg = NULL;
	entry_t*	entry = NULL;
	int			rec_num = 0;

	mi = (master_t *) malloc( sizeof( *mi ) );
	if( mi == NULL ) {
		return NULL;
	}
	memset( mi, 0, sizeof( *mi ) );
	mi->port = 4561;

	rb = f2r( cfname );						// get a record buffer to parse the config file
	if( rb == NULL ) {
		fprintf( stderr, "[FAIL] unable to open config file: %s: %s\n", cfname, strerror( errno ) );
		free( mi );
		return NULL;
	}

	while( (rec = next_rec( rb )) != NULL ) {
		if( *rec ) {
			rec_num++;
			tokens = tokenise( rec, ' ' );

			fprintf( stderr, "parsing %d: %s\n", rec_num, rec );

			for( i = 0; i < tokens->ntoks && *tokens->tokens[i] != '#'; i++ );			// simple comment strip
			tokens->ntoks = i;

			if( tokens->ntoks > 0 ) {
				tok = tokens->tokens[0];
				switch( *tok ) {					// faster jump table based on 1st ch; strcmp only if needed later
					case '#':
						break;

					case 'e':
						if( table != NULL  && table->nentries < MAX_ENTRIES ) {
							entry = &table->entries[table->nentries++];
							entry->subid = -1;			// no subscription id if user omits
						} else {
							fprintf( stderr, "[ERR] @%d no table started, or table full\n", rec_num );
						}
						break;

					case 'm':
						if( entry != NULL ) {
							entry->mtype = atoi( tokens->tokens[1] );
						} else {
							fprintf( stderr, "[ERR] @%d no entry started\n", rec_num );
						}
						break;

					case 'p':
						if( tokens->ntoks > 1 ) {
							mi->port = atoi( tokens->tokens[1] );
							if( mi->port < 1000 ) {
								fprintf( stderr, "[WRN] @%d assigned default xAPP port smells fishy: %s\n", rec_num, tokens->tokens[1] );
							}
						}
						break;

					case 'r':							// round robin group
						if( entry != NULL && entry->ngroups < MAX_GROUPS ) {
							if( tokens->ntoks < MAX_RRG_SIZE ) {
								rrg = &entry->groups[entry->ngroups++];
	
								for( i = 1; i < tokens->ntoks; i++ ) {
									rrg->apps[rrg->napps++] = strdup( tokens->tokens[i] );
								}
							} else {
								fprintf( stderr, "[ERR] @%d round robin group too big.\n", rec_num );
							}
						} else {
							fprintf( stderr, "[ERR] @%d no previous entry, or entry is full\n", rec_num );
						}
						break;

					case 's':
						if( *(tok+1) == 'e' ) {			// send2
							if( table != NULL && tokens->ntoks < MAX_SEND2  ) {
								table->first_app = mi->napps;

								for( i = 1; i < tokens->ntoks; i++ ) {
									add_app( mi, tokens->tokens[i] );
								}

								table->napps = tokens->ntoks - 1;
							}
						} else {						// subid
							if( entry != NULL ) {
								entry->subid = atoi( tokens->tokens[1] );
							} else {
								fprintf( stderr, "[ERR] @%d no entry started\n", rec_num );
							}
						}
						break;

					case 't':
						entry = NULL;
						if( mi->ntables < MAX_TABLES ) {
							table = &mi->tables[mi->ntables++];
						} else {
							fprintf( stderr, "[ERR] @%d too many tables defined\n", rec_num );
							table = NULL;
						}
						break;

					default:
						fprintf( stderr, "record from config was ignored: %s\n", rec );
						break;
				}
			}
		}
	}

	free_rbuf( rb );
	return mi;
}


/*
	Build a buffer with the entry n from table t. Both entry and table
	numbers are 0 based.
	Caller must free returned buffer.
*/
static char* mk_entry( master_t* mi, int table, int entry ) {
	char		wbuf[4096];
	char		sbuf[256];
	table_t*	tab;
	entry_t*	ent;
	int			i;
	int			j;
	int			len;
	int			alen;

	if( !mi || mi->ntables <= table ) {
		return NULL;
	}

	tab = &mi->tables[table];
	if( tab->nentries <= entry ) {
		return NULL;
	}

	ent = &tab->entries[entry];

	snprintf( wbuf, sizeof( wbuf ), "mse | %d | %d | ", ent->mtype, ent->subid  );			// we only generate mse records

	len = strlen( wbuf );
	for( i = 0; i < ent->ngroups; i++ ) {
		if( i ) {
			strcat( wbuf, "; " );
		}

		for( j = 0; j < ent->groups[i].napps; j++ ) {
			alen = strlen( ent->groups[i].apps[j] ) + 3;				// not percise, but close enough for testing
			if( alen + len > sizeof( wbuf ) ) {
				fprintf( stderr, "[ERR] entry %d for table %d is too large to format\n", entry, table );
				return NULL;
			}

			if( j ) {
				strcat( wbuf, "," );
			}

			strcat( wbuf, ent->groups[i].apps[j] );
		}
	}

	strcat( wbuf, "\n" );
	return strdup( wbuf );
}

/*
	Sends a buffer to all apps in the range.
*/
void send2range( master_t* mi, char* buf, int first, int n2send ) {
	int			a;						// application offset in master array
	int			last;					// stopping point (index)
	rr_mbuf_t*	mbuf = NULL;

	if( !mi  ) {											// safe to dance
		return;
	}

	a = first;
	last = first + n2send;

	mbuf = rr_new_buffer( mbuf, strlen( buf ) + 5 );		// ensure buffer is large enough
	while(  a < last ) {
		fprintf( stderr, "%s ", mi->apps[a].name );

		memcpy( mbuf->payload, buf, strlen( buf ) );
		mbuf->used = strlen( buf );
		mbuf = rr_send( mi->apps[a].shunt, mbuf, ALLOC_NEW );

		a++;

		fprintf( stderr, "\n" );
	}

	rr_free_mbuf( mbuf );
}

/*
	Formats the entries for the table and sends to all applications that the
	table should be sent to per the send2 directive in the config.
*/
static void send_table( master_t* mi, int table ) {
	table_t*	tab;
	char*		ent;				// entry string to send
	int			e = 0;				// entry number
	int			a;
	int			last;
	rr_mbuf_t*	mbuf = NULL;

	if( !mi || table > mi->ntables ) {
		return;
	}

	tab = &mi->tables[table];
	fprintf( stderr, "[INF] send table start message: " );
	send2range( mi, "newrt | start\n", tab->first_app, tab->napps );		// send table start req to all

	while( (ent = mk_entry( mi, table, e++ )) != NULL ) {					// build each entry once, send to all
		fprintf( stderr, "[INF] sending table %d entry %d: ", table, e );
		send2range( mi, ent, tab->first_app, tab->napps );
	}

	fprintf( stderr, "[INF] send table end message: " );
	send2range( mi, "newrt | end\n", tab->first_app, tab->napps );		// send table end notice to all
}

/*
	Run the list of tables and send them all out.
*/
static void send_all_tables( master_t* mi ) {
	int i;

	if( ! mi ) {
		return;
	}

	for( i = 0; i < mi->ntables; i++ ) {
		send_table( mi, i );
	}
}


// ----------------- testing ----------------------------------------------------------------------

/*
	Dump table entries in the form we'll send to apps to stderr.
*/
static void print_tables( master_t* mi ) {
	char* 	ent;
	int		t;
	int		e;

	if( ! mi ) {
		return;
	}

	for( t = 0; t < mi->ntables; t++ ) {
		fprintf( stderr, "=== table %d ===\n", t );
		e = 0;
		while( (ent = mk_entry( mi, t, e++ )) != NULL ) {
			fprintf( stderr, "%s", ent );
			free( ent );
		}
	}
}

static void print_tokens( tokens_t* tokens ) {
	int i;

	fprintf( stderr, "there are %d tokens\n", tokens->ntoks );
	for( i = 0; i < tokens->ntoks; i++ ) {
		fprintf( stderr, "[%02d] (%s)\n", i, tokens->tokens[i] );
	}
}

static void self_test( char* fname ) {
	char*	s0 = "		  Now is   the time for     all    to stand up and cheer!";
	char*	s1 = "		  Now is   \"the time for     all\"    to stand up and cheer!";
	char*	s2 = " field1 | field2||field4|field5";
	tokens_t*	tokens;
	rbuffer_t 	*rb;
	int i;

	tokens = tokenise( s0, ' ' );
	if( tokens->ntoks != 11 ) {
		fprintf( stderr, "didn't parse into 11 tokens (got %d): %s\n", tokens->ntoks, s1 );
	}
	print_tokens( tokens );

	tokens = tokenise( s1, ' ' );
	if( tokens->ntoks != 8 ) {
		fprintf( stderr, "didn't parse into 11 tokens (got %d): %s\n", tokens->ntoks, s1 );
	}
	print_tokens( tokens );

	tokens = tokenise( s2, '|' );
	if( tokens->ntoks != 5 ) {
		fprintf( stderr, "didn't parse into 5 tokens (got %d): %s\n", tokens->ntoks, s2 );
	}
	print_tokens( tokens );

	free_tokens( tokens );

	if( fname == NULL ) {
		return;
	}

	rb = f2r( fname );
	if( rb == NULL ) {
		fprintf( stderr, "[FAIL] couldn't read file into rbuffer: %s\n", strerror( errno ) );
	} else {
		while( (s2 = next_rec( rb )) != NULL ) {
			fprintf( stderr, "record: (%s)\n", s2 );
		}

		free_rbuf( rb );
	}

	return;
}


// ----------------------------------------------------------------------------------------------

int main( int argc, char** argv ) {
	void*	mi;
	int		not_ready;

	if( argc > 1 ) {
		if( strcmp( argv[1], "selftest" ) == 0 ) {
			self_test( argc > 1 ? argv[2] : NULL );
			exit( 0 );
		}

		mi = init( argv[1] );									// parse the config and generate table structs
		if( ! mi ) {
			fprintf( stderr, "[CRI] initialisation failed\n" );
			exit( 1 );
		}

		print_tables( mi );

		while( (not_ready = connect2all( mi ) ) > 0 ) {
			fprintf( stderr, "[INF] still waiting to connect to %d applications\n", not_ready );
			sleep( 2 );
		}

		fprintf( stderr, "[INF] connected to all applications, sending tables\n" );

		send_all_tables( mi );
	} else {
		fprintf( stderr, "[INFO] usage: %s file-name\n", argv[0] );
	}
}


// : vi ts=4 sw=4 noet :
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
------------------------------------------------------------------------------
Mnemonic:	symtab.c
Abstract:	Symbol table -- slightly streamlined from it's original 2000 version
			(a part of the {X}fm opensource code), though we must retain the
			original copyright.

			Things changed for the Ric Msg implemention (Nov 2018):
				- no concept of copy/free of the user data (functions removed)
				- add ability to support an integer key (class 0)
				  Numeric key is an unsigned, 64bit key.
				- externally visible names given a rmr_ extension as it's being
				  incorporated into the RIC msg routing library and will be
				  available to user applications.

Date:		11 Feb 2000
Author:		E. Scott Daniels

Mod:		2016 23 Feb - converted Symtab refs so that caller need only a
				void pointer to use and struct does not need to be exposed.
			2018 30 Nov - Augmented from original form (see above).
------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <netdb.h>

#include "rmr_symtab.h"

//-----------------------------------------------------------------------------------------------

typedef struct Sym_ele
{
	struct Sym_ele *next;   /* pointer at next element in list */
	struct Sym_ele *prev;   /* larger table, easier deletes */
	const char *name;		/* symbol name */
	uint64_t nkey;			// the numeric key
	void *val;              /* user data associated with name */
	unsigned long mcount;   /* modificaitons to value */
	unsigned long rcount;   /* references to symbol */
	//unsigned int flags;
	unsigned int class;		/* helps divide things up and allows for duplicate names */
} Sym_ele;

typedef struct Sym_tab {
	Sym_ele **symlist;			/* pointer to list of element pointerss */
	long	inhabitants;		/* number of active residents */
	long	deaths;				/* number of deletes */
	long	size;
} Sym_tab;

// -------------------- internal ------------------------------------------------------------------

static int sym_hash( const char *n, long size )
{
	const char *p;
	long t = 0;
	unsigned long tt = 0;
	unsigned long x = 79;

	for( p = n; *p; p++ )      /* a bit of magic */
		t = (t * 79 ) + *p;

	if( t < 0 )
		t = ~t;

	return (int ) (t % size);
}

/* delete element pointed to by eptr at hash loc hv */
static void del_ele( Sym_tab *table, int hv, Sym_ele *eptr )
{
	Sym_ele **sym_tab;

	sym_tab = table->symlist;

	if( eptr )         /* unchain it */
	{
		if( eptr->prev )
			eptr->prev->next = eptr->next;
		else
			sym_tab[hv] = eptr->next;

		if( eptr->next )
			eptr->next->prev = eptr->prev;

		if( eptr->class && eptr->name ) {				// class 0 entries are numeric, so name is NOT a pointer
			free( (void *) eptr->name );			// and if free fails, what?  panic?
		}

		free( eptr );

		table->deaths++;
		table->inhabitants--;
	}
}

/*
	Determine if these are the same.
*/
static inline int same( unsigned int c1, unsigned int c2, const char *s1, const char* s2 )
{
	if( c1 != c2 )
		return 0;		/* different class - not the same */

	if( *s1 == *s2 && strcmp( s1, s2 ) == 0 )
		return 1;
	return 0;
}

/*
	Generic routine to put something into the table
	called by sym_map or sym_put since the logic for each is pretty
	much the same.
*/
static int putin( Sym_tab *table, const char *name, unsigned int class, void *val ) {
	Sym_ele *eptr;    		/* pointer into hash table */
	Sym_ele **sym_tab;    	/* pointer into hash table */
	int hv;                 /* hash value */
	int rc = 0;             /* assume it existed */
	uint64_t nkey = 0;		// numeric key if class == 0

	sym_tab = table->symlist;

	if( class ) {								// string key
		hv = sym_hash( name, table->size );		// hash it
		for( eptr=sym_tab[hv]; eptr && ! same( class, eptr->class, eptr->name, name); eptr=eptr->next );
	} else {
		nkey = *((uint64_t *) name);
		hv = nkey % table->size;					// just hash the number
		for( eptr=sym_tab[hv]; eptr && eptr->nkey != nkey; eptr=eptr->next );
	}

	if( ! eptr ) { 			// not found above, so add
		rc++;
		table->inhabitants++;

		eptr = (Sym_ele *) malloc( sizeof( Sym_ele) );
		if( ! eptr ) {
			fprintf( stderr, "[FAIL] symtab/putin: out of memory\n" );
			return -1;
		}

		eptr->prev = NULL;
		eptr->class = class;
		eptr->mcount = eptr->rcount = 0;	/* init counters */
		eptr->val = NULL;
		eptr->nkey = nkey;
		if( class ) {
			eptr->name = strdup( name );
		} else {
			eptr->name = NULL;				// for a numeric key, just save the value
		}
		eptr->next = sym_tab[hv];			// add to head of list
		sym_tab[hv] = eptr;
		if( eptr->next )
			eptr->next->prev = eptr;         /* chain back to new one */
	}

	eptr->mcount++;

	eptr->val = val;

	return rc;
}

// -------------------- visible  ------------------------------------------------------------------

/* delete all elements in the table */
extern void rmr_sym_clear( void *vtable )
{
	Sym_tab *table;
	Sym_ele **sym_tab;
	int i;

	table = (Sym_tab *) vtable;
	sym_tab = table->symlist;

	for( i = 0; i < table->size; i++ )
		while( sym_tab[i] )
			del_ele( table, i, sym_tab[i] );
}

/*
	Clear and then free the whole thing.
*/
extern void rmr_sym_free( void *vtable ) {
	Sym_tab *table;

	table = (Sym_tab *) vtable;

	if( table == NULL )
		return;

	rmr_sym_clear( vtable );
	free( table->symlist );
	free( table );
}

extern void rmr_sym_dump( void *vtable )
{
	Sym_tab *table;
	int i;
	Sym_ele *eptr;
	Sym_ele **sym_tab;

	table = (Sym_tab *) vtable;
	sym_tab = table->symlist;

	for( i = 0; i < table->size; i++ )
	{
		if( sym_tab[i] )
		for( eptr = sym_tab[i]; eptr; eptr = eptr->next )
		{
			if( eptr->val && eptr->class ) {
				fprintf( stderr, "key=%s val@=%p\n", eptr->name, eptr->val );
			} else {
				fprintf( stderr, "nkey=%lu val@=%p\n", (unsigned long) eptr->nkey, eptr->val );
			}
		}
	}
}

/*
	Allocate a table the size requested - best if size is prime.
	Returns a pointer to the management block (handle) or NULL on failure.
*/
extern void *rmr_sym_alloc( int size )
{
	int i;
	Sym_tab *table;

	if( size < 11 )     /* provide a bit of sanity */
		size = 11;

	if( (table = (Sym_tab *) malloc( sizeof( Sym_tab ))) == NULL )
	{
		fprintf( stderr, "rmr_sym_alloc: unable to get memory for symtable (%d elements)", size );
		return NULL;
	}

	memset( table, 0, sizeof( *table ) );

	if((table->symlist = (Sym_ele **) malloc( sizeof( Sym_ele *) * size )))
	{
		memset( table->symlist, 0, sizeof( Sym_ele *) * size );
		table->size = size;
	}
	else
	{
		fprintf( stderr, "sym_alloc: unable to get memory for %d elements", size );
		return NULL;
	}

	return (void *) table;    /* user might want to know what the size is */
}

/*
	Delete an element given name/class or numeric key (class 0).
*/
extern void rmr_sym_del( void *vtable, const char *name, unsigned int class )
{
	Sym_tab	*table;
	Sym_ele **sym_tab;
	Sym_ele *eptr;			/* pointer into hash table */
	int hv;                 /* hash value */
	uint64_t	nkey;		// class 0, name points to integer not string

	table = (Sym_tab *) vtable;
	sym_tab = table->symlist;

	if( class ) {
		hv = sym_hash( name, table->size );
		for(eptr=sym_tab[hv]; eptr &&  ! same(class, eptr->class, eptr->name, name); eptr=eptr->next );
	} else {
		nkey = *((uint64_t *) name);
		hv = nkey % table->size;			// just hash the number
		for( eptr=sym_tab[hv]; eptr && eptr->nkey != nkey; eptr=eptr->next );
	}

	del_ele( table, hv, eptr );    /* ignors null ptr, so safe to always call */
}

/*
	Delete element by numberic key.
*/
extern void *rmr_sym_ndel(  void *vtable, uint64_t key ) {
	rmr_sym_del( vtable, (const char *) &key, 0 );
}


extern void *rmr_sym_get( void *vtable, const char *name, unsigned int class )
{
	Sym_tab	*table;
	Sym_ele **sym_tab;
	Sym_ele *eptr;			// element from table
	int hv;                 // hash value of key
	uint64_t nkey;			// numeric key if class 0

	if( (table = (Sym_tab *) vtable) == NULL ) {
		return NULL;
	}

	sym_tab = table->symlist;

	if( class ) {
		hv = sym_hash( name, table->size );
		for(eptr=sym_tab[hv]; eptr &&  ! same(class, eptr->class, eptr->name, name); eptr=eptr->next );
	} else {
		nkey = *((uint64_t *) name);
		hv = nkey % table->size;			// just hash the number
		for( eptr=sym_tab[hv]; eptr && eptr->nkey != nkey; eptr=eptr->next );
	}

	if( eptr )
	{
		eptr->rcount++;
		return eptr->val;
	}

	return NULL;
}

/*
	Retrieve the data referenced by a numerical key.
*/
extern void *rmr_sym_pull(  void *vtable, uint64_t key ) {
	return rmr_sym_get( vtable, (const char *) &key, 0 );
}

/*
	Put an element with a string key into the table. Replaces the element
	if it was already there.  Class must be >0 and if not 1 will be forced.
	(class 0 keys are numeric).
	Returns 1 if new, 0 if existed.
*/
extern int rmr_sym_put( void *vtable, const char *name, unsigned int class, void *val )
{
	Sym_tab	*table;

	if( class == 0 ) {
		class = 1;
	}

	table = (Sym_tab *) vtable;
	return putin( table, name, class, val );
}

/*
	Add a new entry assuming that the key is an unsigned, 64 bit, integer.

	Returns 1 if new, 0 if existed
*/
extern int rmr_sym_map( void *vtable, uint64_t key, void *val ) {
	Sym_tab	*table;

	table = (Sym_tab *) vtable;
	return putin( table, (const char *) &key, 0, val );
}

/*
	Dump some statistics to stderr dev. Higher level is the more info dumpped
*/
extern void rmr_sym_stats( void *vtable, int level )
{
	Sym_tab	*table;
	Sym_ele *eptr;    /* pointer into the elements */
	Sym_ele **sym_tab;
	int i;
	int empty = 0;
	long ch_count;
	long max_chain = 0;
	int maxi = 0;
	int twoper = 0;

	table = (Sym_tab *) vtable;
	sym_tab = table->symlist;

	for( i = 0; i < table->size; i++ )
	{
		ch_count = 0;
		if( sym_tab[i] )
		{
			for( eptr = sym_tab[i]; eptr; eptr = eptr->next )
			{
				ch_count++;
				if( level > 3 ) {
					if( eptr->class  ) {					// a string key
						fprintf( stderr, "sym: (%d) key=%s val@=%p ref=%ld mod=%lu\n", i, eptr->name, eptr->val, eptr->rcount, eptr->mcount );
					} else {
						fprintf( stderr, "sym: (%d) key=%lu val@=%p ref=%ld mod=%lu\n", i, (unsigned long) eptr->nkey, eptr->val, eptr->rcount, eptr->mcount );
					}
				}
			}
		}
		else
			empty++;

		if( ch_count > max_chain )
		{
			max_chain = ch_count;
			maxi = i;
		}
		if( ch_count > 1 )
			twoper++;

		if( level > 2 )
			fprintf( stderr, "sym: (%d) chained=%ld\n", i, ch_count );
	}

	if( level > 1 )
	{
		fprintf( stderr, "sym: longest chain: idx=%d has %ld elsements):\n", maxi, max_chain );
		for( eptr = sym_tab[maxi]; eptr; eptr = eptr->next ) {
			if( eptr->class ) {
				fprintf( stderr, "\t%s\n", eptr->name );
			} else {
				fprintf( stderr, "\t%lu (numeric key)\n", (unsigned long) eptr->nkey );
			}
		}
	}

	fprintf( stderr, "sym:%ld(size)  %ld(inhab) %ld(occupied) %ld(dead) %ld(maxch) %d(>2per)\n",
			table->size, table->inhabitants, table->size - empty, table->deaths, max_chain, twoper );
}

/*
	Drive a user callback function for each entry in a class. It is safe for
	the user to delete the element as we capture a next pointer before
	calling their function.
*/
extern void rmr_sym_foreach_class( void *vst, unsigned int class, void (* user_fun)( void*, void*, const char*, void*, void* ), void *user_data )
{
	Sym_tab	*st;
	Sym_ele **list;
	Sym_ele *se;
	Sym_ele *next;		/* allows user to delete the node(s) we return */
	int 	i;

	st = (Sym_tab *) vst;

	if( st && (list = st->symlist) != NULL && user_fun != NULL )
		for( i = 0; i < st->size; i++ )
			for( se = list[i]; se; se = next )		/* using next allows user to delet via this */
			{
				next = se->next;
				if( class == se->class ) {
					user_fun( st, se, se->name, se->val, user_data );
				}
			}
}

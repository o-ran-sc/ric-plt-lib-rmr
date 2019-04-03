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
	Mnemonic:	ring_static.c
	Abstract:	Implements a ring of information (probably to act as a 
				message queue).
	Author:		E. Scott Daniels
	Date:		31 August 2017
*/

#ifndef _ring_static_c
#define _ring_static_c

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define RING_FAST 1			// when set we skip nil pointer checks on the ring pointer

/*
	Make a new ring.
*/
static void* uta_mk_ring( int size ) {
	ring_t*	r;
	uint16_t max;

	if( size <= 0 || (r = (ring_t *) malloc( sizeof( *r ) )) == NULL ) {
		return NULL;
	}

	r->head = r->tail = 0;

	max = (r->head - 1);
	if( size >= max ) {
		size--;
	}

	r->nelements = size;		// because we always have an empty element when full
	if( (r->data = (void **) malloc( sizeof( void** ) * (r->nelements + 1) )) == NULL ) {
		free( r );
		return NULL;
	}

	memset( r->data, 0, sizeof( void** ) * r->nelements );
	return (void *) r;
}

/*
	Ditch the ring. The caller is responsible for extracting any remaining
	pointers and freeing them as needed.
*/
static void uta_ring_free( void* vr ) {
	ring_t* r;

	if( (r = (ring_t*) vr) == NULL ) {
		return;
	}

	free( r );
}


/*
	Pull the next data pointer from the ring; null if there isn't
	anything to be pulled.
*/
static inline void* uta_ring_extract( void* vr ) {
	ring_t*		r;
	uint16_t	ti;		// real index in data

	if( !RING_FAST ) {								// compiler should drop the conditional when always false
		if( (r = (ring_t*) vr) == NULL ) {
			return 0;
		}
	} else {
		r = (ring_t*) vr;
	}

	if( r->tail == r->head ) {			// empty ring
		return NULL;
	}

	ti = r->tail;
	r->tail++;
	if( r->tail >= r->nelements ) {
		r->tail = 0;
	}

	return r->data[ti];
}

/*
	Insert the pointer at the next open space in the ring.
	Returns 1 if the inert was ok, and 0 if the ring is full.
*/
static inline int uta_ring_insert( void* vr, void* new_data ) {
	ring_t*		r;

	if( !RING_FAST ) {								// compiler should drop the conditional when always false
		if( (r = (ring_t*) vr) == NULL ) {
			return 0;
		}
	} else {
		r = (ring_t*) vr;
	}

	if( r->head+1 == r->tail || (r->head+1 >= r->nelements && !r->tail) ) {		// ring is full
		return 0;
	}

	r->data[r->head] = new_data;
	r->head++;
	if( r->head >= r->nelements ) {
		r->head = 0;
	}

	return 1;
}



#endif

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
#include <sys/eventfd.h>

#define RING_FAST 1			// when set we skip nil pointer checks on the ring pointer

/*
	This returns the ring's pollable file descriptor. If one does not exist, then
	it is created.
*/
static int uta_ring_getpfd( void* vr ) {
	ring_t*		r;

	if( !RING_FAST ) {								// compiler should drop the conditional when always false
		if( (r = (ring_t*) vr) == NULL ) {
			return 0;
		}
	} else {
		r = (ring_t*) vr;
	}

	if( r->pfd < 0 ) {
		r->pfd = eventfd( 0, EFD_SEMAPHORE | EFD_NONBLOCK );
	}

	return r->pfd;
}

/*
	Make a new ring. The default is to NOT create a lock; if the user 
	wants read locking then uta_config_ring() can be used to setup the
	mutex. (We use several rings internally and the assumption is that
	there is no locking for these.)
*/
static void* uta_mk_ring( int size ) {
	ring_t*	r;
	uint16_t max;

	if( size <= 0 || (r = (ring_t *) malloc( sizeof( *r ) )) == NULL ) {
		return NULL;
	}

	r->rgate = NULL;
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
	r->pfd = eventfd( 0, EFD_SEMAPHORE | EFD_NONBLOCK );		// in semaphore mode counter is maintained with each insert/extract
	return (void *) r;
}

/*
	Allows for configuration of a ring after it has been allocated.
	Options are RING_* options that allow for things like setting/clearing
	read locking. Returns 0 for failure 1 on success.

	Options can be ORd together and all made effective at the same time, but
	it will be impossible to determine a specific failure if invoked this
	way.  Control is returned on the first error, and no provision is made
	to "undo" previously set options if an error occurs.
*/
static int uta_ring_config( void* vr, int options ) {
	ring_t*	r;

	if( (r = (ring_t*) vr) == NULL ) {
		errno = EINVAL;
		return 0;
	}

	if( options & RING_WLOCK ) {
		if( r->wgate == NULL ) {		// don't realloc
			r->wgate = (pthread_mutex_t *) malloc( sizeof( *r->wgate ) );
			if( r->wgate == NULL ) {
				return 0;
			}
	
			pthread_mutex_init( r->wgate, NULL );
		}
	}

	if( options & RING_RLOCK ) {
		if( r->rgate == NULL ) {		// don't realloc
			r->rgate = (pthread_mutex_t *) malloc( sizeof( *r->rgate ) );
			if( r->rgate == NULL ) {
				return 0;
			}
	
			pthread_mutex_init( r->rgate, NULL );
		}
	}

	return 1;
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

	If the read lock exists for the ring, then this will BLOCK until
	it gets the lock.  There is always a chance that once the lock
	is obtained that the ring is empty, so the caller MUST handle
	a nil pointer as the return.
*/
static inline void* uta_ring_extract( void* vr ) {
	ring_t*		r;
	uint16_t	ti;		// real index in data
	int64_t	ctr;		// pfd counter

	if( !RING_FAST ) {								// compiler should drop the conditional when always false
		if( (r = (ring_t*) vr) == NULL ) {
			return 0;
		}
	} else {
		r = (ring_t*) vr;
	}

	if( r->tail == r->head ) {						// empty ring we can bail out quickly
		return NULL;
	}

	if( r->rgate != NULL ) {						// if lock exists we must honour it
		pthread_mutex_lock( r->rgate );
		if( r->tail == r->head ) {					// ensure ring didn't go empty while waiting
			return NULL;
		}
	}

	ti = r->tail;
	r->tail++;
	if( r->tail >= r->nelements ) {
		r->tail = 0;
	}

	read( r->pfd, &ctr, sizeof( ctr )  );				// when not in semaphore, this zeros the counter and value is meaningless
/*
future -- investigate if it's possible only to set/clear when empty or going to empty
	if( r->tail == r->head ) {								// if this emptied the ring, turn off ready
	}
*/

	if( r->rgate != NULL ) {							// if locked above...
		pthread_mutex_unlock( r->rgate );
	}
	return r->data[ti];
}

/*
	Insert the pointer at the next open space in the ring.
	Returns 1 if the inert was ok, and 0 if the ring is full.
*/
static inline int uta_ring_insert( void* vr, void* new_data ) {
	ring_t*		r;
	int64_t	inc = 1;				// used to set the counter in the pfd

	if( !RING_FAST ) {								// compiler should drop the conditional when always false
		if( (r = (ring_t*) vr) == NULL ) {
			return 0;
		}
	} else {
		r = (ring_t*) vr;
	}

	if( r->wgate != NULL ) {						// if lock exists we must honour it
		pthread_mutex_lock( r->wgate );
	}

	if( r->head+1 == r->tail || (r->head+1 >= r->nelements && !r->tail) ) {		// ring is full
		if( r->wgate != NULL ) {					// ensure released if needed
			pthread_mutex_unlock( r->wgate );
		}
		return 0;
	}

	write( r->pfd, &inc, sizeof( inc ) );
/*
future -- investigate if it's possible only to set/clear when empty or going to empty
	if( r->tail == r->head ) {								// turn on ready if ring was empty
	}
*/

	r->data[r->head] = new_data;
	r->head++;
	if( r->head >= r->nelements ) {
		r->head = 0;
	}

	if( r->wgate != NULL ) {						// if lock exists we must unlock before going
		pthread_mutex_unlock( r->wgate );
	}
	return 1;
}



#endif

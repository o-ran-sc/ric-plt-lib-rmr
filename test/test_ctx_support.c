// : vi ts=4 sw=4 noet :
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
	Mnemonic:	test_ctx_support.c
	Abstract:	This is some support for defining dummy contex struct as
				they may need to contian rings and the like for successful
				testing, and it may be too much to call rmr_init() to generate
				one without dragging the whole test under.

	Author:		E. Scott Daniels
	Date:		21 February 2020
*/

#ifndef _test_ctx_support_c
#define _test_ctx_support_c

#ifdef NNG_UNDER_TEST							// add in the nng only things

#include <rmr_nng_private.h>			// nng specific context

static inline uta_ctx_t *mk_dummy_ctx() {
	uta_ctx_t*	ctx;

	ctx = (uta_ctx_t *) malloc( sizeof( *ctx ) );
	if( ctx == NULL ) {
		return NULL;
	}

	memset( ctx, 0, sizeof( *ctx ) );

	return ctx;
}

#else											// assume si is under test

#include <rmr_si_private.h>						// si specific context
#include <ring_static.c>

static inline uta_ctx_t *mk_dummy_ctx() {
	uta_ctx_t*	ctx;

	ctx = (uta_ctx_t *) malloc( sizeof( *ctx ) );
	if( ctx == NULL ) {
		return NULL;
	}

	memset( ctx, 0, sizeof( *ctx ) );

	ctx->mring = uta_mk_ring( 4096 );				// message ring is always on for si
	ctx->zcb_mring = uta_mk_ring( 128 );		// zero copy buffer mbuf ring to reduce malloc/free calls
	ctx->si_ctx = malloc( 1024 );
	ctx->my_name = strdup( "hostname1" );
	ctx->my_ip = strdup( "123.45.67.89" );

	return ctx;
}

#endif

#endif

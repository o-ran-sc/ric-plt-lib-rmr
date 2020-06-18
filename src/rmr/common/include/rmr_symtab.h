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
	Mnemonic:	rmr_symtab.h
	Abstract:	Header file for the symbol table.
	Date:		4 February 2019
	Author:		E. Scott Daniels
*/

#ifndef _rmr_symtab_h
#define _rmr_symtab_h

#ifdef __cplusplus
extern "C" {
#endif


// ------------ prototypes  -----------------------------
extern void rmr_sym_clear( void *s );
extern void rmr_sym_dump( void *s );
extern void *rmr_sym_alloc( int size );
extern void rmr_sym_del( void *s, const char *name, unsigned int st_class );
extern void rmr_sym_ndel( void *vtable, uint64_t key );
extern void rmr_sym_free( void *vtable );
extern void *rmr_sym_get( void *s,  const char *name, unsigned int st_class );
extern int rmr_sym_put( void *s,  const char *name, unsigned int st_class, void *val );
extern int rmr_sym_map( void *s,  uint64_t key, void *val );
extern void *rmr_sym_pull(  void *vtable, uint64_t key );
extern void rmr_sym_stats( void *s, int level );
extern void rmr_sym_foreach_class( void *vst, unsigned int st_class, void (* user_fun)( void*, void*, const char*, void*, void* ), void *user_data );


#ifdef __cplusplus
}
#endif


#endif

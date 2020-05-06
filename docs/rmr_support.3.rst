.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_support 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

RMR support functions 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 #include <rmr/ring_inline.h>
 char* rmr_fib( char* fname );
 int rmr_has_str( char const* buf, char const* str, char sep, int max );
 int rmr_tokenise( char* buf, char** tokens, int max, char sep );
 void* rmr_mk_ring( int size );
 void rmr_ring_free( void* vr );
 static inline void* rmr_ring_extract( void* vr )
 static inline int rmr_ring_insert( void* vr, void* new_data )
 


1.3. DESCRIPTION
----------------

These functions support the RMR library, and are made 
available to user applications as some (e.g. route table 
generators) might need and/or want to make use of them. The 
``rmr_fib`` function accepts a file name and reads the entire 
file into a single buffer. The intent is to provide an easy 
way to load a static route table without a lot of buffered 
I/O hoops. 
 
The ``rmr_has_str`` function accepts a *buffer* containing a 
set of delimited tokens (e.g. foo,bar,goo) and returns true 
if the target string, *str,* matches one of the tokens. The 
*sep* parameter provides the separation character in the 
buffer (e.g a comma) and *max* indicates the maximum number 
of tokens to split the buffer into before checking. 
 
The ``rmr_tokenise`` function is a simple tokeniser which 
splits *buf* into tokens at each occurrence of *sep*. 
Multiple occurrences of the separator character (e.g. a,,b) 
result in a nil token. Pointers to the tokens are placed into 
the *tokens* array provided by the caller which is assumed to 
have at least enough space for *max* entries. 
 
The ``rmr_mk_ring`` function creates a buffer ring with 
*size* entries. 
 
The ``rmr_ring_free`` function accepts a pointer to a ring 
context and frees the associated memory. 
 
The ``rmr_ring_insert`` and ``rmr_ring_extract`` functions 
are provided as static inline functions via the 
*rmr/ring_inline.h* header file. These functions both accept 
the ring *context* returned by ``mk_ring,`` and either insert 
a pointer at the next available slot (tail) or extract the 
data at the head. 


1.4. RETURN VALUES
------------------

The following are the return values for each of these 
functions. 
 
The ``rmr_fib`` function returns a pointer to the buffer 
containing the contents of the file. The buffer is terminated 
with a single nil character (0) making it a legitimate C 
string. If the file was empty or nonexistent, a buffer with 
an immediate nil character. If it is important to the calling 
programme to know if the file was empty or did not exist, the 
caller should use the system stat function call to make that 
determination. 
 
The ``rmr_has_str`` function returns 1 if *buf* contains the 
token referenced by &ita and false (0) if it does not. On 
error, a -1 value is returned and ``errno`` is set 
accordingly. 
 
The ``rmr_tokenise`` function returns the actual number of 
token pointers placed into *tokens* 
 
The ``rmr_mk_ring`` function returns a void pointer which is 
the *context* for the ring. 
 
The ``rmr_ring_insert`` function returns 1 if the data was 
successfully inserted into the ring, and 0 if the ring is 
full and the pointer could not be deposited. 
 
The ``rmr_ring_extract`` will return the data which is at the 
head of the ring, or NULL if the ring is empty. 


1.5. ERRORS
-----------

Not many of these functions set the value in ``errno,`` 
however the value may be one of the following: 
 
INVAL 
  Parameter(s) passed to the function were not valid. 


1.6. EXAMPLE
------------



1.7. SEE ALSO
-------------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 

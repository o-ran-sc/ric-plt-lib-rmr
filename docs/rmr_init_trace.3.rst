 
 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
============================================================================================ 
Man Page: rmr_init_trace 
============================================================================================ 
 
RMR Library Functions 
============================================================================================ 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_init_trace 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void* rmr_init_trace( void* ctx )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_init_trace function establishes the default trace 
space placed in each message buffer allocated with 
rmr_alloc_msg(). If this function is never called, then no 
trace space is allocated by default into any message buffer. 
 
Trace space allows the user application to pass some trace 
token, or other data with the message, but outside of the 
payload. Trace data may be added to any message with 
rmr_set_trace(), and may be extracted from a message with 
rmr_get_trace(). The number of bytes that a message contains 
for/with trace data can be determined by invoking 
rmr_get_trlen(). 
 
This function may be safely called at any time during the 
life of the user programme to (re)set the default trace space 
reserved. If the user programme needs to allocate a message 
with trace space of a different size than is allocated by 
default, without fear of extra overhead of reallocating a 
message later, the rmr_tralloc_msg() function can be used. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
A value of 1 is returned on success, and 0 on failure. A 
failure indicates that the RMr context (a void pointer passed 
to this function was not valid. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_tr_alloc_msg(3), rmr_call(3), 
rmr_free_msg(3), rmr_get_rcvfd(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_set_trace(3) 

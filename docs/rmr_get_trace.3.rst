.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_get_trace 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_trace 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 int rmr_get_trace( rmr_mbuf_t* mbuf, unsigned char* dest, int size )
 


DESCRIPTION
-----------

The ``rmr_get_trace`` function will copy the trace 
information from the message into the user's allocated memory 
referenced by ``dest.`` The ``size`` parameter is assumed to 
be the maximum number of bytes which can be copied (size of 
the destination buffer). 


RETURN VALUE
------------

On success, the number of bytes actually copied is returned. 
If the return value is 0, no bytes copied, then the reason 
could be that the message pointer was nil, or the size 
parameter was <= 0. 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_tralloc_msg(3), rmr_bytes2xact(3), 
rmr_bytes2meid(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_get_trlen(3), rmr_init(3), 
rmr_init_trace(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3), 
rmr_set_trace(3), rmr_trace_ref(3) 

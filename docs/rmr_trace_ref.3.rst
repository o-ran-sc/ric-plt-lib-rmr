.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_trace_ref 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_trace_ref 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 int rmr_trace_ref( rmr_mbuf_t* mbuf, int* sizeptr )
 


DESCRIPTION
-----------

The ``rmr_trace_ref`` function returns a pointer to the trace 
area in the message, and optionally populates the user 
programme supplied size integer with the trace area size, if 
*sizeptr* is not nil. 


RETURN VALUE
------------

On success, a void pointer to the trace area of the message 
is returned. A nil pointer is returned if the message has no 
trace data area allocated, or if the message itself is 
invalid. 


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
rmr_set_trace(3) 

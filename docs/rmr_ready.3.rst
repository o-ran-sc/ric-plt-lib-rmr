.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_ready 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_ready 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 int rmr_ready( void* vctx );
 


DESCRIPTION
-----------

The ``rmr_ready`` function checks to see if a routing table 
has been successfully received and installed. The return 
value indicates the state of readiness. 


RETURN VALUE
------------

A return value of 1 (true) indicates that the routing table 
is in place and attempts to send messages can be made. When 0 
is returned (false) the routing table has not been received 
and thus attempts to send messages will fail with *no 
endpoint* errors. 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_fib(3), 
rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 

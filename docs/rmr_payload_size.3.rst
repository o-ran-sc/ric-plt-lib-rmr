.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_payload_size 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

rmr_payload_size 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 int rmr_payload_size( rmr_mbuf_t* msg );
 


1.3. DESCRIPTION
----------------

Given a message buffer, this function returns the amount of 
space (bytes) available for the user application to consume 
in the message payload. This is different than the message 
length available as a field in the message buffer. 


1.4. RETURN VALUE
-----------------

The number of bytes available in the payload. 


1.5. ERRORS
-----------

 
INVAL 
  Parameter(s) passed to the function were not valid. 


1.6. SEE ALSO
-------------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 

.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_close 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

rmr_close 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 void rmr_close( void* vctx )
 


1.3. DESCRIPTION
----------------

The ``rmr_close`` function closes the listen socket 
effectively cutting the application off. The route table 
listener is also stopped. Calls to rmr_rcv_msg() will fail 
with unpredictable error codes, and calls to rmr_send_msg(), 
rmr_call(), and rmr_rts_msg() will have unknown results. 
 


1.4. SEE ALSO
-------------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_open(3), 
rmr_wh_send_msg(3) 

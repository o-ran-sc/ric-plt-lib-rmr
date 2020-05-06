.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_bytes2payload 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

rmr_bytes2payload 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 void rmr_bytes2payload( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 


1.3. DESCRIPTION
----------------

This is a convenience function as some wrapper languages 
might not have the ability to directly copy into the payload 
buffer. The bytes from *src* for the length given are copied 
to the payload. It is the caller's responsibility to ensure 
that the payload is large enough. Upon successfully copy, the 
``len`` field in the message buffer is updated to reflect the 
number of bytes copied. 
 
There is little error checking, and no error reporting. 


1.4. RETURN VALUE
-----------------

None. 


1.5. EXAMPLE
------------



1.6. SEE ALSO
-------------

rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2payload(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_meid(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 

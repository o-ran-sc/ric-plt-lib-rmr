.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_get_srcip 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_srcip 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 unsigned char* rmr_get_srcip( rmr_mbuf_t* mbuf, unsigned char* dest )
 


DESCRIPTION
-----------

The ``rmr_get_srcip`` function will copy the *source IP 
address* from the message to a buffer (dest) supplied by the 
user. In an RMR message, the source IP address is the 
sender's information that is used for return to sender 
function calls; this function makes it available to the user 
application. The address is maintained as IP:port where *IP* 
could be either an IPv6 or IPv4 address depending on what was 
provided by the sending application. 
 
The maximum size allowed by RMR is 64 bytes (including the 
nil string terminator), so the user must ensure that the 
destination buffer given is at least 64 bytes. The user 
application should use the RMR constant RMR_MAX_SRC to ensure 
that the buffer supplied is large enough, and to protect 
against future RMR enhancements which might increase the 
address buffer size requirement. 


RETURN VALUE
------------

On success, a pointer to the destination buffer is given as a 
convenience to the user programme. On failure, a nil pointer 
is returned and the value of errno is set. 


ERRORS
------

If an error occurs, the value of the global variable 
``errno`` will be set to one of the following with the 
indicated meaning. 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
      
     * - **EINVAL** 
       - 
         The message, or an internal portion of the message, was 
         corrupted or the pointer was invalid. 
          
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_src(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 

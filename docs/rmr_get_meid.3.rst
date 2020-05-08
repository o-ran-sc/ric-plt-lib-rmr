.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_get_meid 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_meid 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 char* rmr_get_meid( rmr_mbuf_t* mbuf, unsigned char* dest )
 


DESCRIPTION
-----------

The ``rmr_get_meid`` function will copy the managed entity ID 
(meid) field from the message into the *dest* buffer provided 
by the user. The buffer referenced by *dest* is assumed to be 
at least ``RMR_MAX_MEID`` bytes in length. If *dest* is NULL, 
then a buffer is allocated (the calling application is 
expected to free when the buffer is no longer needed). 


RETURN VALUE
------------

On success, a pointer to the extracted string is returned. If 
*dest* was supplied, then this is just a pointer to the 
caller's buffer. If *dest* was NULL, this is a pointer to the 
allocated buffer. If an error occurs, a nil pointer is 
returned and errno is set as described below. 


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
          
          
         | 
      
     * - **ENOMEM** 
       - 
         A nil pointer was passed for *dest,* however it was not 
         possible to allocate a buffer using malloc(). 
          
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_xact(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 

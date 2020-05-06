.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_realloc_payload 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_realloc_payload 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 extern rmr_mbuf_t* rmr_realloc_payload( rmr_mbuf_t* msg, int new_len, int copy, int clone );
 


DESCRIPTION
-----------

The ``rmr_realloc_payload`` function will return a pointer to 
an RMR message buffer struct (rmr_mbuf_t) which has a payload 
large enough to accomodate *new_len* bytes. If necessary, the 
underlying payload is reallocated, and the bytes from the 
original payload are copied if the *copy* parameter is true 
(1). If the message passed in has a payload large enough, 
there is no additional memory allocation and copying. 


Cloning The Message Buffer
--------------------------

This function can also be used to generate a separate copy of 
the original message, with the desired payload size, without 
destroying the original message buffer or the original 
payload. A standalone copy is made only when the *clone* 
parameter is true (1). When cloning, the payload is copied to 
the cloned message **only** if the *copy* parameter is true. 


Message Buffer Metadata
-----------------------

The metadata in the original message buffer (message type, 
subscription ID, and payload length) will be preserved if the 
*copy* parameter is true. When this parameter is not true 
(0), then these values are set to the uninitialised value 
(-1) for type and ID, and the length is set to 0. 


RETURN VALUE
------------

The ``rmr_realloc_payload`` function returns a pointer to the 
message buffer with the payload which is large enough to hold 
*new_len* bytes. If the *clone* option is true, this will be 
a pointer to the newly cloned message buffer; the original 
message buffer pointer may still be used to reference that 
message. It is the calling application's responsibility to 
free the memory associateed with both messages using the 
rmr_free_msg() function. 
 
When the *clone* option is not used, it is still good 
practice by the calling application to capture and use this 
reference as it is possible that the message buffer, and not 
just the payload buffer, was reallocated. In the event of an 
error, a nil pointer will be returned and the value of 
*errno* will be set to reflect the problem. 


ERRORS
------

These value of *errno* will reflect the error condition if a 
nil pointer is returned: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **ENOMEM** 
       - 
         Memory allocation of the new payload failed. 
          
         | 
      
     * - **EINVAL** 
       - 
         The pointer passed in was nil, or referenced an invalid 
         message, or the required length was not valid. 
          
 


EXAMPLE
-------

The following code bit illustrates how this function can be 
used to reallocate a buffer for a return to sender 
acknowledgement message which is larger than the message 
received. 
 
 
:: 
 
   if( rmr_payload_size( msg ) < ack_sz ) {              // received message too small for ack
     msg = rmr_realloc_payload( msg, ack_sz, 0, 0 );     // reallocate the message with a payload big enough
     if( msg == NULL ) {
       fprintf( stderr, "[ERR] realloc returned a nil pointer: %s\\n", strerror( errno ) );
     } else {
       // populate and send ack message
     }
 }
  
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_set_stimeout(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 

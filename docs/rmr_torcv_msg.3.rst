.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_torcv_msg 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_torcv_msg 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_torcv_msg( void* vctx, rmr_mbuf_t* old_msg, int ms_to );
 


DESCRIPTION
-----------

The ``rmr_torcv_msg`` function will pause for *ms_to* 
milliseconds waiting for a message to arrive. If a message 
arrives before the timeout expires the message buffer 
returned will have a status of RMR_OK and the payload will 
contain the data received. If the timeout expires before the 
message is received, the status will have the value 
RMR_ERR_TIMEOUT. When a received message is returned the 
message buffer will also contain the message type and length 
set by the sender. If messages were queued while waiting for 
the response to a previous invocation of ``rmr_call,`` the 
oldest message is removed from the queue and returned without 
delay. 
 
The *vctx* pointer is the pointer returned by the 
``rmr_init`` function. *Old_msg* is a pointer to a previously 
used message buffer or NULL. The ability to reuse message 
buffers helps to avoid alloc/free cycles in the user 
application. When no buffer is available to supply, the 
receive function will allocate one. 


RETURN VALUE
------------

The function returns a pointer to the ``rmr_mbuf_t`` 
structure which references the message information (state, 
length, payload), or a nil pointer in the case of an extreme 
error. 


ERRORS
------

The *state* field in the message buffer will be one of the 
following: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_OK** 
       - 
         The message buffer (payload) references the received data. 
          
          
         | 
      
     * - **RMR_ERR_INITFAILED** 
       - 
         The first call to this function must initialise an underlying 
         system notification mechanism. On failure, this error is 
         returned and errno will have the system error status set. If 
         this function fails to intialise, the poll mechansim, it is 
         likely that message receives will never be successful. 
          
          
         | 
      
     * - **RMR_ERR_TIMEOUT** 
       - 
         The timeout expired before a complete message was received. 
         All other fields in the message buffer are not valid. 
          
          
         | 
      
     * - **RMR_ERR_EMPTY** 
       - 
         A message was received, but it had no payload. All other 
         fields in the message buffer are not valid. 
          
 
 
Upon return the system error number, *errno* might be set 
with a value that can help to explain the meaning of the 
state indicated in the message. The following are possible: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **INVAL** 
       - 
         Parameter(s) passed to the function were not valid. 
          
          
         | 
      
     * - **EBADF** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **ENOTSUP** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **EFSM** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **EAGAIN** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **EINTR** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **ETIMEDOUT** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
          
         | 
      
     * - **ETERM** 
       - 
         The underlying message transport is unable to process the 
         request. 
          
 


EXAMPLE
-------



SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_init(3), rmr_payload_size(3), 
rmr_rcv_msg(3), rmr_send_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 

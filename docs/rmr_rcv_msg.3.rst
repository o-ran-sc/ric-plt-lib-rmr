.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_rcv_msg 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_rcv_msg 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_rcv_msg( void* vctx, rmr_mbuf_t* old_msg );
 


DESCRIPTION
-----------

The ``rmr_rcv_msg`` function blocks until a message is 
received, returning the message to the caller via a pointer 
to a ``rmr_mbuf_t`` structure type. If messages were queued 
while waiting for the response to a previous invocation of 
``rmr_call,`` the oldest message is removed from the queue 
and returned without delay. 
 
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

The *state* field in the message buffer will indicate 
``RMR_OK`` when the message receive process was successful 
and the message can be used by the caller. Depending on the 
underlying transport mechanism, one of the following RMR 
error stats may be returned: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_ERR_EMPTY** 
       - 
         The message received had no payload, or was completely empty. 
          
          
         | 
      
     * - **RMR_ERR_TIMEOUT** 
       - 
         For some transport mechanisms, or if reading the receive 
         queue from multiple threads, it is possible for one thread to 
         find no data waiting when it queries the queue. When this 
         state is reported, the message buffer does not contain 
         message data and the user application should reinvoke the 
         receive function. 
          
 
 
When an RMR error state is reported, the underlying 
``errno`` value might provide more information. The following 
is a list of possible values that might accompany the states 
listed above: 
 
``RMR_ERR_EMPTY`` if an empty message was received. If a nil 
pointer is returned, or any other state value was set in the 
message buffer, ``errno`` will be set to one of the 
following: 
 
 
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
rmr_get_rcvfd(3), rmr_init(3), rmr_mk_ring(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_torcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_ring_free(3), rmr_torcv_msg(3) 

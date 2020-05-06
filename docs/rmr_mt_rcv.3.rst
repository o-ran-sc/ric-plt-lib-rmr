.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_mt_rcv 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_mt_rcv 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_mt_rcv( void* vctx, rmr_mbuf_t* old_msg, int timeout );
 


DESCRIPTION
-----------

The ``rmr_mt_rcv`` function blocks until a message is 
received, or the timeout period (milliseconds) has passed. 
The result is an RMR message buffer which references a 
received message. In the case of a timeout the state will be 
reflected in an "empty buffer" (if old_msg was not nil, or 
simply with the return of a nil pointer. If a timeout value 
of zero (0) is given, then the function will block until the 
next message received. 
 
The *vctx* pointer is the pointer returned by the 
``rmr_init`` function. *Old_msg* is a pointer to a previously 
used message buffer or NULL. The ability to reuse message 
buffers helps to avoid alloc/free cycles in the user 
application. When no buffer is available to supply, the 
receive function will allocate one. 
 
The *old_msg* parameter allows the user to pass a previously 
generated RMR message back to RMR for reuse. Optionally, the 
user application may pass a nil pointer if no reusable 
message is available. When a timeout occurs, and old_msg was 
not nil, the state will be returned by returning a pointer to 
the old message with the state set. 
 
It is possible to use the *rmr_rcv_msg()* function instead of 
this function. Doing so might be advantageous if the user 
programme does not always start the multi-threaded mode and 
the use of *rmr_rcv_msg()* would make the flow of the code 
more simple. The advantages of using this function are the 
ability to set a timeout without using epoll, and a small 
performance gain (if multi-threaded mode is enabled, and the 
*rmr_rcv_msg()* function is used, it simply invokes this 
function without a timeout value, thus there is the small 
cost of a second call that results). Similarly, the 
*rmr_torcv_msg()* call can be used when in multi-threaded 
mode with the same "pass through" overhead to using this 
function directly. 


RETURN VALUE
------------

When a message is received before the timeout period expires, 
a pointer to the RMR message buffer which describes the 
message is returned. This will, with a high probability, be a 
different message buffer than *old_msg;* the user application 
should not continue to use *old_msg* after it is passed to 
this function. 
 
In the event of a timeout the return value will be the old 
msg with the state set, or a nil pointer if no old message 
was provided. 


ERRORS
------

The *state* field in the message buffer will be set to one of 
the following values: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_OK** 
       - 
         The message was received without error. 
          
         | 
      
     * - **RMR_ERR_BADARG** 
       - 
         A parameter passed to the function was not valid (e.g. a nil 
         pointer). indicate either ``RMR_OK`` or ``RMR_ERR_EMPTY`` if 
         an empty message was received. 
          
         | 
      
     * - **RMR_ERR_EMPTY** 
       - 
         The message received had no associated data. The length of 
         the message will be 0. 
          
         | 
      
     * - **RMR_ERR_NOTSUPP** 
       - 
         The multi-threaded option was not enabled when RMR was 
         initialised. See the man page for *rmr_init()* for details. 
          
         | 
      
     * - **RMR_ERR_RCVFAILED** 
       - 
         A hard error occurred preventing the receive from completing. 
          
 
When a nil pointer is returned, or any other state value was 
set in the message buffer, ``errno`` will be set to one of 
the following: 
 
 
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

 
 
:: 
 
     rmr_mbuf_t*  mbuf = NULL;   // received msg
  
     msg = rmr_mt_recv( mr, mbuf, 100 );     // wait up to 100ms
     if( msg != NULL ) {
         switch( msg->state ) {
             case RMR_OK:
                 printf( "got a good message\\n" );
                 break;
  
             case RMR_ERR_EMPTY:
                 printf( "received timed out\\n" );
                 break;
  
             default:
                 printf( "receive error: %d\\n", mbuf->state );
                 break;
         }
     } else {
         printf( "receive timeout (nil)\\n" );
     }
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_init(3), rmr_mk_ring(3), 
rmr_mt_call(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_torcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_ring_free(3), rmr_torcv_msg(3) 

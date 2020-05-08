.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_call 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_call 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 extern rmr_mbuf_t* rmr_call( void* vctx, rmr_mbuf_t* msg );
 


DESCRIPTION
-----------

The ``rmr_call`` function sends the user application message 
to a remote endpoint, and waits for a corresponding response 
message before returning control to the user application. The 
user application supplies a completed message buffer, as it 
would for a ``rmr_send`` call, but unlike with the send, the 
buffer returned will have the response from the application 
that received the message. 
 
Messages which are received while waiting for the response 
are queued internally by RMR, and are returned to the user 
application when ``rmr_rcv_msg`` is invoked. These messages 
are returned in the order received, one per call to 
``rmr_rcv_msg.`` 


Call Timeout
------------

The ``rmr_call`` function implements a timeout failsafe to 
prevent, in most cases, the function from blocking forever. 
The timeout period is **not** based on time (calls to clock 
are deemed too expensive for a low latency system level 
library), but instead the period is based on the number of 
received messages which are not the response. Using a 
non-time mechanism for *timeout* prevents the async queue 
from filling (which would lead to message drops) in an 
environment where there is heavy message traffic. 
 
When the threshold number of messages have been queued 
without receiving a response message, control is returned to 
the user application and a nil pointer is returned to 
indicate that no message was received to process. Currently 
the threshold is fixed at 20 messages, though in future 
versions of the library this might be extended to be a 
parameter which the user application may set. 


Retries
-------

The send operations in RMR will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 &item The message is sent without error 
  
 &item The underlying transport reports a *hard* failure 
  
 &item The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attempts 
**without** any intervening calls to *sleep()* or *usleep().* 
The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMR 
initialisation using the *rmr_set_stimeout()* function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 


Transport Level Blocking
------------------------

The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremely likely that retry 
conditions will happen during normal operations. These are 
completely out of RMR's control, and there is nothing that 
RMR can do to avoid or mitigate these other than by allowing 
RMR to retry the send operation, and even then it is possible 
(e.g., during connection reattempts), that a single retry 
loop is not enough to guarantee a successful send. 


RETURN VALUE
------------

The ``rmr_call`` function returns a pointer to a message 
buffer with the state set to reflect the overall state of 
call processing (see Errors below). In some cases a nil 
pointer will be returned; when this is the case only *errno* 
will be available to describe the reason for failure. 


ERRORS
------

These values are reflected in the state field of the returned 
message. 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_OK** 
       - 
         The call was successful and the message buffer references the 
         response message. 
          
          
         | 
      
     * - **RMR_ERR_CALLFAILED** 
       - 
         The call failed and the value of *errno,* as described below, 
         should be checked for the specific reason. 
          
 
 
The global "variable" *errno* will be set to one of the 
following values if the overall call processing was not 
successful. 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **ETIMEDOUT** 
       - 
         Too many messages were queued before receiving the expected 
         response 
          
          
         | 
      
     * - **ENOBUFS** 
       - 
         The queued message ring is full, messages were dropped 
          
          
         | 
      
     * - **EINVAL** 
       - 
         A parameter was not valid 
          
          
         | 
      
     * - **EAGAIN** 
       - 
         The underlying message system was interrupted or the device 
         was busy; the message was **not** sent, and the user 
         application should call this function with the message again. 
          
 


EXAMPLE
-------

The following code snippet shows one way of using the 
``rmr_call`` function, and illustrates how the transaction ID 
must be set. 
 
 
:: 
 
     int retries_left = 5;               // max retries on dev not available
     int retry_delay = 50000;            // retry delay (usec)
     static rmr_mbuf_t*  mbuf = NULL;    // response msg
     msg_t*  pm;                         // application struct for payload
  
     // get a send buffer and reference the payload
     mbuf = rmr_alloc_msg( mr, sizeof( pm->req ) );
     pm = (msg_t*) mbuf->payload;
  
     // generate an xaction ID and fill in payload with data and msg type
     snprintf( mbuf->xaction, RMR_MAX_XID, "%s", gen_xaction() );
     snprintf( pm->req, sizeof( pm->req ), "{ \\"req\\": \\"num users\\"}" );
     mbuf->mtype = MT_REQ;
  
     msg = rmr_call( mr, msg );
     if( ! msg ) {               // probably a timeout and no msg received
         return NULL;            // let errno trickle up
     }
  
     if( mbuf->state != RMR_OK ) {
         while( retries_left-- > 0 &&             // loop as long as eagain
                errno == EAGAIN &&
                (msg = rmr_call( mr, msg )) != NULL &&
                mbuf->state != RMR_OK ) {
  
             usleep( retry_delay );
         }
  
         if( mbuf == NULL || mbuf->state != RMR_OK ) {
             rmr_free_msg( mbuf );        // safe if nil
             return NULL;
         }
     }
  
     // do something with mbuf
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_set_stimeout(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 

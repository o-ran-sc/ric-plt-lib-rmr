.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_mt_call 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_mt_call 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 extern rmr_mbuf_t* rmr_mt_call( void* vctx, rmr_mbuf_t* msg, int id, int timeout );
 


DESCRIPTION
-----------

The ``rmr_mt_call`` function sends the user application 
message to a remote endpoint, and waits for a corresponding 
response message before returning control to the user 
application. The user application supplies a completed 
message buffer, as it would for a ``rmr_send_msg`` call, but 
unlike with a send, the buffer returned will have the 
response from the application that received the message. The 
thread invoking the *rmr_mt_call()* will block until a 
message arrives or until *timeout* milliseconds has passed; 
which ever comes first. Using a timeout value of zero (0) 
will cause the thread to block without a timeout. 
 
The *id* supplied as the third parameter is an integer in the 
range of 2 through 255 inclusive. This is a caller defined 
"thread number" and is used to match the response message 
with the correct user application thread. If the ID value is 
not in the proper range, the attempt to make the call will 
fail. 
 
Messages which are received while waiting for the response 
are queued on a *normal* receive queue and will be delivered 
to the user application with the next invocation of 
*rmr_mt_rcv()* or *rmr_rvv_msg().* by RMR, and are returned 
to the user application when ``rmr_rcv_msg`` is invoked. 
These messages are returned in the order received, one per 
call to ``rmr_rcv_msg.`` 


The Transaction ID
------------------

The user application is responsible for setting the value of 
the transaction ID field before invoking *rmr_mt_call.* The 
transaction ID is a ``RMR_MAX_XID`` byte field that is used 
to match the response message when it arrives. RMR will 
compare **all** of the bytes in the field, so the caller must 
ensure that they are set correctly to avoid missing the 
response message. The application which returns the response 
message is also expected to ensure that the return buffer has 
the matching transaction ID. This can be done transparently 
if the application uses the *rmr_rts_msg()* function and does 
not adjust the transaction ID. 


Retries
-------

The send operations in RMR will retry *soft* send failures 
until one of three conditions occurs: 
 
 
* The message is sent without error 
  
* The underlying transport reports a *hard* failure 
  
* The maximum number of retry loops has been attempted 
 
 
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

The ``rmr_mt_call`` function returns a pointer to a message 
buffer with the state set to reflect the overall state of 
call processing. If the state is ``RMR_OK`` then the buffer 
contains the response message; otherwise the state indicates 
the error encountered while attempting to send the message. 
 
If no response message is received when the timeout period 
has expired, a nil pointer will be returned (NULL). 


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
      
     * - **RMR_ERR_BADARG** 
       - 
         An argument passed to the function was invalid. 
      
     * - **RMR_ERR_CALLFAILED** 
       - 
         The call failed and the value of *errno,* as described below, 
         should be checked for the specific reason. 
      
     * - **RMR_ERR_NOENDPT** 
       - 
         An endpoint associated with the message type could not be 
         found in the route table. 
      
     * - **RMR_ERR_RETRY** 
       - 
         The underlying transport mechanism was unable to accept the 
         message for sending. The user application can retry the call 
         operation if appropriate to do so. 
          
 
 
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
      
     * - **ENOBUFS** 
       - 
         The queued message ring is full, messages were dropped 
      
     * - **EINVAL** 
       - 
         A parameter was not valid 
      
     * - **EAGAIN** 
       - 
         The underlying message system wsa interrupted or the device 
         was busy; the message was **not** sent, and user application 
         should call this function with the message again. 
          
 


EXAMPLE
-------

The following code bit shows one way of using the 
``rmr_mt_call`` function, and illustrates how the transaction 
ID must be set. 
 
 
:: 
 
     int retries_left = 5;               // max retries on dev not available
     static rmr_mbuf_t*  mbuf = NULL;    // response msg
     msg_t*  pm;                         // appl message struct (payload)
  
     // get a send buffer and reference the payload
     mbuf = rmr_alloc_msg( mr, sizeof( pm->req ) );
     pm = (msg_t*) mbuf->payload;
  
     // generate an xaction ID and fill in payload with data and msg type
     rmr_bytes2xact( mbuf, xid, RMR_MAX_XID );
     snprintf( pm->req, sizeof( pm->req ), "{ \\"req\\": \\"num users\\"}" );
     mbuf->mtype = MT_USR_RESP;
  
     msg = rmr_mt_call( mr, msg, my_id, 100 );        // wait up to 100ms
     if( ! msg ) {               // probably a timeout and no msg received
         return NULL;            // let errno trickle up
     }
  
     if( mbuf->state != RMR_OK ) {
         while( retries_left-- > 0 &&             // loop as long as eagain
                mbuf->state == RMR_ERR_RETRY &&
                (msg = rmr_mt_call( mr, msg )) != NULL &&
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
rmr_mt_rcv(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_set_stimeout(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 

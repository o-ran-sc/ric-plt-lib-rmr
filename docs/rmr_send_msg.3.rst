.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_send_msg 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_send_msg 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_send_msg( void* vctx, rmr_mbuf_t* msg );
 


DESCRIPTION
-----------

The ``rmr_send_msg`` function accepts a message buffer from 
the user application and attempts to send it. The destination 
of the message is selected based on the message type 
specified in the message buffer, and the matching information 
in the routing tables which are currently in use by the RMR 
library. This may actually result in the sending of the 
message to multiple destinations which could degrade expected 
overall performance of the user application. (Limiting 
excessive sending of messages is the responsibility of the 
application(s) responsible for building the routing table 
used by the RMR library, and not the responsibility of the 
library.) 


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

On success, a new message buffer, with an empty payload, is 
returned for the application to use for the next send. The 
state in this buffer will reflect the overall send operation 
state and will be ``RMR_OK`` when the send was successful. 
 
When the message cannot be successfully sent this function 
will return the unsent (original) message buffer with the 
state set to indicate the reason for failure. The value of 
*errno* may also be set to reflect a more detailed failure 
reason if it is known. 
 
In the event of extreme failure, a nil pointer is returned. 
In this case the value of ``errno`` might be of some use, for 
documentation, but there will be little that the user 
application can do other than to move on. 
 
**CAUTION:** In some cases it is extremely likely that the 
message returned by the send function does **not** reference 
the same memory structure. Thus is important for the user 
programme to capture the new pointer for future use or to be 
passed to ``rmr_free().`` If you are experiencing either 
double free errors or segment faults in either 
``rmr_free()`` or ``rmr_send_msg(),`` ensure that the return 
value from this function is being captured and used. 


ERRORS
------

The following values may be passed back in the *state* field 
of the returned message buffer. 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_RETRY** 
       - 
         The message could not be sent, but the underlying transport 
         mechanism indicates that the failure is temporary. If the 
         send operation is tried again it might be successful. 
      
     * - **RMR_SEND_FAILED** 
       - 
         The send operation was not successful and the underlying 
         transport mechanism indicates a permanent (hard) failure; 
         retrying the send is not possible. 
      
     * - **RMR_ERR_BADARG** 
       - 
         The message buffer pointer did not refer to a valid message. 
      
     * - **RMR_ERR_NOHDR** 
       - 
         The header in the message buffer was not valid or corrupted. 
      
     * - **RMR_ERR_NOENDPT** 
       - 
         The message type in the message buffer did not map to a known 
         endpoint. 
          
 
 
The following values may be assigned to ``errno`` on failure. 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **INVAL** 
       - 
         Parameter(s) passed to the function were not valid, or the 
         underlying message processing environment was unable to 
         interpret the message. 
      
     * - **ENOKEY** 
       - 
         The header information in the message buffer was invalid. 
      
     * - **ENXIO** 
       - 
         No known endpoint for the message could be found. 
      
     * - **EMSGSIZE** 
       - 
         The underlying transport refused to accept the message 
         because of a size value issue (message was not attempted to 
         be sent). 
      
     * - **EFAULT** 
       - 
         The message referenced by the message buffer is corrupt (nil 
         pointer or bad internal length). 
      
     * - **EBADF** 
       - 
         Internal RMR error; information provided to the message 
         transport environment was not valid. 
      
     * - **ENOTSUP** 
       - 
         Sending was not supported by the underlying message 
         transport. 
      
     * - **EFSM** 
       - 
         The device is not in a state that can accept the message. 
      
     * - **EAGAIN** 
       - 
         The device is not able to accept a message for sending. The 
         user application should attempt to resend. 
      
     * - **EINTR** 
       - 
         The operation was interrupted by delivery of a signal before 
         the message was sent. 
      
     * - **ETIMEDOUT** 
       - 
         The underlying message environment timed out during the send 
         process. 
      
     * - **ETERM** 
       - 
         The underlying message environment is in a shutdown state. 
          
 


EXAMPLE
-------

The following is a simple example of how the 
``rmr_send_msg`` function is called. In this example, the 
send message buffer is saved between calls and reused 
eliminating alloc/free cycles. 
 
 
:: 
 
     static rmr_mbuf_t*  send_msg = NULL;        // message to send; reused on each call
     msg_t*  send_pm;                            // payload for send
     msg_t*  pm;                                 // our message format in the received payload
  
     if( send_msg  == NULL ) {
         send_msg = rmr_alloc_msg( mr, MAX_SIZE ); // new buffer to send
     }
  
     // reference payload and fill in message type
     pm = (msg_t*) send_msg->payload;
     send_msg->mtype = MT_ANSWER;
  
     msg->len = generate_data( pm );       // something that fills the payload in
     msg = rmr_send_msg( mr, send_msg );   // ensure new pointer used after send
     if( ! msg ) {
         return ERROR;
     } else {
         if( msg->state != RMR_OK ) {
             // check for RMR_ERR_RETRY, and resend if needed
             // else return error
         }
     }
     return OK;
  
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_mk_ring(3), 
rmr_ring_free(3), rmr_torcv_rcv(3), rmr_wh_send_msg(3) 

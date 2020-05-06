.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_wh_send_msg 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_wh_send_msg 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_wh_send_msg( void* vctx, rmr_whid_t id, rmr_mbuf_t* msg );
 


DESCRIPTION
-----------

The ``rmr_wh_send_msg`` function accepts a message buffer 
from the user application and attempts to send it using the 
wormhole ID provided (id). Unlike *rmr_send_msg,* this 
function attempts to send the message directly to a process 
at the other end of a wormhole which was created with 
*rmr_wh_open().* When sending message via wormholes, the 
normal RMR routing based on message type is ignored, and the 
caller may leave the message type unspecified in the message 
buffer (unless it is needed by the receiving process). 
 
The message buffer (msg) used to send is the same format as 
used for regular RMR send and reply to sender operations, 
thus any buffer allocated by these means, or calls to 
*rmr_rcv_msg()* can be passed to this function. 


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
state and should be ``RMR_OK.`` 
 
If the state in the returned buffer is anything other than 
``RMR_OK,`` the user application may need to attempt a 
retransmission of the message, or take other action depending 
on the setting of ``errno`` as described below. 
 
In the event of extreme failure, a nil pointer is returned. 
In this case the value of ``errno`` might be of some use, for 
documentation, but there will be little that the user 
application can do other than to move on. 


ERRORS
------

The following values may be passed back in the *state* field 
of the returned message buffer. 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **RMR_ERR_WHID** 
       - 
         The wormhole ID passed in was not associated with an open 
         wormhole, or was out of range for a valid ID. 
          
         | 
      
     * - **RMR_ERR_NOWHOPEN** 
       - 
         No wormholes exist, further attempt to validate the ID are 
         skipped. 
          
         | 
      
     * - **RMR_ERR_BADARG** 
       - 
         The message buffer pointer did not refer to a valid message. 
          
         | 
      
     * - **RMR_ERR_NOHDR** 
       - 
         The header in the message buffer was not valid or corrupted. 
          
 
 
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
          
         | 
      
     * - **ENOKEY** 
       - 
         The header information in the message buffer was invalid. 
          
         | 
      
     * - **ENXIO** 
       - 
         No known endpoint for the message could be found. 
          
         | 
      
     * - **EMSGSIZE** 
       - 
         The underlying transport refused to accept the message 
         because of a size value issue (message was not attempted to 
         be sent). 
          
         | 
      
     * - **EFAULT** 
       - 
         The message referenced by the message buffer is corrupt (nil 
         pointer or bad internal length). 
          
         | 
      
     * - **EBADF** 
       - 
         Internal RMR error; information provided to the message 
         transport environment was not valid. 
          
         | 
      
     * - **ENOTSUP** 
       - 
         Sending was not supported by the underlying message 
         transport. 
          
         | 
      
     * - **EFSM** 
       - 
         The device is not in a state that can accept the message. 
          
         | 
      
     * - **EAGAIN** 
       - 
         The device is not able to accept a message for sending. The 
         user application should attempt to resend. 
          
         | 
      
     * - **EINTR** 
       - 
         The operation was interrupted by delivery of a signal before 
         the message was sent. 
          
         | 
      
     * - **ETIMEDOUT** 
       - 
         The underlying message environment timed out during the send 
         process. 
          
         | 
      
     * - **ETERM** 
       - 
         The underlying message environment is in a shutdown state. 
          
 


EXAMPLE
-------

The following is a simple example of how the a wormhole is 
created (rmr_wh_open) and then how ``rmr_wh_send_msg`` 
function is used to send messages. Some error checking is 
omitted for clarity. 
 
 
:: 
 
  
 #include <rmr/rmr.h>    // system headers omitted for clarity
  
 int main() {
    rmr_whid_t whid = -1;   // wormhole id for sending
    void* mrc;      //msg router context
         int i;
    rmr_mbuf_t*  sbuf;      // send buffer
    int     count = 0;
    int     norm_msg_size = 1500;  // most msg fit in this size
  
    mrc = rmr_init( "43086", norm_msg_size, RMRFL_NONE );
    if( mrc == NULL ) {
       fprintf( stderr, "[FAIL] unable to initialise RMR environment\\n" );
       exit( 1 );
    }
  
    while( ! rmr_ready( mrc ) ) {        // wait for routing table info
       sleep( 1 );
    }
  
    sbuf = rmr_alloc_msg( mrc, 2048 );
  
    while( 1 ) {
      if( whid < 0 ) {
        whid = rmr_wh_open( mrc, "localhost:6123" );  // open fails if endpoint refuses conn
           if( RMR_WH_CONNECTED( wh ) ) {
            snprintf( sbuf->payload, 1024, "periodic update from sender: %d", count++ );
            sbuf->len =  strlen( sbuf->payload );
            sbuf = rmr_wh_send_msg( mrc, whid, sbuf );
         }
      }
  
      sleep( 5 );
    }
 }
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_set_stimeout(3), rmr_wh_open(3), rmr_wh_close(3), 
rmr_wh_state(3) 

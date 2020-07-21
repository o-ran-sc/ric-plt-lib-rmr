.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_wh_call
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_wh_call


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  rmr_mbuf_t* rmr_wh_call( void* vctx, rmr_whid_t whid, rmr_mbuf_t* msg, int call_id, int max_wait )




DESCRIPTION
-----------

The ``rmr_wh_call`` function accepts a message buffer (msg)
from the user application and attempts to send it using the
wormhole ID provided (whid). If the send is successful, the
call will block until either a response message is received,
or the ``max_wait`` number of milliseconds has passed. In
order for the response to be recognised as a response, the
remote process **must** use ``rmr_rts_msg()`` to send their
response.

Like *rmr_wh_send_msg,* this function attempts to send the
message directly to a process at the other end of a wormhole
which was created with *rmr_wh_open().* When sending message
via wormholes, the normal RMR routing based on message type
is ignored, and the caller may leave the message type
unspecified in the message buffer (unless it is needed by the
receiving process). The ``call_id`` parameter is a number in
the range of 2 through 255 and is used to identify the
calling thread in order to properly match a response message
when it arrives. Providing this value, and ensuring the
proper uniqueness, is the responsibility of the user
application and as such the ability to use the
``rmr_wh_call()`` function from potentially non-threaded
concurrent applications (such as Go's goroutines) is
possible.


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

On success, new message buffer, with the payload containing
the response from the remote endpoint is returned. The state
in this buffer will reflect the overall send operation state
and should be ``RMR_OK.``

If a message is returned with a state which is anything other
than ``RMR_OK,`` the indication is that the send was not
successful. The user application must check the state and
determine the course of action. If the return value is NULL,
no message, the indication is that there was no response
received within the timeout (max_wait) period of time.


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

      * - **RMR_ERR_NOWHOPEN**
        -
          No wormholes exist, further attempt to validate the ID are
          skipped.

      * - **RMR_ERR_BADARG**
        -
          The message buffer pointer did not refer to a valid message.

      * - **RMR_ERR_NOHDR**
        -
          The header in the message buffer was not valid or corrupted.




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
     int     norm_msg_size = 1500;    // most messages fit in this size

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
             sbuf = rmr_wh_call( mrc, whid, sbuf, 1000 );        // expect a response in 1s or less
             if( sbuf != NULL && sbuf->state = RMR_OK ) {
               sprintf( stderr, "response: %s\\n", sbuf->payload );    // assume they sent a string
             } else {
               sprintf( stderr, "response not received, or send error\\n" );
             }
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

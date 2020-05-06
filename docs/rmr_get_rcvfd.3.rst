.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_get_rcvfd 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_rcvfd 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 void* rmr_get_rcvfd( void* ctx )
 


DESCRIPTION
-----------

The ``rmr_get_rcvfd`` function returns a file descriptor 
which may be given to epoll_wait() by an application that 
wishes to use event poll in a single thread rather than block 
on the arrival of a message via calls to rmr_rcv_msg(). When 
epoll_wait() indicates that this file descriptor is ready, a 
call to rmr_rcv_msg() will not block as at least one message 
has been received. 
 
The context (ctx) pointer passed in is the pointer returned 
by the call to rmr_init(). 


RETURN VALUE
------------

The ``rmr_get_rcvfd`` function returns a file descriptor 
greater or equal to 0 on success and -1 on error. 


ERRORS
------

The following error values are specifically set by this RMR 
function. In some cases the error message of a system call is 
propagated up, and thus this list might be incomplete. 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **EINVAL** 
       - 
         The use of this function is invalid in this environment. 
          
 


EXAMPLE
-------

The following short code bit illustrates the use of this 
function. Error checking has been omitted for clarity. 
 
 
:: 
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/epoll.h>
 #include <rmr/rmr.h>
  
 int main() {
     int rcv_fd;     // pollable fd
     void* mrc;      //msg router context
     struct epoll_event events[10];          // support 10 events to poll
     struct epoll_event epe;                 // event definition for event to listen to
     int     ep_fd = -1;
     rmr_mbuf_t* msg = NULL;
     int nready;
     int i;
     int norm_msg_size = 1500;               // 95% messages are less than this
  
     mrc = rmr_init( "43086", norm_msg_size, RMRFL_NONE );
     rcv_fd = rmr_get_rcvfd( mrc );
  
     ep_fd = epoll_create1( 0 );             // initialise epoll environment
     epe.events = EPOLLIN;
     epe.data.fd = rcv_fd;
     epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe );    // add our info to the mix
  
     while( 1 ) {
         nready = epoll_wait( ep_fd, events, 10, -1 );   // -1 == block forever (no timeout)
         for( i = 0; i < nready && i < 10; i++ ) {       // loop through to find what is ready
             if( events[i].data.fd == rcv_fd ) {         // RMR has something
                 msg = rmr_rcv_msg( mrc, msg );
                 if( msg ) {
                     // do something with msg
                 }
             }
  
             // check for other ready fds....
         }
     }
 }
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 

.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_wh_open 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_wh_open 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 void* rmr_wh_open( void* vctx, char* target )
 


DESCRIPTION
-----------

The ``rmr_wh_open`` function creates a direct link for 
sending, a wormhole, to another RMR based process. Sending 
messages through a wormhole requires that the connection be 
established overtly by the user application (via this 
function), and that the ID returned by ``rmr_wh_open`` be 
passed to the ``rmr_wh_send_msg`` function. 
 
*Target* is the *name* or *IP-address* combination of the 
processes that the wormhole should be connected to. *Vctx* is 
the RMR void context pointer that was returned by the 
``rmr_init`` function. 
 
When invoked, this function immediately attempts to connect 
to the target process. If the connection cannot be 
established, an error is returned to the caller, and no 
direct messages can be sent to the target. Once a wormhole is 
connected, the underlying transport mechanism (e.g. NNG) will 
provide reconnects should the connection be lost, however the 
handling of messages sent when a connection is broken is 
undetermined as each underlying transport mechanism may 
handle buffering and retries differently. 


RETURN VALUE
------------

The ``rmr_wh_open`` function returns a type 
``rmr_whid_t`` which must be passed to the 
``rmr_wh_send_msg`` function when sending a message. The id 
may also be tested to determine success or failure of the 
connection by using the RMR_WH_CONNECTED macro and passing 
the ID as the parameter; a result of 1 indicates that the 
connection was established and that the ID is valid. 


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
         A parameter passed was not valid. 
          
         | 
      
     * - **EACCESS** 
       - 
         The user application does not have the ability to establish a 
         wormhole to the indicated target (or maybe any target). 
          
         | 
      
     * - **ECONNREFUSED** 
       - 
         The connection was refused. 
          
 


EXAMPLE
-------

 
:: 
 
    void*  rmc;
    rmr_whid_t wh;
  
    rmc = rmr_init( "43086", 4096, 0 ); // init context
    wh = rmr_wh_open( rmc, "localhost:6123" );
    if( !RMR_WH_CONNECTED( wh ) ) {
      fprintf( stderr, "unable to connect wormhole: %s\\n",
              strerror( errno ) );
    }
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_close(3), 
rmr_wh_send_msg(3), rmr_wh_state(3) 

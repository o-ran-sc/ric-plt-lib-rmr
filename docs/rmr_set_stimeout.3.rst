.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_set_stimeout
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_set_stimeout


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  int rmr_set_stimeout( void* vctx, int rloops );




DESCRIPTION
-----------

The ``rmr_set_stimeout`` function sets the configuration for
how RMR will retry message send operations which complete
with either a *timeout* or *again* completion value. (Send
operations include all of the possible message send
functions: *rmr_send_msg(), rmr_call(), rmr_rts_msg()* and
*rmr_wh_send_msg().* The *rloops* parameter sets the maximum
number of retry loops that will be attempted before giving up
and returning the unsuccessful state to the user application.
Each retry loop is approximately 1000 attempts, and RMR does
**not** invoke any sleep function between retries in the
loop; a small, 1 mu-sec, sleep is executed between loop sets
if the *rloops* value is greater than 1.



Disabling Retries
-----------------

By default, the send operations will execute with an *rloop*
setting of 1; each send operation will attempt to resend the
message approximately 1000 times before giving up. If the
user application does not want to have send operations retry
when the underlying transport mechanism indicates *timeout*
or *again,* the application should invoke this function and
pass a value of 0 (zero) for *rloops.* With this setting, all
RMR send operations will attempt a send operation only
**once,** returning immediately to the caller with the state
of that single attempt.


RETURN VALUE
------------

This function returns a -1 to indicate that the *rloops*
value could not be set, and the value *RMR_OK* to indicate
success.


ERRORS
------

Currently errno is **not** set by this function; the only
cause of a failure is an invalid context (*vctx*) pointer.


EXAMPLE
-------

The following is a simple example of how the
``rmr_set_stimeout`` function is called.


::

      #define NO_FLAGS    0

      char* port = "43086";     // port for message router listen
      int   max_size = 4096;    // max message size for default allocations
      void* mr_context;         // message router context

      mr_context = rmr_init( port, max_size, NO_FLAGS );
      if( mr_context != NULL ) {
          rmr_set_stimeout( mr_context, 0 );    // turn off retries
      }




SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3),
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3),
rmr_rts_msg(3), rmr_ready(3), rmr_mk_ring(3),
rmr_ring_free(3), rmr_send_msg(3), rmr_torcv_rcv(3),
rmr_wh_send_msg(3)

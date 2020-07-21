.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_wh_close
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_wh_close


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  void rmr_wh_close( void* vctx, rmr_whid_t whid )



DESCRIPTION
-----------

The ``rmr_wh_close`` function closes the wormhole associated
with the wormhole id passed in. Future calls to
``rmr_wh_send_msg`` with this ID will fail.

The underlying TCP connection to the remote endpoint is
**not** closed as this session may be required for regularly
routed messages (messages routed based on message type).
There is no way to force a TCP session to be closed at this
point in time.


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3),
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3),
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3),
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3),
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_open(3),
rmr_wh_send_msg(3)

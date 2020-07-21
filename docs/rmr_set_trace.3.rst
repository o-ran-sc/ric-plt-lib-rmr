.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_set_trace
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_set_trace


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  int rmr_set_trace( rmr_mbuf_t* mbuf, unsigned char* data, int len )



DESCRIPTION
-----------

The ``rmr_set_trace`` function will copy ``len`` bytes from
``data`` into the trace portion of ``mbuf.`` If the trace
area of ``mbuf`` is not the correct size, the message buffer
will be reallocated to ensure that enough space is available
for the trace data.


RETURN VALUE
------------

The ``rmr_set_trace`` function returns the number of bytes
successfully copied to the message. If 0 is returned either
the message pointer was nil, or the size in the parameters
was <= 0.


SEE ALSO
--------

rmr_alloc_msg(3), rmr_tralloc_msg(3), rmr_bytes2xact(3),
rmr_bytes2payload(3), rmr_call(3), rmr_free_msg(3),
rmr_get_rcvfd(3), rmr_get_meid(3), rmr_get_trace(3),
rmr_get_trlen(3), rmr_init(3), rmr_init_trace(3),
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3),
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3),
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3),
rmr_ring_free(3), rmr_str2meid(3), rmr_str2xact(3),
rmr_wh_open(3), rmr_wh_send_msg(3)

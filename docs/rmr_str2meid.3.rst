.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_str2meid
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_str2meid


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  int rmr_str2meid( rmr_mbuf_t* mbuf, unsigned char* src, int len )



DESCRIPTION
-----------

The ``rmr_str2meid`` function will copy the string pointed to
by src to the managed entity ID (meid) field in the given
message. The field is a fixed length, gated by the constant
``RMR_MAX_MEID`` and if string length is larger than this
value, then **nothing** will be copied. (Note, this differs
slightly from the behaviour of the ``lrmr_bytes2meid()``
function.)


RETURN VALUE
------------

On success, the value RMR_OK is returned. If the string
cannot be copied to the message, the return value will be one
of the errors listed below.


ERRORS
------

If the return value is not RMR_OK, then it will be set to one
of the values below.

    .. list-table::
      :widths: auto
      :header-rows: 0
      :class: borderless

      * - **RMR_ERR_BADARG**
        -
          The message, or an internal portion of the message, was
          corrupted or the pointer was invalid.

      * - **RMR_ERR_OVERFLOW**
        -
          The length passed in was larger than the maximum length of
          the field; only a portion of the source bytes were copied.




EXAMPLE
-------



SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3),
rmr_get_meid(3), rmr_get_rcvfd(3), rmr_payload_size(3),
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3),
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3),
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3),
rmr_bytes2meid(3), rmr_wh_open(3), rmr_wh_send_msg(3)

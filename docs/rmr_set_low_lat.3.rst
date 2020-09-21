.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_set_low_lat
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_set_low_latency


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  void rmr_set_low_latency( void* vctx );




DESCRIPTION
-----------

The ``rmr_set_low_latency`` function enables *TCP NO_DELAY*
if the underlying transport library supports it. This might
be useful for applications which must send messages at a
maximum rate.

A call to this function will cause all subsequent connections
made by the application to set the no delay (low latency)
option. When no delay is needed, it is recommended that this
function be called immediately upon a successful call to the
RMR initialisation function.

The effect of setting "low latency" mode is to disable
Nagel's algorithm and to send a packet on a connection as
soon as it is given to the TCP transport. When this option is
not enabled, TCP may employ Nagel's algorithm and hold the
packet with the assumption that it can be combined with
another and sent in the same datagram in an effort to improve
bandwidth but at the expense of added latency.


RETURN VALUE
------------

There is no return value.


ERRORS
------

This function does not generate any errors.


SEE ALSO
--------

rmr_init(3), rmr_fast_ack(3)

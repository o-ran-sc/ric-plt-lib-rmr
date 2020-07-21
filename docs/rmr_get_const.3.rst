.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
Man Page: rmr_get_const
============================================================================================




RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_const


SYNOPSIS
--------


::

  #include <rmr/rmr.h>

  unsigned char* rmr_get_const();



DESCRIPTION
-----------

The ``rmr_get_const`` function is a convenience function for
wrappers which do not have the ability to "compile in" RMR
constants. The function will build a nil terminated string
containing JSON which defines the RMR constants that C and Go
applications have at compile time via the ``rmr.h`` header
file.

All values are represented as strings and the JSON format is
illustrated in the following (partial) example:


::

  {
    "RMR_MAX_XID": "32",
    "RMR_OK": "0",
    "RMR_ERR_BADARG", "1",
    "RMR_ERR_NOENDPT" "2"
  }



RETURN VALUE
------------

On success, a pointer to a string containing the JSON
defining constant and value pairs. On failure a nil pointer
is returned.


SEE ALSO
--------

rmr(7)

.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_set_fack 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

rmr_set_fack 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 void rmr_set_fack( void* vctx );
 


1.3. DESCRIPTION
----------------

The ``rmr_set_fack`` function enables *fast TCP 
acknowledgements* if the underlying transport library 
supports it. This might be useful for applications which must 
send messages at a maximum rate. 


1.4. RETURN VALUE
-----------------

There is no return value. 


1.5. ERRORS
-----------

This function does not generate any errors. 


1.6. SEE ALSO
-------------

rmr_init(3), 

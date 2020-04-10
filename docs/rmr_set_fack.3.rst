 
 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
============================================================================================ 
Man Page: rmr_set_fack 
============================================================================================ 
 
RMR Library Functions 
============================================================================================ 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_set_fack 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void rmr_set_fack( void* vctx );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_set_fack function enables *fast TCP acknowledgements* 
if the underlying transport library supports it. This might 
be useful for applications which must send messages at a 
maximum rate. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
There is no return value. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
This function does not generate any errors. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_init(3), 

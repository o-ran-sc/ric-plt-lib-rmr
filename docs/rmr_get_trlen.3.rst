.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_get_trlen 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_get_trlen 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 int rmr_get_trlen( rmr_mbuf_t* msg );
 


DESCRIPTION
-----------

Given a message buffer, this function returns the amount of 
space (bytes) that have been allocated for trace data. If no 
trace data has been allocated, then 0 is returned. 


RETURN VALUE
------------

The number of bytes allocated for trace information in the 
given message. 


ERRORS
------

 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **INVAL** 
       - 
         Parameter(s) passed to the function were not valid. 
          
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_trace(3), rmr_init(3), rmr_init_trace(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_set_trace(3), rmr_tralloc_msg(3) 

.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_wh_state 
============================================================================================ 
 
 


1. RMR LIBRARY FUNCTIONS
========================



1.1. NAME
---------

rmr_wh_state 


1.2. SYNOPSIS
-------------

 
:: 
 
 #include <rmr/rmr.h>
 int rmr_wh_state( void* vctx, rmr_whid_t whid )
 


1.3. DESCRIPTION
----------------

The ``rmr_wh_state`` function will return the current state 
of the connection associated with the given wormhole (whid). 
The return value indicates whether the connection is open 
(RMR_OK), or closed (any other return value). 
 
When using some transport mechanisms (e.g. NNG), it may not 
be possible for RMR to know the actual state and the 
connection may always be reported as "open." 


1.4. RETURN
-----------

The following values are potential return values. 
 
 
RMR_OK 
  The wormhole ID is valid and the connection is "open." 
   
RMR_ERR_WHID 
  THe wormhole ID passed into the function was not valid. 
   
RMR_ERR_NOENDPT 
  The wormhole is not open (not connected). 
   
RMR_ERR_BADARG 
  The context passed to the function was nil or invalid. 
   
RMR_ERR_NOWHOPEN 
  Wormholes have not been initialised (no wormhole open call 
  has been made). 
   


1.5. SEE ALSO
-------------

rmr_wh_open(3), rmr_wh_send_msg(3), rmr_wh_close(3) 

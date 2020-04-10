 
 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
============================================================================================ 
Man Page: rmr_bytes2xact 
============================================================================================ 
 
RMR Library Functions 
============================================================================================ 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_bytes2xact 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_bytes2xact( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_bytes2xact function will copy up to *len* butes from 
*src* to the transaction ID (xaction) field in the message. 
The field is a fixed length, gated by the constant 
RMR_MAX_XID and if len is larger than this value, only 
RMR_MAX_XID bytes will actually be copied. 
 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, the actual number of bytes copied is returned, 
or -1 to indicate a hard error. If the length is less than 
0, or not the same as length passed in, errno is set to 
one of the errors described in the *Errors* section. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If the returned length does not match the length passed 
in, errno will be set to one of the following constants 
with the meaning listed below. 
 
 
EINVAL 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
   
 
EOVERFLOW 
   
  The length passed in was larger than the maximum length of 
  the field; only a portion of the source bytes were copied. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2meid(3), rmr_call(3), 
rmr_free_msg(3), rmr_get_meid(3), rmr_get_rcvfd(3), 
rmr_get_xact(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_wh_open(3), rmr_wh_send_msg(3) 

.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
============================================================================================ 
Man Page: rmr_tralloc_msg 
============================================================================================ 
 
 


RMR LIBRARY FUNCTIONS
=====================



NAME
----

rmr_tralloc_msg 


SYNOPSIS
--------

 
:: 
 
 #include <rmr/rmr.h>
  
 rmr_mbuf_t* rmr_tralloc_msg( void* vctx, int size,
                              int trace_size, unsigned const char *tr_data );
 


DESCRIPTION
-----------

The ``rmr_tralloc_msg`` function is used to allocate a buffer 
which the user programme can write into and then send through 
the library. The buffer is allocated such that sending it 
requires no additional copying from the buffer as it passes 
through the underlying transport mechanism. 
 
The *size* parameter is used to set the payload length in the 
message. If it is 0, then the default size supplied on the 
*rmr_init* call will be used. In addition to allocating the 
payload, a space in the buffer is reserved for *trace* data 
(tr_size bytes), and the bytes pointed to by *tr_data* are 
copied into that portion of the message. The *vctx* parameter 
is the void context pointer that was returned by the 
*rmr_init* function. 
 
The pointer to the message buffer returned is a structure 
which has some user application visible fields; the structure 
is described in ``rmr.h,`` and is illustrated below. 
 
 
:: 
 
 typedef struct {
     int state;
     int mtype;
     int len;
     unsigned char* payload;
     unsigned char* xaction;
 } rmr_mbuf_t;
 
 
Where: 
 
 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **state** 
       - 
         Is the current buffer state. Following a call to 
         ``rmr_send_msg`` the state indicates whether the buffer was 
         successfully sent which determines exactly what the payload 
         points to. If the send failed, the payload referenced by the 
         buffer is the message that failed to send (allowing the 
         application to attempt a retransmission). When the state is 
         ``a_OK`` the buffer represents an empty buffer that the 
         application may fill in in preparation to send. 
          
         | 
      
     * - **mtype** 
       - 
         When sending a message, the application is expected to set 
         this field to the appropriate message type value (as 
         determined by the user programme). Upon send this value 
         determines how the a library will route the message. For a 
         buffer which has been received, this field will contain the 
         message type that was set by the sending application. 
          
         | 
      
     * - **len** 
       - 
         The application using a buffer to send a message is expected 
         to set the length value to the actual number of bytes that it 
         placed into the message. This is likely less than the total 
         number of bytes that the message can carry. For a message 
         buffer that is passed to the application as the result of a 
         receive call, this will be the value that the sending 
         application supplied and should indicate the number of bytes 
         in the payload which are valid. 
          
         | 
      
     * - **payload** 
       - 
         The payload is a pointer to the actual received data. The 
         user programme may read and write from/to the memory 
         referenced by the payload up until the point in time that the 
         buffer is used on a ``rmr_send, rmr_call`` or 
         ``rmr_reply`` function call. Once the buffer has been passed 
         back to a a library function the user programme should 
         **NOT** make use of the payload pointer. 
          
         | 
      
     * - **xaction** 
       - 
         The *xaction* field is a pointer to a fixed sized area in the 
         message into which the user may write a transaction ID. The 
         ID is optional with the exception of when the user 
         application uses the ``rmr_call`` function to send a message 
         and wait for the reply; the underlying processing expects 
         that the matching reply message will also contain the same 
         data in the *xaction* field. 
          
 


RETURN VALUE
------------

The function returns a pointer to a ``rmr_mbuf`` structure, 
or NULL on error. 


ERRORS
------

 
   .. list-table:: 
     :widths: auto 
     :header-rows: 0 
     :class: borderless 
      
     * - **ENOMEM** 
       - 
         Unable to allocate memory. 
          
 


SEE ALSO
--------

rmr_alloc_msg(3), rmr_mbuf(3) rmr_call(3), rmr_free_msg(3), 
rmr_init(3), rmr_init_trace(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_set_trace(3) 

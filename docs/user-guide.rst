 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
RMR User's Guide 
============================================================================================ 
 
The RIC Message Router (RMR) is a library which applications 
use to send and receive messages where the message routing, 
endpoint selection, is based on the message type rather than 
on traditional DNS names or IP addresses. Because the user 
documentation for RMR is a collection of UNIX manpages 
(included in the development package, and available via the 
man command when installed), there is no separate "User's 
Guide." To provide something for the document scrapers to 
find, this is a collection of the RMR manual pages formatted 
directly from their source which might be a bit ragged when 
combined into a single markup document. Read the manual pages 
:) 
 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_alloc_msg( void* ctx, int size );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_alloc_msg function is used to allocate a buffer which 
the user programme can write into and then send through the 
RMR library. The buffer is allocated such that sending it 
requires no additional copying out of the buffer. If the 
value passed in size is 0, then the default size supplied on 
the *rmr_init* call will be used. The *ctx* parameter is the 
void context pointer that was returned by the *rmr_init* 
function. 
 
The pointer to the message buffer returned is a structure 
which has some user application visible fields; the structure 
is described in rmr.h, and is illustrated below. 
 
 
:: 
  
 typedef struct {
     int state;
     int mtype;
     int len;
     unsigned char* payload;
     unsigned char* xaction;
     uint sub_id;
     uint tp_state;
 } rmr_mbuf_t;
 
 
 
 
 
state 
   
  Is the current buffer state. Following a call to 
  rmr_send_msg the state indicates whether the buffer was 
  successfully sent which determines exactly what the 
  payload points to. If the send failed, the payload 
  referenced by the buffer is the message that failed to 
  send (allowing the application to attempt a 
  retransmission). When the state is RMR_OK the buffer 
  represents an empty buffer that the application may fill 
  in in preparation to send. 
   
 
mtype 
   
  When sending a message, the application is expected to set 
  this field to the appropriate message type value (as 
  determined by the user programme). Upon send this value 
  determines how the RMR library will route the message. For 
  a buffer which has been received, this field will contain 
  the message type that was set by the sending application. 
   
 
len 
   
  The application using a buffer to send a message is 
  expected to set the length value to the actual number of 
  bytes that it placed into the message. This is likely less 
  than the total number of bytes that the message can carry. 
  For a message buffer that is passed to the application as 
  the result of a receive call, this will be the value that 
  the sending application supplied and should indicate the 
  number of bytes in the payload which are valid. 
   
 
payload 
   
  The payload is a pointer to the actual received data. The 
  user programme may read and write from/to the memory 
  referenced by the payload up until the point in time that 
  the buffer is used on a rmr_send, rmr_call or rmr_reply 
  function call. Once the buffer has been passed back to a 
  RMR library function the user programme should **NOT** 
  make use of the payload pointer. 
   
 
xaction 
   
  The *xaction* field is a pointer to a fixed sized area in 
  the message into which the user may write a transaction 
  ID. The ID is optional with the exception of when the user 
  application uses the rmr_call function to send a message 
  and wait for the reply; the underlying RMR processing 
  expects that the matching reply message will also contain 
  the same data in the *xaction* field. 
   
 
sub_id 
   
  This value is the subscription ID. It, in combination with 
  the message type is used by rmr to determine the target 
  endpoint when sending a message. If the application to 
  application protocol does not warrant the use of a 
  subscription ID, the RMR constant RMR_VOID_SUBID should be 
  placed in this field. When an application is forwarding or 
  returning a buffer to the sender, it is the application's 
  responsibility to set/reset this value. 
   
 
tp_state 
   
  For C applications making use of RMR, the state of a 
  transport based failure will often be available via errno. 
  However, some wrapper environments may not have direct 
  access to the C-lib errno value. RMR send and receive 
  operations will place the current value of errno into this 
  field which should make it available to wrapper functions. 
  User applications are strongly cautioned against relying 
  on the value of errno as some transport mechanisms may not 
  set this value on all calls. This value should also be 
  ignored any time the message status is RMR_OK. 
 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The function returns a pointer to a rmr_mbuf structure, or 
NULL on error. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
 
 
ENOMEM 
   
  Unable to allocate memory. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_tralloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_init(3), rmr_init_trace(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_set_trace(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_bytes2meid 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_bytes2meid( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_bytes2meid function will copy up to *len* butes from 
*src* to the managed entity ID (meid) field in the message. 
The field is a fixed length, gated by the constant 
RMR_MAX_MEID and if len is larger than this value, only 
RMR_MAX_MEID bytes will actually be copied. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, the actual number of bytes copied is returned, or 
-1 to indicate a hard error. If the length is less than 0, or 
not the same as length passed in, errno is set to one of the 
errors described in the *Errors* section. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If the returned length does not match the length passed in, 
errno will be set to one of the following constants with the 
meaning listed below. 
 
 
 
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
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_call(3), 
rmr_free_msg(3), rmr_get_rcvfd(3), rmr_get_meid(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3), rmr_str2meid(3), rmr_str2xact(3), 
rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_bytes2payload 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void rmr_bytes2payload( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
This is a convenience function as some wrapper languages 
might not have the ability to directly copy into the payload 
buffer. The bytes from *src* for the length given are copied 
to the payload. It is the caller's responsibility to ensure 
that the payload is large enough. Upon successfully copy, the 
len field in the message buffer is updated to reflect the 
number of bytes copied. 
 
There is little error checking, and no error reporting. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
None. 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2payload(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_meid(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
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
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_call 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 extern rmr_mbuf_t* rmr_call( void* vctx, rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_call function sends the user application message to a 
remote endpoint, and waits for a corresponding response 
message before returning control to the user application. The 
user application supplies a completed message buffer, as it 
would for a rmr_send call, but unlike with the send, the 
buffer returned will have the response from the application 
that received the message. 
 
Messages which are received while waiting for the response 
are queued internally by RMR, and are returned to the user 
application when rmr_rcv_msg is invoked. These messages are 
returned in th order received, one per call to rmr_rcv_msg. 
 
Call Timeout 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The rmr_call function implements a timeout failsafe to 
prevent, in most cases, the function from blocking forever. 
The timeout period is **not** based on time (calls to clock 
are deemed too expensive for a low latency system level 
library, but instead the period is based on the number of 
received messages which are not the response. Using a 
non-time mechanism for *timeout* prevents the async queue 
from filling (which would lead to message drops) in an 
environment where there is heavy message traffic. 
 
When the threshold number of messages have been queued 
without receiving a response message, control is returned to 
the user application and a NULL pointer is returned to 
indicate that no message was received to process. Currently 
the threshold is fixed at 20 messages, though in future 
versions of the library this might be extended to be a 
parameter which the user application may set. 
 
Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The send operations in RMr will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 
1. 
   
  The message is sent without error 
   
 
2. 
   
  The underlying transport reports a * hard * failure 
   
 
3. 
   
  The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attemps ** 
without** any intervening calls to * sleep() * or * usleep(). 
* The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMr 
initialisation using the * rmr_set_stimeout() * function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 
 
Transport Level Blocking 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremly possible that during 
normal operations that retry conditions are very likely to 
happen. These are completely out of RMr's control, and there 
is nothing that RMr can do to avoid or midigate these other 
than by allowing RMr to retry the send operation, and even 
then it is possible (e.g. during connection reattempts), that 
a single retry loop is not enough to guarentee a successful 
send. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_call function returns a pointer to a message buffer 
with the state set to reflect the overall state of call 
processing (see Errors below). In some cases a NULL pointer 
will be returned; when this is the case only *errno* will be 
available to describe the reason for failure. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
These values are reflected in the state field of the returned 
message. 
 
 
 
RMR_OK 
   
  The call was successful and the message buffer references 
  the response message. 
   
 
RMR_ERR_CALLFAILED 
   
  The call failed and the value of *errno,* as described 
  below, should be checked for the specific reason. 
 
 
The global "variable" *errno* will be set to one of the 
following values if the overall call processing was not 
successful. 
 
 
 
ETIMEDOUT 
   
  Too many messages were queued before receiving the 
  expected response 
   
 
ENOBUFS 
   
  The queued message ring is full, messages were dropped 
   
 
EINVAL 
   
  A parameter was not valid 
   
 
EAGAIN 
   
  The underlying message system wsa interrupted or the 
  device was busy; the message was **not** sent, and user 
  application should call this function with the message 
  again. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following code bit shows one way of using the rmr_call 
function, and illustrates how the transaction ID must be set. 
 
 
:: 
  
     int retries_left = 5;               // max retries on dev not available
     int retry_delay = 50000;            // retry delay (usec)
     static rmr_mbuf_t*  mbuf = NULL;    // response msg
     msg_t*  pm;                         // private message (payload)
     m// get a send buffer and reference the payload 
     mbuf = rmr_alloc_msg( mr, RMR_MAX_RCV_BYTES );
     pm = (msg_t*) mbuf->payload;
     p// generate an xaction ID and fill in payload with data and msg type
     snprintf( mbuf->xaction, RMR_MAX_XID, "%s", gen_xaction() );
     snprintf( pm->req, sizeof( pm->req ), "{ \\"req\\": \\"num users\\"}" );
     mbuf->mtype = MT_REQ;
     
     msg = rmr_call( mr, msg );
     if( ! msg ) {               // probably a timeout and no msg received
         return NULL;            // let errno trickle up
     } 
     if( mbuf->state != RMR_OK ) {
         while( retries_left-- > 0 &&             // loop as long as eagain
                errno == EAGAIN && 
                (msg = rmr_call( mr, msg )) != NULL && 
                mbuf->state != RMR_OK ) {
             usleep( retry_delay );
         }
     
         if( mbuf == NULL || mbuf->state != RMR_OK ) {
             rmr_free_msg( mbuf );        // safe if nil
             return NULL;
         }
     }
     // do something with mbuf
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_set_stimeout(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_open 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void rmr_close( void* vctx )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_close function closes the listen socket effectively 
cutting the application off. The route table listener is also 
stopped. Calls to rmr_rcv_msg() will fail with unpredictable 
error codes, and calls to rmr_send_msg(), rmr_call(), and 
rmr_rts_msg() will have unknown results. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_open(3), 
rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_free_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void rmr_free_msg( rmr_mbuf_t* mbuf );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The message buffer is returned to the pool, or the associated 
memory is released depending on the needs of the underlying 
messaging system. This allows the user application to release 
a buffer that is not going to be used. It is safe to pass a 
nil pointer to this function, and doing so does not result in 
a change to the value of errrno. 
 
After calling, the user application should **not** use any of 
the pointers (transaction ID, or payload) which were 
available. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_meid 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 char* rmr_get_meid( rmr_mbuf_t* mbuf, unsigned char* dest )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_meid function will copy the managed entity ID 
(meid) field from the message into the *dest* buffer provided 
by the user. The buffer referenced by *dest* is assumed to be 
at least RMR_MAX_MEID bytes in length. If *dest* is NULL, 
then a buffer is allocated (the calling application is 
expected to free when the buffer is no longer needed). 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a pointer to the extracted string is returned. If 
*dest* was supplied, then this is just a pointer to the 
caller's buffer. If *dest* was NULL, this is a pointer to the 
allocated buffer. If an error occurs, a nil pointer is 
returned and errno is set as described below. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If an error occurs, the value of the global variable errno 
will be set to one of the following with the indicated 
meaning. 
 
 
 
EINVAL 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
   
 
ENOMEM 
   
  A nil pointer was passed for *dest,* however it was not 
  possible to allocate a buffer using malloc(). 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_xact(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_rcvfd 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void* rmr_get_rcvfd( void* ctx )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_rcvfd function returns a file descriptor which 
may be given to epoll_wait() by an application that wishes to 
use event poll in a single thread rather than block on the 
arrival of a message via calls to rmr_rcv_msg(). When 
epoll_wait() indicates that this file descriptor is ready, a 
call to rmr_rcv_msg() will not block as at least one message 
has been received. 
 
The context (ctx) pointer passed in is the pointer returned 
by the call to rmr_init(). 
 
**NOTE:** There is no support for epoll in Nanomsg, thus his 
function is only supported when linking with the NNG version 
of RMr and the file descriptor returned when using the 
Nanomsg verfsion will always return an error. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_rcvfd function returns a file descriptor greater 
or equal to 0 on success and -1 on error. If this function is 
called from a user application linked against the Nanomsg RMr 
library, calls will always return -1 with errno set to 
EINVAL. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following error values are specifically set by this RMR 
function. In some cases the error message of a system call is 
propagated up, and thus this list might be incomplete. 
 
 
EINVAL 
   
  The use of this function is invalid in this environment. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following short code bit illustrates the use of this 
function. Error checking has been omitted for clarity. 
 
 
:: 
  
 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/epoll.h>
 #include <rmr/rmr.h>
 int main() {
     int rcv_fd;     // pollable fd
     void* mrc;      //msg router context
     struct epoll_event events[10];          // support 10 events to poll
     struct epoll_event epe;                 // event definition for event to listen to
     int     ep_fd = -1;
     rmr_mbuf_t* msg = NULL;
     int nready;
     int i;
  
     mrc = rmr_init( "43086", RMR_MAX_RCV_BYTES, RMRFL_NONE );
     rcv_fd = rmr_get_rcvfd( mrc );
  
     rep_fd = epoll_create1( 0 );    _    B    ,// initialise epoll environment
     epe.events = EPOLLIN;
     epe.data.fd = rcv_fd;
     epoll_ctl( ep_fd, EPOLL_CTL_ADD, rcv_fd, &epe );    // add our info to the mix
  
     while( 1 ) {
         nready = epoll_wait( ep_fd, events, 10, -1 );       // -1 == block forever (no timeout)
         for( i = 0; i < nready && i < 10; i++ ) {           // loop through to find what is ready
             if( events[i].data.fd == rcv_fd ) {             // RMr has something
                 msg = rmr_rcv_msg( mrc, msg );
                 if( msg ) {
                     // do something with msg
                 }
             }
  
             // check for other ready fds....
         }
     }
 }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_src 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 unsigned char* rmr_get_src( rmr_mbuf_t* mbuf, unsigned char* dest )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_src function will copy the *source* information 
from the message to a buffer (dest) supplied by the user. In 
an RMr message, the source is the sender's information that 
is used for return to sender function calls, and is generally 
the hostname and port in the form *name*. The source might be 
an IP address port combination; the data is populated by the 
sending process and the only requirement is that it be 
capable of being used to start a TCP session with the sender. 
 
The maximum size allowed by RMr is 64 bytes (including the 
nil string terminator), so the user must ensure that the 
destination buffer given is at least 64 bytes. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a pointer to the destination buffer is given as a 
convenience to the user programme. On failure, a nil pointer 
is returned and the value of errno is set. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If an error occurs, the value of the global variable errno 
will be set to one of the following with the indicated 
meaning. 
 
 
 
EINVAL 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_srcip(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_srcip 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 unsigned char* rmr_get_srcip( rmr_mbuf_t* mbuf, unsigned char* dest )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_srcip function will copy the *source IP address* 
from the message to a buffer (dest) supplied by the user. In 
an RMr message, the source IP address is the sender's 
information that is used for return to sender function calls; 
this function makes it available to the user application. The 
address is maintained as IP:port where *IP* could be either 
an IPv6 or IPv4 address depending on what was provided by the 
sending application. 
 
The maximum size allowed by RMr is 64 bytes (including the 
nil string terminator), so the user must ensure that the 
destination buffer given is at least 64 bytes. The user 
application should use the RMr constant RMR_MAX_SRC to ensure 
that the buffer supplied is large enough, and to protect 
against future RMr enhancements which might increase the 
address buffer size requirement. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a pointer to the destination buffer is given as a 
convenience to the user programme. On failure, a nil pointer 
is returned and the value of errno is set. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If an error occurs, the value of the global variable errno 
will be set to one of the following with the indicated 
meaning. 
 
 
 
EINVAL 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_src(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_trace 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_get_trace( rmr_mbuf_t* mbuf, unsigned char* dest, int size )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_trace function will copy the trace information 
from the message into the user's allocated memory referenced 
by dest. The size parameter is assumed to be the maximum 
number of bytes which can be copied (size of the destination 
buffer). 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, the number of bytes actually copied is returned. 
If the return value is 0, no bytes copied, then the reason 
could be that the message pointer was nil, or the size 
parameter was <= 0. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_tralloc_msg(3), rmr_bytes2xact(3), 
rmr_bytes2meid(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_get_trlen(3), rmr_init(3), 
rmr_init_trace(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3), 
rmr_set_trace(3), rmr_trace_ref(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_trlen 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_get_trlen( rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
Given a message buffer, this function returns the amount of 
space (bytes) that have been allocated for trace data. If no 
trace data has been allocated, then 0 is returned. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The number of bytes allocated for trace information in the 
given message. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_trace(3), rmr_init(3), rmr_init_trace(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_set_trace(3), rmr_tralloc_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_get_xact 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 char* rmr_get_xact( rmr_mbuf_t* mbuf, unsigned char* dest )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_get_xact function will copy the transaction field 
from the message into the *dest* buffer provided by the user. 
The buffer referenced by *dest* is assumed to be at least 
RMR_MAX_XID bytes in length. If *dest* is NULL, then a buffer 
is allocated (the calling application is expected to free 
when the buffer is no longer needed). 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a pointer to the extracted string is returned. If 
*dest* was supplied, then this is just a pointer to the 
caller's buffer. If *dest* was NULL, this is a pointer to the 
allocated buffer. If an error occurs, a nil pointer is 
returned and errno is set as described below. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If an error occurs, the value of the global variable errno 
will be set to one of the following with the indicated 
meaning. 
 
 
 
EINVAL 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
   
 
ENOMEM 
   
  A nil pointer was passed for *dest,* however it was not 
  possible to allocate a buffer using malloc(). 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2xact(3), rmr_bytes2meid(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_rcvfd(3), 
rmr_get_meid(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_init 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void* rmr_init( char* proto_port, int max_msg_size, int flags );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_init function prepares the environment for sending 
and receiving messages. It does so by establishing a worker 
thread (pthread) which subscribes to a route table generator 
which provides the necessary routing information for the RMR 
library to send messages. 
 
*Port* is used to listen for connection requests from other 
RMR based applications. The *max_msg_size* parameter is used 
to allocate receive buffers and is the maximum message size 
which the application expects to receive. This value is the 
sum of **both** the maximum payload size **and** the maximum 
trace data size. This value is also used as the default 
message size when allocating message buffers. Messages 
arriving which are longer than the given maximum will be 
dropped without notification to the application. A warning is 
written to standard error for the first message which is too 
large on each connection. 
 
*Flags* allows for selection of some RMr options at the time 
of initialisation. These are set by ORing RMRFL constants 
from the RMr header file. Currently the following flags are 
supported: 
 
 
 
RMRFL_NONE 
   
  No flags are set. 
   
 
RMRFL_NOTHREAD 
   
  The route table collector thread is not to be started. 
  This should only be used by the route table generator 
  application if it is based on RMr. 
   
 
RMRFL_MTCALL 
   
  Enable multi-threaded call support. 
   
 
RMRFL_NOLOCK 
   
  Some underlying transport providers (e.g. SI95) enable 
  locking to be turned off if the user application is single 
  threaded, or otherwise can guarantee that RMR functions 
  will not be invoked concurrently from different threads. 
  Turning off locking can help make message receipt more 
  efficient. If this flag is set when the underlying 
  transport does not support disabling locks, it will be 
  ignored. 
 
 
Multi-threaded Calling 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The support for an application to issue a *blocking call* by 
the rmr_call() function was limited such that only user 
applications which were operating in a single thread could 
safely use the function. Further, timeouts were message count 
based and not time unit based. Multi-threaded call support 
adds the ability for a user application with multiple threads 
to invoke a blocking call function with the guarantee that 
the correct response message is delivered to the thread. The 
additional support is implemented with the *rmr_mt_call()* 
and *rmr_mt_rcv()* function calls. 
 
Multi-threaded call support requires the user application to 
specifically enable it when RMr is initialised. This is 
necessary because a second, dedicated, receiver thread must 
be started, and requires all messages to be examined and 
queued by this thread. The additional overhead is minimal, 
queuing information is all in the RMr message header, but as 
an additional process is necessary the user application must 
"opt in" to this approach. 
 
 
ENVIRONMENT 
-------------------------------------------------------------------------------------------- 
 
As a part of the initialisation process rmr_init will look 
into the available environment variables to influence it's 
setup. The following variables will be used when found. 
 
 
 
RMR_ASYNC_CONN 
   
  Allows the async connection mode to be turned off (by 
  setting the value to 0. When set to 1, or missing from the 
  environment, RMR will invoke the connection interface in 
  the transport mechanism using the non-blocking (async) 
  mode. This will likely result in many "soft failures" 
  (retry) until the connection is established, but allows 
  the application to continue unimpeeded should the 
  connection be slow to set up. 
   
 
RMR_BIND_IF 
   
  This provides the interface that RMR will bind listen 
  ports to allowing for a single interface to be used rather 
  than listening across all interfaces. This should be the 
  IP address assigned to the interface that RMR should 
  listen on, and if not defined RMR will listen on all 
  interfaces. 
   
 
RMR_CTL_PORT 
   
  This variable defines the port that RMR should open for 
  communications with Route Manager, and other RMR control 
  applications. If not defined, the port 4561 is assumed. 
   
  Previously, the RMR_RTG_SVC (route table generator service 
  port) was used to define this port. However, a future 
  version of Route Manager will require RMR to connect and 
  request tables, thus that variable is now used to supply 
  the Route Manager well known address and port. 
   
  To maintain backwards compatablibility with the older 
  Route Manager versions, the presence of this variable in 
  the environment will shift RMR's behaviour with respect to 
  the default value used when RMR_RTG_SVC is **not** 
  defined. 
   
  When RMR_CTL_PORT is **defined:** RMR assumes that Route 
  Manager requires RMR to connect and request table updates 
  is made, and the default well known address for Route 
  manager is used (routemgr:4561). 
   
  When RMR_CTL_PORT is **undefined:** RMR assumes that Route 
  Manager will connect and push table updates, thus the 
  default listen port (4561) is used. 
   
  To avoid any possible misinterpretation and/or incorrect 
  assumptions on the part of RMR, it is recommended that 
  both the RMR_CTL_PORT and RMR_RTG_SVC be defined. In the 
  case where both variables are defined, RMR will behave 
  exactly as is communicated with the variable's values. 
   
 
RMR_RTG_SVC 
   
  The value of this variable depends on the Route Manager in 
  use. 
   
  When the Route Manager is expecting to connect to an xAPP 
  and push route tables, this variable must indicate the 
  port which RMR should use to listen for these connections. 
   
  When the Route Manager is expecting RMR to connect and 
  request a table update during initialisation, the variable 
  should be the host of the Route Manager process. 
   
  The RMR_CTL_PORT variable (added with the support of 
  sending table update requests to Route manager), controls 
  the behaviour if this variable is not set. See the 
  description of that variable for details. 
   
 
RMR_HR_LOG 
   
  By default RMR writes messages to standard error 
  (incorrectly referred to as log messages) in human 
  readable format. If this environment variable is set to 0, 
  the format of standard error messages might be written in 
  some format not easily read by humans. If missing, a value 
  of 1 is assumed. 
   
 
RMR_LOG_VLEVEL 
   
  This is a numeric value which corresponds to the verbosity 
  level used to limit messages written to standard error. 
  The lower the number the less chatty RMR functions are 
  during execution. The following is the current 
  relationship between the value set on this variable and 
  the messages written: 
   
 
0 
   
  Off; no messages of any sort are written. 
   
 
1 
   
  Only critical messages are written (default if this 
  variable does not exist) 
   
 
2 
   
  Errors and all messages written with a lower value. 
   
 
3 
   
  Warnings and all messages written with a lower value. 
   
 
4 
   
  Informational and all messages written with a lower 
  value. 
   
 
5 
   
  Debugging mode -- all messages written, however this 
  requires RMR to have been compiled with debugging 
  support enabled. 
 
 
 
RMR_RTG_ISRAW 
   
  **Deprecated.** Should be set to 1 if the route table 
  generator is sending "plain" messages (not using RMR to 
  send messages, 0 if the rtg is using RMR to send. The 
  default is 1 as we don't expect the rtg to use RMR. 
   
  This variable is only recognised when using the NNG 
  transport library as it is not possible to support NNG 
  "raw" communications with other transport libraries. It is 
  also necessary to match the value of this variable with 
  the capabilities of the Route Manager; at some point in 
  the future RMR will assume that all Route Manager messages 
  will arrive via an RMR connection and will ignore this 
  variable. 
 
RMR_SEED_RT 
   
  This is used to supply a static route table which can be 
  used for debugging, testing, or if no route table 
  generator process is being used to supply the route table. 
  If not defined, no static table is used and RMR will not 
  report *ready* until a table is received. The static route 
  table may contain both the route table (between newrt 
  start and end records), and the MEID map (between meid_map 
  start and end records) 
 
RMR_SRC_ID 
   
  This is either the name or IP address which is placed into 
  outbound messages as the message source. This will used 
  when an RMR based application uses the rmr_rts_msg() 
  function to return a response to the sender. If not 
  supplied RMR will use the hostname which in some container 
  environments might not be routable. 
   
  The value of this variable is also used for Route Manager 
  messages which are sent via an RMR connection. 
 
RMR_VCTL_FILE 
   
  This supplies the name of a verbosity control file. The 
  core RMR functions do not produce messages unless there is 
  a critical failure. However, the route table collection 
  thread, not a part of the main message processing 
  component, can write additional messages to standard 
  error. If this variable is set, RMR will extract the 
  verbosity level for these messages (0 is silent) from the 
  first line of the file. Changes to the file are detected 
  and thus the level can be changed dynamically, however RMR 
  will only suss out this variable during initialisation, so 
  it is impossible to enable verbosity after startup. 
 
RMR_WARNINGS 
   
  If set to 1, RMR will write some warnings which are 
  non-performance impacting. If the variable is not defined, 
  or set to 0, RMR will not write these additional warnings. 
 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_init function returns a void pointer (a contex if you 
will) that is passed as the first parameter to nearly all 
other RMR functions. If rmr_init is unable to properly 
initialise the environment, NULL is returned and errno is set 
to an appropriate value. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following error values are specifically set by this RMR 
function. In some cases the error message of a system call is 
propagated up, and thus this list might be incomplete. 
 
 
ENOMEM 
   
  Unable to allocate memory. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
    void*  uh;
    rmr_mbuf* buf = NULL;
    uh = rmr_init( "43086", 4096, 0 );
    buf = rmr_rcv_msg( uh, buf );
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_mt_call(3), rmr_mt_rcv(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_init_trace 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void* rmr_init_trace( void* ctx )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_init_trace function establishes the default trace 
space placed in each message buffer allocated with 
rmr_alloc_msg(). If this function is never called, then no 
trace space is allocated by default into any message buffer. 
 
Trace space allows the user application to pass some trace 
token, or other data with the message, but outside of the 
payload. Trace data may be added to any message with 
rmr_set_trace(), and may be extracted from a message with 
rmr_get_trace(). The number of bytes that a message contains 
for/with trace data can be determined by invoking 
rmr_get_trlen(). 
 
This function may be safely called at any time during the 
life of the user programme to (re)set the default trace space 
reserved. If the user programme needs to allocate a message 
with trace space of a different size than is allocated by 
default, without fear of extra overhead of reallocating a 
message later, the rmr_tralloc_msg() function can be used. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
A value of 1 is returned on success, and 0 on failure. A 
failure indicates that the RMr context (a void pointer passed 
to this function was not valid. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_tr_alloc_msg(3), rmr_call(3), 
rmr_free_msg(3), rmr_get_rcvfd(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_set_trace(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_mt_call 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 extern rmr_mbuf_t* rmr_mt_call( void* vctx, rmr_mbuf_t* msg, int id, int timeout );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_mt_call function sends the user application message 
to a remote endpoint, and waits for a corresponding response 
message before returning control to the user application. The 
user application supplies a completed message buffer, as it 
would for a rmr_send_msg call, but unlike with a send, the 
buffer returned will have the response from the application 
that received the message. The thread invoking the 
*rmr_mt_call()* will block until a message arrives or until 
*timeout* milliseconds has passed; which ever comes first. 
Using a timeout value of zero (0) will cause the thread to 
block without a timeout. 
 
The *id* supplied as the third parameter is an integer in the 
range of 2 through 255 inclusive. This is a caller defined 
"thread number" and is used to match the response message 
with the correct user application thread. If the ID value is 
not in the proper range, the attempt to make the call will 
fail. 
 
Messages which are received while waiting for the response 
are queued on a *normal* receive queue and will be delivered 
to the user application with the next invocation of 
*rmr_mt_rcv()* or *rmr_rvv_msg().* by RMR, and are returned 
to the user application when rmr_rcv_msg is invoked. These 
messages are returned in th order received, one per call to 
rmr_rcv_msg. 
 
NOTE: Currently the multi-threaded functions are supported 
only when the NNG transport mechanism is being used. It will 
not be possible to link a programme using the Nanomsg version 
of the library when references to this function are present. 
 
The Transaction ID 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The user application is responsible for setting the value of 
the transaction ID field before invoking *rmr_mt_call.* The 
transaction ID is a RMR_MAX_XID byte field that is used to 
match the response message when it arrives. RMr will compare 
**all** of the bytes in the field, so the caller must ensure 
that they are set correctly to avoid missing the response 
message. (The application which returns the response message 
is also expected to ensure that the return buffer has the 
matching transaction ID. This can be done transparently if 
the application uses the *rmr_rts_msg()* function and does 
not adjust the transaction ID. 
 
Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The send operations in RMr will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 
1. 
   
  The message is sent without error 
   
 
2. 
   
  The underlying transport reports a * hard * failure 
   
 
3. 
   
  The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attemps ** 
without** any intervening calls to * sleep() * or * usleep(). 
* The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMr 
initialisation using the * rmr_set_stimeout() * function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 
 
Transport Level Blocking 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremly possible that during 
normal operations that retry conditions are very likely to 
happen. These are completely out of RMr's control, and there 
is nothing that RMr can do to avoid or midigate these other 
than by allowing RMr to retry the send operation, and even 
then it is possible (e.g. during connection reattempts), that 
a single retry loop is not enough to guarentee a successful 
send. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_mt_call function returns a pointer to a message 
buffer with the state set to reflect the overall state of 
call processing. If the state is RMR_OK then the buffer 
contains the response message; otherwise the state indicates 
the error encountered while attempting to send the message. 
 
If no response message is received when the timeout period 
has expired, a nil pointer will be returned (NULL). 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
These values are reflected in the state field of the returned 
message. 
 
 
 
RMR_OK 
   
  The call was successful and the message buffer references 
  the response message. 
   
 
RMR_ERR_BADARG 
   
  An argument passed to the function was invalid. 
   
 
RMR_ERR_CALLFAILED 
   
  The call failed and the value of *errno,* as described 
  below, should be checked for the specific reason. 
   
 
RMR_ERR_NOENDPT 
   
  An endpoint associated with the message type could not be 
  found in the route table. 
   
 
RMR_ERR_RETRY 
   
  The underlying transport mechanism was unable to accept 
  the message for sending. The user application can retry 
  the call operation if appropriate to do so. 
 
 
The global "variable" *errno* will be set to one of the 
following values if the overall call processing was not 
successful. 
 
 
 
ETIMEDOUT 
   
  Too many messages were queued before receiving the 
  expected response 
   
 
ENOBUFS 
   
  The queued message ring is full, messages were dropped 
   
 
EINVAL 
   
  A parameter was not valid 
   
 
EAGAIN 
   
  The underlying message system wsa interrupted or the 
  device was busy; the message was **not** sent, and user 
  application should call this function with the message 
  again. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following code bit shows one way of using the rmr_mt_call 
function, and illustrates how the transaction ID must be set. 
 
 
:: 
  
     int retries_left = 5;               // max retries on dev not available
     static rmr_mbuf_t*  mbuf = NULL;    // response msg
     msg_t*  pm;                         // private message (payload)
     m// get a send buffer and reference the payload 
     mbuf = rmr_alloc_msg( mr, RMR_MAX_RCV_BYTES );
     pm = (msg_t*) mbuf->payload;
     p// generate an xaction ID and fill in payload with data and msg type
     rmr_bytes2xact( mbuf, xid, RMR_MAX_XID );
     snprintf( pm->req, sizeof( pm->req ), "{ \\"req\\": \\"num users\\"}" );
     mbuf->mtype = MT_USR_RESP;
     
     msg = rmr_mt_call( mr, msg, my_id, 100 );    e    :// wait up to 100ms
     if( ! msg ) {               // probably a timeout and no msg received
         return NULL;            // let errno trickle up
     } 
     if( mbuf->state != RMR_OK ) {
         while( retries_left-- > 0 &&             // loop as long as eagain
                mbuf->state == RMR_ERR_RETRY && 
                (msg = rmr_mt_call( mr, msg )) != NULL && 
                mbuf->state != RMR_OK ) {
             usleep( retry_delay );
         }
     
         if( mbuf == NULL || mbuf->state != RMR_OK ) {
             rmr_free_msg( mbuf );        // safe if nil
             return NULL;
         }
     }
     // do something with mbuf
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_free_msg(3), rmr_init(3), 
rmr_mt_rcv(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_set_stimeout(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_mt_rcv 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_mt_rcv( void* vctx, rmr_mbuf_t* old_msg, int timeout );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_mt_rcv function blocks until a message is received, 
or the timeout period (milliseconds) has passed. The result 
is an RMr message buffer which references a received message. 
In the case of a timeout the state will be reflected in an 
"empty buffer" (if old_msg was not nil, or simply with the 
return of a nil pointer. If a timeout value of zero (0) is 
given, then the function will block until the next message 
received. 
 
The *vctx* pointer is the pointer returned by the rmr_init 
function. *Old_msg* is a pointer to a previously used message 
buffer or NULL. The ability to reuse message buffers helps to 
avoid alloc/free cycles in the user application. When no 
buffer is available to supply, the receive function will 
allocate one. 
 
The *old_msg* parameter allows the user to pass a previously 
generated RMr message back to RMr for reuse. Optionally, the 
user application may pass a nil pointer if no reusable 
message is available. When a timeout occurs, and old_msg was 
not nil, the state will be returned by returning a pointer to 
the old message with the state set. 
 
It is possible to use the *rmr_rcv_msg()* function instead of 
this function. Doing so might be advantagous if the user 
programme does not always start the multi-threaded mode and 
the use of *rmr_rcv_msg()* would make the flow of the code 
more simple. The advantags of using this function are the 
ability to set a timeout without using epoll, and a small 
performance gain (if multi-threaded mode is enabled, and the 
*rmr_rcv_msg()* function is used, it simply invokes this 
function without a timeout value, thus there is the small 
cost of a second call that results). Similarly, the 
*rmr_torcv_msg()* call can be used when in multi-threaded 
mode with the same "pass through" overhead to using this 
function directly. 
 
NOTE: Currently the multi-threaded functions are supported 
only when the NNG transport mechanism is being used. It will 
not be possible to link a programme using the nanomsg version 
of the library when references to this function are present. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
When a message is received before the timeout period expires, 
a pointer to the RMr message buffer which describes the 
message is returned. This will, with a high probability, be a 
different message buffer than *old_msg;* the user application 
should not continue to use *old_msg* after it is passed to 
this function. 
 
In the event of a timeout the return value will be the old 
msg with the state set, or a nil pointer if no old message 
was provided. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The *state* field in the message buffer will be set to one of 
the following values: 
 
 
 
RMR_OK 
   
  The message was received without error. 
   
 
RMR_ERR_BADARG 
   
  A parameter passed to the function was not valid (e.g. a 
  nil pointer). indicate either RMR_OK or RMR_ERR_EMPTY if 
  an empty message was received. 
   
 
RMR_ERR_EMPTY 
   
  The message received had no associated data. The length of 
  the message will be 0. 
   
 
RMR_ERR_NOTSUPP 
   
  The multi-threaded option was not enabled when RMr was 
  initialised. See the man page for *rmr_init()* for 
  details. 
   
 
RMR_ERR_RCVFAILED 
   
  A hard error occurred preventing the receive from 
  completing. 
 
When a nil pointer is returned, or any other state value was 
set in the message buffer, errno will be set to one of the 
following: 
 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
   
 
EBADF 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ENOTSUP 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EFSM 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EAGAIN 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EINTR 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETIMEDOUT 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETERM 
   
  The underlying message transport is unable to process the 
  request. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
 
:: 
  
     rmr_mbuf_t*  mbuf = NULL;   // received msg
     msg = rmr_mt_recv( mr, mbuf, 100 );     // wait up to 100ms
     if( msg != NULL ) {
         switch( msg->state ) {
             case RMR_OK:
                 printf( "got a good message\\n" );
                 break;
             case RMR_ERR_EMPTY:
                 printf( "received timed out\\n" );
                 break;
             default:
                 printf( "receive error: %d\\n", mbuf->state );
                 break;
         }
     } else {
         printf( "receive timeout (nil)\\n" );
     }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_init(3), rmr_mk_ring(3), 
rmr_mt_call(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_torcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_ring_free(3), rmr_torcv_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_payload_size 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_payload_size( rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
Given a message buffer, this function returns the amount of 
space (bytes) available for the user application to consume 
in the message payload. This is different than the message 
length available as a field in the message buffer. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The number of bytes available in the payload. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_rcv_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_rcv_msg( void* vctx, rmr_mbuf_t* old_msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_rcv_msg function blocks until a message is received, 
returning the message to the caller via a pointer to a 
rmr_mbuf_t structure type. If messages were queued while 
waiting for the response to a previous invocation of 
rmr_call, the oldest message is removed from the queue and 
returned without delay. 
 
The *vctx* pointer is the pointer returned by the rmr_init 
function. *Old_msg* is a pointer to a previously used message 
buffer or NULL. The ability to reuse message buffers helps to 
avoid alloc/free cycles in the user application. When no 
buffer is available to supply, the receive function will 
allocate one. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The function returns a pointer to the rmr_mbuf_t structure 
which references the message information (state, length, 
payload), or a NULL pointer in the case of an extreme error. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The *state* field in the message buffer will indicate either 
RMR_OK when the message receive process was successful and 
the message can be used by the caller. Depending on the 
underlying transport mechanism, one of the following RMR 
error stats may be returned: 
 
 
 
RMR_ERR_EMPTY 
   
  The message received had no payload, or was completely 
  empty. 
   
 
RMR_ERR_TIMEOUT 
   
  For some transport mechanisms, or if reading the receive 
  queue from multiple threads, it is possible for one thread 
  to find no data waiting when it queries the queue. When 
  this state is reported, the message buffer does not 
  contain message data and the user application should 
  reinvoke the receive function. 
 
 
When an RMR error state is reported, the underlying errno 
value might provide more information. The following is a list 
of possible values that might accompany the states listed 
above: 
 
RMR_ERR_EMPTY if an empty message was received. If a nil 
pointer is returned, or any other state value was set in the 
message buffer, errno will be set to one of the following: 
 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
   
 
EBADF 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ENOTSUP 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EFSM 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EAGAIN 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EINTR 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETIMEDOUT 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETERM 
   
  The underlying message transport is unable to process the 
  request. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_init(3), rmr_mk_ring(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_torcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_ring_free(3), rmr_torcv_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_ready 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_ready( void* vctx );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_ready function checks to see if a routing table has 
been successfully received and installed. The return value 
indicates the state of readiness. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
A return value of 1 (true) indicates that the routing table 
is in place and attempts to send messages can be made. When 0 
is returned (false) the routing table has not been received 
and thus attempts to send messages will fail with *no 
endpoint* errors. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_fib(3), 
rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_realloc_payload 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 extern rmr_mbuf_t* rmr_realloc_payload( rmr_mbuf_t* msg, int new_len, int copy, int clone );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_realloc_payload function will return a pointer to an 
RMR message buffer struct (rmr_mbuf_t) which has a payload 
large enough to accomodate *new_len* bytes. If necessary, the 
underlying payload is reallocated, and the bytes from the 
original payload are copied if the *copy* parameter is true 
(1). If the message passed in has a payload large enough, 
there is no additional memory allocation and copying. 
 
Cloning The Message Buffer 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
This function can also be used to generate a separate copy of 
the original message, with the desired payload size, without 
destroying the original message buffer or the original 
payload. A standalone copy is made only when the *clone* 
parameter is true (1). When cloning, the payload is copied to 
the cloned message **only** if the *copy* parameter is true. 
 
Message Buffer Metadata 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The metadata in the original message buffer (message type, 
subscription ID, and payload length) will be preserved if the 
*copy* parameter is true. When this parameter is not true 
(0), then these values are set to the uninitialised value 
(-1) for type and ID, and the length is set to 0. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_realloc_payload function returns a pointer to the 
message buffer with the payload which is large enough to hold 
*new_len* bytes. If the *clone* option is true, this will be 
a pointer to the newly cloned message buffer; the original 
message buffer pointer may still be used to referenced that 
message. It is the calling application's responsibility to 
free the memory associateed with both messages using the 
rmr_free_msg() function. 
 
When the *clone* option is not used, it is still good 
practice by the calling application to capture and use this 
reference as it is possible that the message buffer, and not 
just the payload buffer, was reallocated. In the event of an 
error, a nil pointer will be returned and the value of 
*errno* will be set to reflect the problem. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
These value of *errno* will reflect the error condition if a 
nil pointer is returned: 
 
 
 
ENOMEM 
   
  Memory allocation of the new payload failed. 
   
 
EINVAL 
   
  The pointer passed in was nil, or refrenced an invalid 
  message, or the required length was not valid. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following code bit illustrates how this function can be 
used to reallocate a buffer for a return to sender 
acknowledgement message which is larger than the message 
received. 
 
 
:: 
  
   if( rmr_payload_size( msg ) < ack_sz ) {              // received message too small for ack
     msg = rmr_realloc_payload( msg, ack_sz, 0, 0 );     // reallocate the message with a payload big enough
     if( msg == NULL ) {
       fprintf( stderr, "[ERR] realloc returned a nil pointer: %s\\n", strerror( errno ) );
     } else {
     }    e// populate and send ack message
     }}
 }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_set_stimeout(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_rts_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t*  rmr_rts_msg( void* vctx, rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_rts_msg function sends a message returning it to the 
endpoint which sent the message rather than selecting an 
endpoint based on the message type and routing table. Other 
than this small difference, the behaviour is exactly the same 
as rmr_send_msg. 
 
Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The send operations in RMr will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 
1. 
   
  The message is sent without error 
   
 
2. 
   
  The underlying transport reports a * hard * failure 
   
 
3. 
   
  The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attemps ** 
without** any intervening calls to * sleep() * or * usleep(). 
* The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMr 
initialisation using the * rmr_set_stimeout() * function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 
 
Transport Level Blocking 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremly possible that during 
normal operations that retry conditions are very likely to 
happen. These are completely out of RMr's control, and there 
is nothing that RMr can do to avoid or midigate these other 
than by allowing RMr to retry the send operation, and even 
then it is possible (e.g. during connection reattempts), that 
a single retry loop is not enough to guarentee a successful 
send. 
 
PAYLOAD SIZE 
-------------------------------------------------------------------------------------------- 
 
When crafting a response based on a received message, the 
user application must take care not to write more bytes to 
the message payload than the allocated message has. In the 
case of a received message, it is possible that the response 
needs to be larger than the payload associated with the 
inbound message. In order to use the return to sender 
function, the source infomration in the orignal message must 
be present in the response; information which cannot be added 
to a message buffer allocated through the standard RMR 
allocation function. To allocate a buffer with a larger 
payload, and which retains the necessary sender data needed 
by this function, the *rmr_realloc_payload()* function must 
be used to extend the payload to a size suitable for the 
response. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a new message buffer, with an empty payload, is 
returned for the application to use for the next send. The 
state in this buffer will reflect the overall send operation 
state and should be RMR_OK. 
 
If the state in the returned buffer is anything other than 
UT_OK, the user application may need to attempt a 
retransmission of the message, or take other action depending 
on the setting of errno as described below. 
 
In the event of extreme failure, a NULL pointer is returned. 
In this case the value of errno might be of some use, for 
documentation, but there will be little that the user 
application can do other than to move on. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following values may be passed back in the *state* field 
of the returned message buffer. 
 
 
 
RMR_ERR_BADARG 
   
  The message buffer pointer did not refer to a valid 
  message. 
 
RMR_ERR_NOHDR 
   
  The header in the message buffer was not valid or 
  corrupted. 
 
RMR_ERR_NOENDPT 
   
  The message type in the message buffer did not map to a 
  known endpoint. 
 
RMR_ERR_SENDFAILED 
   
  The send failed; errno has the possible reason. 
 
 
The following values may be assigned to errno on failure. 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid, or the 
  underlying message processing environment was unable to 
  interpret the message. 
   
 
ENOKEY 
   
  The header information in the message buffer was invalid. 
   
 
ENXIO 
   
  No known endpoint for the message could be found. 
   
 
EMSGSIZE 
   
  The underlying transport refused to accept the message 
  because of a size value issue (message was not attempted 
  to be sent). 
   
 
EFAULT 
   
  The message referenced by the message buffer is corrupt 
  (NULL pointer or bad internal length). 
   
 
EBADF 
   
  Internal RMR error; information provided to the message 
  transport environment was not valid. 
   
 
ENOTSUP 
   
  Sending was not supported by the underlying message 
  transport. 
   
 
EFSM 
   
  The device is not in a state that can accept the message. 
   
 
EAGAIN 
   
  The device is not able to accept a message for sending. 
  The user application should attempt to resend. 
   
 
EINTR 
   
  The operation was interrupted by delivery of a signal 
  before the message was sent. 
   
 
ETIMEDOUT 
   
  The underlying message environment timed out during the 
  send process. 
   
 
ETERM 
   
  The underlying message environment is in a shutdown state. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_ready(3), rmr_fib(3), 
rmr_has_str(3), rmr_set_stimeout(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_send_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_send_msg( void* vctx, rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_send_msg function accepts a message buffer from the 
user application and attempts to send it. The destination of 
the message is selected based on the message type specified 
in the message buffer, and the matching information in the 
routing tables which are currently in use by the RMR library. 
This may actually result in the sending of the message to 
multiple destinations which could degrade expected overall 
performance of the user application. (Limiting excessive 
sending of messages is the responsibility of the 
application(s) responsible for building the routing table 
used by the RMR library, and not the responsibility of the 
library.) 
 
Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The send operations in RMr will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 
1. 
   
  The message is sent without error 
   
 
2. 
   
  The underlying transport reports a * hard * failure 
   
 
3. 
   
  The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attemps ** 
without** any intervening calls to * sleep() * or * usleep(). 
* The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMr 
initialisation using the * rmr_set_stimeout() * function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 
 
Transport Level Blocking 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremly possible that during 
normal operations that retry conditions are very likely to 
happen. These are completely out of RMr's control, and there 
is nothing that RMr can do to avoid or midigate these other 
than by allowing RMr to retry the send operation, and even 
then it is possible (e.g. during connection reattempts), that 
a single retry loop is not enough to guarentee a successful 
send. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a new message buffer, with an empty payload, is 
returned for the application to use for the next send. The 
state in this buffer will reflect the overall send operation 
state and will be RMR_OK when the send was successful. 
 
When the message cannot be successfully sent this function 
will return the unsent (original) message buffer with the 
state set to indicate the reason for failure. The value of 
*errno* may also be set to reflect a more detailed failure 
reason if it is known. 
 
In the event of extreme failure, a NULL pointer is returned. 
In this case the value of errno might be of some use, for 
documentation, but there will be little that the user 
application can do other than to move on. 
 
**CAUTION:** In some cases it is extremely likely that the 
message returned by the send function does **not** reference 
the same memory structure. Thus is important for the user 
programme to capture the new pointer for future use or to be 
passed to rmr_free(). If you are experiencing either double 
free errors or segment faults in either rmr_free() or 
rmr_send_msg(), ensure that the return value from this 
function is being captured and used. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following values may be passed back in the *state* field 
of the returned message buffer. 
 
 
 
RMR_RETRY 
   
  The message could not be sent, but the underlying 
  transport mechanism indicates that the failure is 
  temporary. If the send operation is tried again it might 
  be successful. 
 
RMR_SEND_FAILED 
   
  The send operation was not successful and the underlying 
  transport mechanism indicates a permanent (hard) failure; 
  retrying the send is not possible. 
 
RMR_ERR_BADARG 
   
  The message buffer pointer did not refer to a valid 
  message. 
 
RMR_ERR_NOHDR 
   
  The header in the message buffer was not valid or 
  corrupted. 
 
RMR_ERR_NOENDPT 
   
  The message type in the message buffer did not map to a 
  known endpoint. 
 
 
The following values may be assigned to errno on failure. 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid, or the 
  underlying message processing environment was unable to 
  interpret the message. 
   
 
ENOKEY 
   
  The header information in the message buffer was invalid. 
   
 
ENXIO 
   
  No known endpoint for the message could be found. 
   
 
EMSGSIZE 
   
  The underlying transport refused to accept the message 
  because of a size value issue (message was not attempted 
  to be sent). 
   
 
EFAULT 
   
  The message referenced by the message buffer is corrupt 
  (NULL pointer or bad internal length). 
   
 
EBADF 
   
  Internal RMR error; information provided to the message 
  transport environment was not valid. 
   
 
ENOTSUP 
   
  Sending was not supported by the underlying message 
  transport. 
   
 
EFSM 
   
  The device is not in a state that can accept the message. 
   
 
EAGAIN 
   
  The device is not able to accept a message for sending. 
  The user application should attempt to resend. 
   
 
EINTR 
   
  The operation was interrupted by delivery of a signal 
  before the message was sent. 
   
 
ETIMEDOUT 
   
  The underlying message environment timed out during the 
  send process. 
   
 
ETERM 
   
  The underlying message environment is in a shutdown state. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following is a simple example of how the rmr_send_msg 
function is called. In this example, the send message buffer 
is saved between calls and reused eliminating alloc/free 
cycles. 
 
 
:: 
  
     static rmr_mbuf_t*  send_msg = NULL;        // message to send; reused on each call
     msg_t*  send_pm;                            // payload for send
     msg_t*  pm;                                 // our message format in the received payload
     mif( send_msg  == NULL ) {
         send_msg = rmr_alloc_msg( mr, MAX_SIZE );    r// new buffer to send
      }
      // reference payload and fill in message type
     pm = (msg_t*) send_msg->payload;
     send_msg->mtype = MT_ANSWER;
     msg->len = generate_data( pm );       // something that fills the payload in
     msg = rmr_send_msg( mr, send_msg );   // ensure new pointer used after send
     mif( ! msg ) {
     m    !return ERROR;
     m} else {
     m    sif( msg->state != RMR_OK ) {
     m    s    m// check for RMR_ERR_RETRY, and resend if needed
     m    s    m// else return error
     m    s}
     m}
     mreturn OK;
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_mk_ring(3), 
rmr_ring_free(3), rmr_torcv_rcv(3), rmr_wh_send_msg(3) 
 
 
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
be useful for applications which must send messages as a 
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
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_set_stimeout 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_set_stimeout( void* vctx, int rloops );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_set_stimeout function sets the configuration for how 
RMr will retry message send operations which complete with 
either a *timeout* or *again* completion value. (Send 
operations include all of the possible message send 
functions: *rmr_send_msg(), rmr_call(), rmr_rts_msg()* and 
*rmr_wh_send_msg().* The *rloops* parameter sets the maximum 
number of retry loops that will be attempted before giving up 
and returning the unsuccessful state to the user application. 
Each retry loop is approximately 1000 attempts, and RMr does 
**not** invoke any sleep function between retries in the 
loop; a small, 1 mu-sec, sleep is executed between loop sets 
if the *rloops* value is greater than 1. 
 
 
Disabling Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
By default, the send operations will execute with an *rloop* 
setting of 1; each send operation will attempt to resend the 
message approximately 1000 times before giving up. If the 
user application does not want to have send operations retry 
when the underlying transport mechanism indicates *timeout* 
or *again,* the application should invoke this function and 
pass a value of 0 (zero) for *rloops.* With this setting, all 
RMr send operations will attempt a send operation only 
**once,** returning immediately to the caller with the state 
of that single attempt. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
This function returns a -1 to indicate that the *rloops* 
value could not be set, and the value *RMR_OK* to indicate 
success. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
Currently errno is **not** set by this function; the only 
cause of a failure is an invalid context (*vctx*) pointer. 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following is a simple example of how the rmr_set_stimeout 
function is called. 
 
 
:: 
  
     #define NO_FLAGS    0
     char*    Oport = "43086";     // port for message router listen
     int         rmax_size = 4096;    // max message size for default allocations
     void*     mr_context;         // message router context
     mr_context = rmr_init( port, max_size, NO_FLAGS );
     if( mr_context != NULL ) {
         rmr_set_stimeout( mr_context, 0 );    // turn off retries
     }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_mk_ring(3), 
rmr_ring_free(3), rmr_send_msg(3), rmr_torcv_rcv(3), 
rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_set_trace 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_set_trace( rmr_mbuf_t* mbuf, unsigned char* data, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_set_trace function will copy len bytes from data into 
the trace portion of mbuf. If the trace area of mbuf is not 
the correct size, the message buffer will be reallocated to 
ensure that enough space is available for the trace data. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_set_trace function returns the number of bytes 
successfully copied to the message. If 0 is returned either 
the message pointer was nil, or the size in the parameters 
was <= 0. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_tralloc_msg(3), rmr_bytes2xact(3), 
rmr_bytes2payload(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_get_meid(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_init(3), rmr_init_trace(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), rmr_mk_ring(3), 
rmr_ring_free(3), rmr_str2meid(3), rmr_str2xact(3), 
rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_set_trace 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 #include <rmr/rmr_logging.h>
 void rmr_set_vlevel( int new_level )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_set_vlevel allows the user programme to set the 
verbosity level which is used to determine the messages RMR 
writes to standard error. The new_vlevel value must be one of 
the following constants which have the indicated meanings: 
 
 
RMR_VL_OFF 
   
  Turns off all message writing. This includes the stats and 
  debugging messages generated by the route collector thread 
  which are normally affected only by the externally managed 
  verbose level file (and related environment variable). 
   
 
RMR_VL_CRIT 
   
  Write only messages of critical importance. From the point 
  of view of RMR, when a critical proper behaviour of the 
  library cannot be expected or guaranteed. 
 
RMR_VL_ERR 
   
  Include error messages in the output. An error is an event 
  from which RMR has no means to recover. Continued proper 
  execution is likely except where the affected connection 
  and/or component mentioned in the error is concerned. 
 
RMR_VL_WARN 
   
  Include warning messages in the output. A warning 
  indicates an event which is not considered to be normal, 
  but is expected and continued acceptable behaviour of the 
  system is assured. 
 
RMR_VL_INFO 
   
  Include informational messagees in the output. 
  Informational messages include some diagnostic information 
  which explain the activities of RMR. 
 
RMR_VL_DEBUG 
   
  Include all debugging messages in the output. Debugging 
  must have also been enabled during the build as a 
  precaution to accidentally enabling this level of output 
  as it can grossly affect performance. 
 
 
generally RMR does not write messages to the standard error 
device from *critical path* functions, therefore it is 
usually not harmful to enable a verbosity level of either 
RMR_VL_CRIT, or RMR_VL_ERR. 
 
Messages written from the route table collection thread are 
still governed by the value placed into the verbose level 
control file (see the man page for rmr_init()); those 
messages are affected only when logging is completely 
disabled by passing RMR_VL_OFF to this function. 
 
The verbosity level can also be set via an environment 
variable prior to the start of the RMR based application. The 
environment variable is read only during initialisation; if 
the programme must change the value during execution, this 
function must be used. The default value, if this function is 
never called, and the environment variable is not present, is 
RMR_VL_ERR. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_init(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_str2meid 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_str2meid( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_str2meid function will copy the string pointed to by 
src to the managed entity ID (meid) field in the given 
message. The field is a fixed length, gated by the constant 
RMR_MAX_MEID and if string length is larger than this value, 
then **nothing** will be copied. (Note, this differs slightly 
from the behaviour of the lrmr_bytes2meid() function.) 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, the value RMR_OK is returned. If the string 
cannot be copied to the message, the return value will be one 
of the errors listed below. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If the return value is not RMR_OK, then it will be set to one 
of the values below. 
 
 
 
RMR_ERR_BADARG 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
   
 
RMR_ERR_OVERFLOW 
   
  The length passed in was larger than the maximum length of 
  the field; only a portion of the source bytes were copied. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_meid(3), rmr_get_rcvfd(3), rmr_payload_size(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_bytes2meid(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_str2xact 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_str2xact( rmr_mbuf_t* mbuf, unsigned char* src, int len )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_str2xact function will copy the string pointed to by 
src to the transaction ID (xaction) field in the given 
message. The field is a fixed length, gated by the constant 
RMR_MAX_XID and if string length is larger than this value, 
then **nothing** will be copied. (Note, this differs slightly 
from the behaviour of the lrmr_bytes2xact() function.) 
 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, the value RMR_OK is returned. If the string 
cannot be copied to the message, the return value will be 
one of the errors listed below. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
If the return value is not RMR_OK, then it will be set to 
one of the values below. 
 
 
RMR_ERR_BADARG 
   
  The message, or an internal portion of the message, was 
  corrupted or the pointer was invalid. 
   
 
RMR_ERR_OVERFLOW 
   
  The length passed in was larger than the maximum length of 
  the field; only a portion of the source bytes were copied. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_bytes2meid(3), rmr_bytes2xact(3), 
rmr_call(3), rmr_free_msg(3), rmr_get_meid(3), 
rmr_get_rcvfd(3), rmr_get_xact(3), rmr_payload_size(3), 
rmr_send_msg(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_str2meid(3), rmr_wh_open(3), rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
RMR support functions 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 #include <rmr/ring_inline.h>
 char* rmr_fib( char* fname );
 int rmr_has_str( char const* buf, char const* str, char sep, int max );
 int rmr_tokenise( char* buf, char** tokens, int max, char sep );
 void* rmr_mk_ring( int size );
 void rmr_ring_free( void* vr );
 static inline void* rmr_ring_extract( void* vr )
 static inline int rmr_ring_insert( void* vr, void* new_data )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
These functions support the RMR library, and are made 
available to user applications as some (e.g. route table 
generators) might need and/or want to make use of them. The 
rmr_fib function accepts a file name and reads the entire 
file into a single buffer. The intent is to provide an easy 
way to load a static route table without a lot of buffered 
I/O hoops. 
 
The rmr_has_str function accepts a *buffer* containing a set 
of delimited tokens (e.g. foo,bar,goo) and returns true if 
the target string, *str,* matches one of the tokens. The 
*sep* parameter provides the separation character in the 
buffer (e.g a comma) and *max* indicates the maximum number 
of tokens to split the buffer into before checking. 
 
The rmr_tokenise function is a simple tokeniser which splits 
*buf* into tokens at each occurrence of *sep*. Multiple 
occurrences of the separator character (e.g. a,,b) result in 
a nil token. Pointers to the tokens are placed into the 
*tokens* array provided by the caller which is assumed to 
have at least enough space for *max* entries. 
 
The rmr_mk_ring function creates a buffer ring with *size* 
entries. 
 
The rmr_ring_free function accepts a pointer to a ring 
context and frees the associated memory. 
 
The rmr_ring_insert and rmr_ring_extract functions are 
provided as static inline functions via the 
*rmr/ring_inline.h* header file. These functions both accept 
the ring *context* returned by mk_ring, and either insert a 
pointer at the next available slot (tail) or extract the data 
at the head. 
 
RETURN VALUES 
-------------------------------------------------------------------------------------------- 
 
The following are the return values for each of these 
functions. 
 
The rmr_fib function returns a pointer to the buffer 
containing the contents of the file. The buffer is terminated 
with a single nil character (0) making it a legitimate C 
string. If the file was empty or nonexistent, a buffer with 
an immediate nil character. If it is important to the calling 
programme to know if the file was empty or did not exist, the 
caller should use the system stat function call to make that 
determination. 
 
The rmr_has_str function returns 1 if *buf* contains the 
token referenced by &ita and false (0) if it does not. On 
error, a -1 value is returned and errno is set accordingly. 
 
The rmr_tokenise function returns the actual number of token 
pointers placed into *tokens* 
 
The rmr_mk_ring function returns a void pointer which is the 
*context* for the ring. 
 
The rmr_ring_insert function returns 1 if the data was 
successfully inserted into the ring, and 0 if the ring is 
full and the pointer could not be deposited. 
 
The rmr_ring_extract will return the data which is at the 
head of the ring, or NULL if the ring is empty. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
Not many of these functions set the value in errno, however 
the value may be one of the following: 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_send_msg(3), rmr_rcv_msg(3), 
rmr_rcv_specific(3), rmr_rts_msg(3), rmr_ready(3), 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_torcv_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_torcv_msg( void* vctx, rmr_mbuf_t* old_msg, int ms_to );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_torcv_msg function will pause for *ms_to* 
milliseconds waiting for a message to arrive. If a message 
arrives before the timeout expires the message buffer 
returned will have a status of RMR_OK and the payload will 
contain the data received. If the timeout expires before the 
message is received, the status will have the value 
RMR_ERR_TIMEOUT. When a received message is returned the 
message buffer will also contain the message type and length 
set by the sender. If messages were queued while waiting for 
the response to a previous invocation of rmr_call, the oldest 
message is removed from the queue and returned without delay. 
 
The *vctx* pointer is the pointer returned by the rmr_init 
function. *Old_msg* is a pointer to a previously used message 
buffer or NULL. The ability to reuse message buffers helps to 
avoid alloc/free cycles in the user application. When no 
buffer is available to supply, the receive function will 
allocate one. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The function returns a pointer to the rmr_mbuf_t structure 
which references the message information (state, length, 
payload), or a NULL pointer in the case of an extreme error. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The *state* field in the message buffer will be one of the 
following: 
 
 
 
RMR_OK 
   
  The message buffer (payload) references the received data. 
   
 
RMR_ERR_INITFAILED 
   
  The first call to this function must initialise an 
  underlying system notification mechanism. On failure, this 
  error is returned and errno will have the system error 
  status set. If this function fails to intialise, the poll 
  mechansim, it is likely that message receives will never 
  be successful. 
   
 
RMR_ERR_TIMEOUT 
   
  The timeout expired before a complete message was 
  received. All other fields in the message buffer are not 
  valid. 
   
 
RMR_ERR_EMPTY 
   
  A message was received, but it had no payload. All other 
  fields in the message buffer are not valid. 
 
 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid. 
   
 
EBADF 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ENOTSUP 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EFSM 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EAGAIN 
   
  The underlying message transport is unable to process the 
  request. 
   
 
EINTR 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETIMEDOUT 
   
  The underlying message transport is unable to process the 
  request. 
   
 
ETERM 
   
  The underlying message transport is unable to process the 
  request. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_init(3), rmr_payload_size(3), 
rmr_rcv_msg(3), rmr_send_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_trace_ref 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_trace_ref( rmr_mbuf_t* mbuf, int* sizeptr )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_trace_ref function return a pointer to the trace area 
in the message, and optionally populate the user programme 
supplied size integer with the trace area size, if *sizeptr* 
is not nil. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a void pointer to the trace area of the message 
is returned. A nil pointer is returned if the message has no 
trace data area allocated, or if the message itself is 
invalid. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_tralloc_msg(3), rmr_bytes2xact(3), 
rmr_bytes2meid(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_get_trlen(3), rmr_init(3), 
rmr_init_trace(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_str2meid(3), 
rmr_str2xact(3), rmr_wh_open(3), rmr_wh_send_msg(3), 
rmr_set_trace(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_tralloc_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_tralloc_msg( void* vctx, int size, 
                              int trace_size, unsigned const char *tr_data );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_alloc_msg function is used to allocate a buffer which 
the user programme can write into and then send through the a 
library. The buffer is allocated such that sending it 
requires no additional copying from the buffer as it passes 
through the underlying transport mechanism. 
 
The *size* parameter is used to set the payload length in the 
message and If it is 0, then the default size supplied on the 
*rmr_init* call will be used. In addition to allocating the 
payload, a space in the buffer is reserved for *trace* data 
(tr_size bytes), and the bytes pointed to by *tr_data* are 
copied into that portion of the message. The *vctx* parameter 
is the void context pointer that was returned by the 
*rmr_init* function. 
 
The pointer to the message buffer returned is a structure 
which has some user application visible fields; the structure 
is described in rmr.h, and is illustrated below. 
 
 
:: 
  
 typedef struct {
     int state;
     int mtype;
     int len;
     unsigned char* payload;
     unsigned char* xaction;
 } rmr_mbuf_t;
 
 
 
 
 
state 
   
  Is the current buffer state. Following a call to 
  rmr_send_msg the state indicates whether the buffer was 
  successfully sent which determines exactly what the 
  payload points to. If the send failed, the payload 
  referenced by the buffer is the message that failed to 
  send (allowing the application to attempt a 
  retransmission). When the state is a_OK the buffer 
  represents an empty buffer that the application may fill 
  in in preparation to send. 
   
 
mtype 
   
  When sending a message, the application is expected to set 
  this field to the appropriate message type value (as 
  determined by the user programme). Upon send this value 
  determines how the a library will route the message. For a 
  buffer which has been received, this field will contain 
  the message type that was set by the sending application. 
   
 
len 
   
  The application using a buffer to send a message is 
  expected to set the length value to the actual number of 
  bytes that it placed into the message. This is likely less 
  than the total number of bytes that the message can carry. 
  For a message buffer that is passed to the application as 
  the result of a receive call, this will be the value that 
  the sending application supplied and should indicate the 
  number of bytes in the payload which are valid. 
   
 
payload 
   
  The payload is a pointer to the actual received data. The 
  user programme may read and write from/to the memory 
  referenced by the payload up until the point in time that 
  the buffer is used on a rmr_send, rmr_call or rmr_reply 
  function call. Once the buffer has been passed back to a a 
  library function the user programme should **NOT** make 
  use of the payload pointer. 
   
 
xaction 
   
  The *xaction* field is a pointer to a fixed sized area in 
  the message into which the user may write a transaction 
  ID. The ID is optional with the exception of when the user 
  application uses the rmr_call function to send a message 
  and wait for the reply; the underlying a processing 
  expects that the matching reply message will also contain 
  the same data in the *xaction* field. 
 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The function returns a pointer to a rmr_mbuf structure, or 
NULL on error. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
 
 
ENOMEM 
   
  Unable to allocate memory. 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_mbuf(3) rmr_call(3), rmr_free_msg(3), 
rmr_init(3), rmr_init_trace(3), rmr_get_trace(3), 
rmr_get_trlen(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_set_trace(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_open 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void rmr_close( void* vctx, rmr_whid_t whid )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_wh_close function closes the wormhole associated with 
the wormhole id passed in. Future calls to rmr_wh_send_msg 
with this ID will fail. 
 
The underlying TCP connection to the remote endpoint is 
**not** closed as this session may be reqruired for 
regularlly routed messages (messages routed based on message 
type). There is no way to force a TCP session to be closed at 
this point in time. 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_open(3), 
rmr_wh_send_msg(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_open 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 void* rmr_wh_open( void* vctx, char* target )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_wh_open function creates a direct link for sending, a 
wormhole, to another RMr based process. Sending messages 
through a wormhole requires that the connection be 
established overtly by the user application (via this 
function), and that the ID returned by rmr_wh_open be passed 
to the rmr_wh_send_msg function. 
 
*Target* is the *name* or *IP-address* combination of the 
processess that the wormhole should be connected to. *Vctx* 
is the RMr void context pointer that was returned by the 
rmr_init function. 
 
When invoked, this function immediatly attempts to connect to 
the target process. If the connection cannot be established, 
an error is returned to the caller, and no direct messages 
can be sent to the target. Once a wormhole is connected, the 
underlying transport mechanism (e.g. NNG) will provide 
reconnects should the connection be lost, however the 
handling of messages sent when a connection is broken is 
undetermined as each underlying transport mechanism may 
handle buffering and retries differently. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
The rmr_wh_open function returns a type rmr_whid_t which must 
be passed to the rmr_wh_send_msg function when sending a 
message. The id may also be tested to determine success or 
failure of the connection by using the RMR_WH_CONNECTED macro 
and passing the ID as the parameter; a result of 1 indicates 
that the connection was esablished and that the ID is valid. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following error values are specifically set by this RMR 
function. In some cases the error message of a system call is 
propagated up, and thus this list might be incomplete. 
 
 
EINVAL 
   
  A parameter passed was not valid. 
 
EACCESS 
   
  The user applicarion does not have the ability to 
  establish a wormhole to the indicated target (or maybe any 
  target). 
 
ECONNREFUSED 
   
  The connection was refused. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
    void*  rmc;
    rmr_whid_t wh;
    rmc = rmr_init( "43086", 4096, 0 ); // init context
    wh = rmr_wh_open( rmc, "localhost:6123" );
    if( !RMR_WH_CONNECTED( wh ) ) { 
     f fprintf( stderr, "unable to connect wormhole: %s\\n",
              strerror( errno ) );
    }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), 
rmr_get_rcvfd(3), rmr_payload_size(3), rmr_send_msg(3), 
rmr_rcv_msg(3), rmr_rcv_specific(3), rmr_rts_msg(3), 
rmr_ready(3), rmr_fib(3), rmr_has_str(3), rmr_tokenise(3), 
rmr_mk_ring(3), rmr_ring_free(3), rmr_wh_close(3), 
rmr_wh_send_msg(3), rmr_wh_state(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_send_msg 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 rmr_mbuf_t* rmr_wh_send_msg( void* vctx, rmr_whid_t id, rmr_mbuf_t* msg );
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_wh_send_msg function accepts a message buffer from 
the user application and attempts to send it using the 
wormhole ID provided (id). Unlike *rmr_send_msg,* this 
function attempts to send the message directly to a process 
at the other end of a wormhole which was created with 
*rmr_wh-open().* When sending message via wormholes, the 
normal RMr routing based on message type is ignored, and the 
caller may leave the message type unspecified in the message 
buffer (unless it is needed by the receiving process). 
 
The message buffer (msg) used to send is the same format as 
used for regular RMr send and reply to sender operations, 
thus any buffer allocated by these means, or calls to 
*rmr_rcv_msg()* can be passed to this function. 
 
Retries 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The send operations in RMr will retry *soft* send failures 
until one of three conditions occurs: 
 
 
 
1. 
   
  The message is sent without error 
   
 
2. 
   
  The underlying transport reports a * hard * failure 
   
 
3. 
   
  The maximum number of retry loops has been attempted 
 
 
A retry loop consists of approximately 1000 send attemps ** 
without** any intervening calls to * sleep() * or * usleep(). 
* The number of retry loops defaults to 1, thus a maximum of 
1000 send attempts is performed before returning to the user 
application. This value can be set at any point after RMr 
initialisation using the * rmr_set_stimeout() * function 
allowing the user application to completely disable retires 
(set to 0), or to increase the number of retry loops. 
 
Transport Level Blocking 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The underlying transport mechanism used to send messages is 
configured in *non-blocking* mode. This means that if a 
message cannot be sent immediately the transport mechanism 
will **not** pause with the assumption that the inability to 
send will clear quickly (within a few milliseconds). This 
means that when the retry loop is completely disabled (set to 
0), that the failure to accept a message for sending by the 
underlying mechanisms (software or hardware) will be reported 
immediately to the user application. 
 
It should be noted that depending on the underlying transport 
mechanism being used, it is extremly possible that during 
normal operations that retry conditions are very likely to 
happen. These are completely out of RMr's control, and there 
is nothing that RMr can do to avoid or midigate these other 
than by allowing RMr to retry the send operation, and even 
then it is possible (e.g. during connection reattempts), that 
a single retry loop is not enough to guarentee a successful 
send. 
 
RETURN VALUE 
-------------------------------------------------------------------------------------------- 
 
On success, a new message buffer, with an empty payload, is 
returned for the application to use for the next send. The 
state in this buffer will reflect the overall send operation 
state and should be RMR_OK. 
 
If the state in the returned buffer is anything other than 
RMR_OK, the user application may need to attempt a 
retransmission of the message, or take other action depending 
on the setting of errno as described below. 
 
In the event of extreme failure, a NULL pointer is returned. 
In this case the value of errno might be of some use, for 
documentation, but there will be little that the user 
application can do other than to move on. 
 
ERRORS 
-------------------------------------------------------------------------------------------- 
 
The following values may be passed back in the *state* field 
of the returned message buffer. 
 
 
 
RMR_ERR_WHID 
   
  The wormhole ID passed in was not associated with an open 
  wormhole, or was out of range for a valid ID. 
 
RMR_ERR_NOWHOPEN 
   
  No wormholes exist, further attempt to validate the ID are 
  skipped. 
 
RMR_ERR_BADARG 
   
  The message buffer pointer did not refer to a valid 
  message. 
 
RMR_ERR_NOHDR 
   
  The header in the message buffer was not valid or 
  corrupted. 
 
 
The following values may be assigned to errno on failure. 
 
 
INVAL 
   
  Parameter(s) passed to the function were not valid, or the 
  underlying message processing environment was unable to 
  interpret the message. 
   
 
ENOKEY 
   
  The header information in the message buffer was invalid. 
   
 
ENXIO 
   
  No known endpoint for the message could be found. 
   
 
EMSGSIZE 
   
  The underlying transport refused to accept the message 
  because of a size value issue (message was not attempted 
  to be sent). 
   
 
EFAULT 
   
  The message referenced by the message buffer is corrupt 
  (NULL pointer or bad internal length). 
   
 
EBADF 
   
  Internal RMR error; information provided to the message 
  transport environment was not valid. 
   
 
ENOTSUP 
   
  Sending was not supported by the underlying message 
  transport. 
   
 
EFSM 
   
  The device is not in a state that can accept the message. 
   
 
EAGAIN 
   
  The device is not able to accept a message for sending. 
  The user application should attempt to resend. 
   
 
EINTR 
   
  The operation was interrupted by delivery of a signal 
  before the message was sent. 
   
 
ETIMEDOUT 
   
  The underlying message environment timed out during the 
  send process. 
   
 
ETERM 
   
  The underlying message environment is in a shutdown state. 
 
 
EXAMPLE 
-------------------------------------------------------------------------------------------- 
 
The following is a simple example of how the a wormhole is 
created (rmr_wh_open) and then how rmr_wh_send_msg function 
is used to send messages. Some error checking is omitted for 
clarity. 
 
 
:: 
  
 #include <rmr/rmr.h>    .// system headers omitted for clarity
 int main() {
    rmr_whid_t whid = -1;   // wormhole id for sending
    void* mrc;      //msg router context
         int i;
    rmr_mbuf_t*  sbuf;      // send buffer
    int     count = 0;
    mrc = rmr_init( "43086", RMR_MAX_RCV_BYTES, RMRFL_NONE );
    if( mrc == NULL ) {
       fprintf( stderr, "[FAIL] unable to initialise RMr environment\\n" );
       exit( 1 );
    }
    while( ! rmr_ready( mrc ) ) {    e    i// wait for routing table info
       sleep( 1 );
    }
    sbuf = rmr_alloc_msg( mrc, 2048 );
    while( 1 ) {
      if( whid < 0 ) {
        whid = rmr_wh_open( mrc, "localhost:6123" );  // open fails if endpoint refuses conn
        w   if( RMR_WH_CONNECTED( wh ) ) { 
            snprintf( sbuf->payload, 1024, "periodic update from sender: %d", count++ );
            sbuf->len =  strlen( sbuf->payload );
            sbuf = rmr_wh_send_msg( mrc, whid, sbuf );
           }
       }
       sleep( 5 );
    }
 }
 
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_alloc_msg(3), rmr_call(3), rmr_free_msg(3), rmr_init(3), 
rmr_payload_size(3), rmr_rcv_msg(3), rmr_rcv_specific(3), 
rmr_rts_msg(3), rmr_ready(3), rmr_fib(3), rmr_has_str(3), 
rmr_tokenise(3), rmr_mk_ring(3), rmr_ring_free(3), 
rmr_set_stimeout(3), rmr_wh_open(3), rmr_wh_close(3), 
rmr_wh_state(3) 
 
 
NAME 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_state 
 
SYNOPSIS 
-------------------------------------------------------------------------------------------- 
 
 
:: 
  
 #include <rmr/rmr.h>
 int rmr_wh_state( void* vctx, rmr_whid_t whid )
 
 
 
DESCRIPTION 
-------------------------------------------------------------------------------------------- 
 
The rmr_wh_state function will return the current state of 
the connection associated with the given wormhole (whid). The 
return value indicates whether the connection is open 
(RMR_OK), or closed (any other return value). 
 
When using some transport mechanisms (e.g. NNG), it may not 
be possible for RMR to know the actual state and the 
connection may always be reported as "open." 
 
RETURN 
-------------------------------------------------------------------------------------------- 
 
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
   
 
 
SEE ALSO 
-------------------------------------------------------------------------------------------- 
 
rmr_wh_open(3), rmr_wh_send_msg(3), rmr_wh_close(3) 

# ==================================================================================
#       Copyright (c) 2019 Nokia
#       Copyright (c) 2018-2019 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ==================================================================================
import uuid
import json
from ctypes import RTLD_GLOBAL, Structure, c_int, POINTER, c_char, c_char_p, c_void_p, memmove, cast
from ctypes import CDLL
from ctypes import create_string_buffer, pythonapi

# https://docs.python.org/3.7/library/ctypes.html
# https://stackoverflow.com/questions/2327344/ctypes-loading-a-c-shared-library-that-has-dependencies/30845750#30845750
# make sure you do a set -x LD_LIBRARY_PATH /usr/local/lib/;

# even though we don't use these directly, they contain symbols we need
_ = CDLL("libnng.so", mode=RTLD_GLOBAL)
rmr_c_lib = CDLL("librmr_nng.so", mode=RTLD_GLOBAL)


_rmr_const = rmr_c_lib.rmr_get_consts
_rmr_const.argtypes = []
_rmr_const.restype = c_char_p


def _get_constants(cache={}):
    """
    Get or build needed constants from rmr
    TODO: are there constants that end user applications need?
    """
    if cache:
        return cache

    js = _rmr_const()  # read json string
    cache = json.loads(str(js.decode()))  # create constants value object as a hash
    return cache


def _get_mapping_dict(cache={}):
    """
    Get or build the state mapping dict

    #define RMR_OK              0       // state is good
    #define RMR_ERR_BADARG      1       // argument passd to function was unusable
    #define RMR_ERR_NOENDPT     2       // send/call could not find an endpoint based on msg type
    #define RMR_ERR_EMPTY       3       // msg received had no payload; attempt to send an empty message
    #define RMR_ERR_NOHDR       4       // message didn't contain a valid header
    #define RMR_ERR_SENDFAILED  5       // send failed; errno has nano reason
    #define RMR_ERR_CALLFAILED  6       // unable to send call() message
    #define RMR_ERR_NOWHOPEN    7       // no wormholes are open
    #define RMR_ERR_WHID        8       // wormhole id was invalid
    #define RMR_ERR_OVERFLOW    9       // operation would have busted through a buffer/field size
    #define RMR_ERR_RETRY       10      // request (send/call/rts) failed, but caller should retry (EAGAIN for wrappers)
    #define RMR_ERR_RCVFAILED   11      // receive failed (hard error)
    #define RMR_ERR_TIMEOUT     12      // message processing call timed out
    #define RMR_ERR_UNSET       13      // the message hasn't been populated with a transport buffer
    #define RMR_ERR_TRUNC       14      // received message likely truncated
    #define RMR_ERR_INITFAILED  15      // initialisation of something (probably message) failed

    """
    if cache:
        return cache

    rmr_consts = _get_constants()
    for key in rmr_consts:  # build the state mapping dict
        if key[:7] in ["RMR_ERR", "RMR_OK"]:
            en = int(rmr_consts[key])
            cache[en] = key

    return cache


def _state_to_status(stateno):
    """
    convery a msg state to status
    """
    sdict = _get_mapping_dict()
    return sdict.get(stateno, "UNKNOWN STATE")


def _errno():
    """Suss out the C error number value which might be useful in understanding
    an underlying reason when RMr returns a failure.
    """
    return c_int.in_dll(pythonapi, "errno").value


##############
# PUBLIC API
##############

# These constants are directly usable by importers of this library
# TODO: Are there others that will be useful?
RMR_MAX_RCV_BYTES = _get_constants()["RMR_MAX_RCV_BYTES"]


class rmr_mbuf_t(Structure):
    """
    Reimplementation of rmr_mbuf_t which is in an unaccessible header file (src/common/include/rmr.h)

    typedef struct {
       int     state;             // state of processing
       int     mtype;             // message type
       int     len;               // length of data in the payload (send or received)
       unsigned char* payload;    // transported data
       unsigned char* xaction;    // pointer to fixed length transaction id bytes
       int sub_id;                // subscription id
                                  // these things are off limits to the user application
       void*   tp_buf;            // underlying transport allocated pointer (e.g. nng message)
       void*   header;            // internal message header (whole buffer: header+payload)
       unsigned char* id;         // if we need an ID in the message separate from the xaction id
       int flags;                 // various MFL_ (private) flags as needed
       int alloc_len;             // the length of the allocated space (hdr+payload)
    } rmr_mbuf_t;

    We do not include the fields we are not supposed to mess with

     RE PAYLOADs type below, see the documentation for c_char_p:
       class ctypes.c_char_p
           Represents the C char * datatype when it points to a zero-terminated string. For a general character pointer that may also point to binary data, POINTER(c_char) must be used. The constructor accepts an integer address, or a bytes object.

    """

    _fields_ = [
        ("state", c_int),
        ("mtype", c_int),
        ("len", c_int),
        (
            "payload",
            POINTER(c_char),
        ),  # according to th following the python bytes are already unsinged https://bytes.com/topic/python/answers/695078-ctypes-unsigned-char
        ("xaction", POINTER(c_char)),
        ("sub_id", c_int),
    ]


# argtypes and restype are important: https://stackoverflow.com/questions/24377845/ctype-why-specify-argtypes


# extern void* rmr_init(char* uproto_port, int max_msg_size, int flags)
rmr_init = rmr_c_lib.rmr_init
rmr_init.argtypes = [c_char_p, c_int, c_int]
rmr_init.restype = c_void_p


# extern void rmr_close(void* vctx)
rmr_close = rmr_c_lib.rmr_close
rmr_close.argtypes = [c_void_p]
# I don't think there's a restype needed here. THe return is simply "return" in the c lib

# extern int rmr_ready(void* vctx)
rmr_ready = rmr_c_lib.rmr_ready
rmr_ready.argtypes = [c_void_p]
rmr_ready.restype = c_int

# extern int rmr_set_stimeout(void* vctx, int time)
# RE "int time", from the C docs:
#        Set send timeout. The value time is assumed to be microseconds.  The timeout is the
#        rough maximum amount of time that RMr will block on a send attempt when the underlying
#        mechnism indicates eagain or etimeedout.  All other error conditions are reported
#        without this delay. Setting a timeout of 0 causes no retries to be attempted in
#        RMr code. Setting a timeout of 1 causes RMr to spin up to 10K retries before returning,
#        but without issuing a sleep.  If timeout is > 1, then RMr will issue a sleep (1us)
#        after every 10K send attempts until the time value is reached. Retries are abandoned
#        if NNG returns anything other than NNG_AGAIN or NNG_TIMEDOUT.
#
#        The default, if this function is not used, is 1; meaning that RMr will retry, but will
#        not enter a sleep.  In all cases the caller should check the status in the message returned
#        after a send call.
rmr_set_stimeout = rmr_c_lib.rmr_set_rtimeout
rmr_set_stimeout.argtypes = [c_void_p, c_int]
rmr_set_stimeout.restype = c_int

# extern rmr_mbuf_t* rmr_alloc_msg(void* vctx, int size)
rmr_alloc_msg = rmr_c_lib.rmr_alloc_msg
rmr_alloc_msg.argtypes = [c_void_p, c_int]
rmr_alloc_msg.restype = POINTER(rmr_mbuf_t)

# extern int rmr_payload_size(rmr_mbuf_t* msg)
rmr_payload_size = rmr_c_lib.rmr_payload_size
rmr_payload_size.argtypes = [POINTER(rmr_mbuf_t)]
rmr_payload_size.restype = c_int


"""
The following functions all seem to have the same interface
"""

# extern rmr_mbuf_t* rmr_send_msg(void* vctx, rmr_mbuf_t* msg)
rmr_send_msg = rmr_c_lib.rmr_send_msg
rmr_send_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
rmr_send_msg.restype = POINTER(rmr_mbuf_t)

# extern rmr_mbuf_t* rmr_rcv_msg(void* vctx, rmr_mbuf_t* old_msg)
# TODO: the old message (Send param) is actually optional, but I don't know how to specify that in Ctypes.
rmr_rcv_msg = rmr_c_lib.rmr_rcv_msg
rmr_rcv_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
rmr_rcv_msg.restype = POINTER(rmr_mbuf_t)

# extern rmr_mbuf_t* rmr_torcv_msg(void* vctx, rmr_mbuf_t* old_msg, int ms_to)
# the version of receive for nng that has a timeout (give up after X ms)
rmr_torcv_msg = rmr_c_lib.rmr_torcv_msg
rmr_torcv_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t), c_int]
rmr_torcv_msg.restype = POINTER(rmr_mbuf_t)

# extern rmr_mbuf_t*  rmr_rts_msg(void* vctx, rmr_mbuf_t* msg)
rmr_rts_msg = rmr_c_lib.rmr_rts_msg
rmr_rts_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
rmr_rts_msg.restype = POINTER(rmr_mbuf_t)

# extern rmr_mbuf_t* rmr_call(void* vctx, rmr_mbuf_t* msg)
rmr_call = rmr_c_lib.rmr_call
rmr_call.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
rmr_call.restype = POINTER(rmr_mbuf_t)


# Header field interface

# extern int rmr_bytes2meid(rmr_mbuf_t* mbuf, unsigned char const* src, int len);
rmr_bytes2meid = rmr_c_lib.rmr_bytes2meid
rmr_bytes2meid.argtypes = [POINTER(rmr_mbuf_t), c_char_p, c_int]
rmr_bytes2meid.restype = c_int


# CAUTION:  Some of the C functions expect a mutable buffer to copy the bytes into;
#           if there is a get_* function below, use it to set up and return the
#           buffer properly.

# extern unsigned char*  rmr_get_meid(rmr_mbuf_t* mbuf, unsigned char* dest);
rmr_get_meid = rmr_c_lib.rmr_get_meid
rmr_get_meid.argtypes = [POINTER(rmr_mbuf_t), c_char_p]
rmr_get_meid.restype = c_char_p

# extern unsigned char*  rmr_get_src(rmr_mbuf_t* mbuf, unsigned char* dest);
rmr_get_src = rmr_c_lib.rmr_get_src
rmr_get_src.argtypes = [POINTER(rmr_mbuf_t), c_char_p]
rmr_get_src.restype = c_char_p


# GET Methods


def get_payload(ptr_to_rmr_buf_t):
    """
    given a rmr_buf_t*, get it's binary payload as a bytes object
    this magical function came from the answer here: https://stackoverflow.com/questions/55103298/python-ctypes-read-pointerc-char-in-python
    """
    sz = ptr_to_rmr_buf_t.contents.len
    CharArr = c_char * sz
    return CharArr(*ptr_to_rmr_buf_t.contents.payload[:sz]).raw


def get_xaction(ptr_to_rmr_buf_t):
    """
    given a rmr_buf_t*, get it's transaction id
    """
    val = cast(ptr_to_rmr_buf_t.contents.xaction, c_char_p).value
    sz = _get_constants().get("RMR_MAX_XID", 0)
    return val[:sz]


def message_summary(ptr_to_rmr_buf_t):
    """
    Used for debugging mostly: returns a dict that contains the fields of a message
    """
    if ptr_to_rmr_buf_t.contents.len > RMR_MAX_RCV_BYTES:
        return "Malformed message: message length is greater than the maximum possible"

    meid = get_meid(ptr_to_rmr_buf_t)
    if meid == "\000" * _get_constants().get("RMR_MAX_MEID", 32):  # special case all nils
        meid = None

    return {
        "payload": get_payload(ptr_to_rmr_buf_t),
        "payload length": ptr_to_rmr_buf_t.contents.len,
        "message type": ptr_to_rmr_buf_t.contents.mtype,
        "subscription id": ptr_to_rmr_buf_t.contents.sub_id,
        "transaction id": get_xaction(ptr_to_rmr_buf_t),
        "message state": ptr_to_rmr_buf_t.contents.state,
        "message status": _state_to_status(ptr_to_rmr_buf_t.contents.state),
        "payload max size": rmr_payload_size(ptr_to_rmr_buf_t),
        "meid": meid,
        "message source": get_src(ptr_to_rmr_buf_t),
        "errno": _errno(),
    }


def set_payload_and_length(byte_str, ptr_to_rmr_buf_t):
    """
    use memmove to set the rmr_buf_t payload and content length
    """
    memmove(ptr_to_rmr_buf_t.contents.payload, byte_str, len(byte_str))
    ptr_to_rmr_buf_t.contents.len = len(byte_str)


def generate_and_set_transaction_id(ptr_to_rmr_buf_t):
    """
    use memmove to set the rmr_buf_t xaction
    """
    uu_id = uuid.uuid1().hex.encode("utf-8")
    sz = _get_constants().get("RMR_MAX_XID", 0)
    memmove(ptr_to_rmr_buf_t.contents.xaction, uu_id, sz)


def get_meid(mbuf):
    """
    Suss out the managed equipment ID (meid) from the message header.
    This is a 32 byte field and RMr returns all 32 bytes which if the
    sender did not set will be garbage.
    """
    sz = _get_constants().get("RMR_MAX_MEID", 64)  # size for buffer to fill
    buf = create_string_buffer(sz)
    rmr_get_meid(mbuf, buf)
    return buf.raw.decode()


def get_src(mbuf):
    """
    Suss out the message source information (likely host:port).
    """
    sz = _get_constants().get("RMR_MAX_SRC", 64)  # size to fill
    buf = create_string_buffer(sz)
    rmr_get_src(mbuf, buf)
    return buf.value.decode()

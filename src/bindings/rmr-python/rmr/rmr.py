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
from ctypes import create_string_buffer

# https://docs.python.org/3.7/library/ctypes.html
# https://stackoverflow.com/questions/2327344/ctypes-loading-a-c-shared-library-that-has-dependencies/30845750#30845750
# make sure you do a set -x LD_LIBRARY_PATH /usr/local/lib/;

# even though we don't use these directly, they contain symbols we need
_ = CDLL("libnng.so", mode=RTLD_GLOBAL)
rmr_c_lib = CDLL("librmr_nng.so", mode=RTLD_GLOBAL)


# Internal Helpers (not a part of public api)


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

    RMR_OK              0   state is good
    RMR_ERR_BADARG      1   argument passd to function was unusable
    RMR_ERR_NOENDPT     2   send/call could not find an endpoint based on msg type
    RMR_ERR_EMPTY       3   msg received had no payload; attempt to send an empty message
    RMR_ERR_NOHDR       4   message didn't contain a valid header
    RMR_ERR_SENDFAILED  5   send failed; errno has nano reason
    RMR_ERR_CALLFAILED  6   unable to send call() message
    RMR_ERR_NOWHOPEN    7   no wormholes are open
    RMR_ERR_WHID        8   wormhole id was invalid
    RMR_ERR_OVERFLOW    9   operation would have busted through a buffer/field size
    RMR_ERR_RETRY       10  request (send/call/rts) failed, but caller should retry (EAGAIN for wrappers)
    RMR_ERR_RCVFAILED   11  receive failed (hard error)
    RMR_ERR_TIMEOUT     12  message processing call timed out
    RMR_ERR_UNSET       13  the message hasn't been populated with a transport buffer
    RMR_ERR_TRUNC       14  received message likely truncated
    RMR_ERR_INITFAILED  15  initialisation of something (probably message) failed

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
    Convert a msg state to status

    """
    sdict = _get_mapping_dict()
    return sdict.get(stateno, "UNKNOWN STATE")


##############
# PUBLIC API
##############


# These constants are directly usable by importers of this library
# TODO: Are there others that will be useful?
RMR_MAX_RCV_BYTES = _get_constants()["RMR_MAX_RCV_BYTES"]


class rmr_mbuf_t(Structure):
    """
    Reimplementation of rmr_mbuf_t which is in an unaccessible header file (src/common/include/rmr.h)

    | typedef struct {
    |    int     state;          // state of processing
    |    int     mtype;          // message type
    |    int     len;            // length of data in the payload (send or received)
    |    unsigned char* payload; // transported data
    |    unsigned char* xaction; // pointer to fixed length transaction id bytes
    |    int sub_id;             // subscription id
    |    int      tp_state;      // transport state (a.k.a errno)
    |
    | these things are off limits to the user application
    |
    |    void*   tp_buf;         // underlying transport allocated pointer (e.g. nng message)
    |    void*   header;         // internal message header (whole buffer: header+payload)
    |    unsigned char* id;      // if we need an ID in the message separate from the xaction id
    |    int flags;              // various MFL (private) flags as needed
    |    int alloc_len;          // the length of the allocated space (hdr+payload)
    | } rmr_mbuf_t;

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
        ("tp_state", c_int),
    ]


# argtypes and restype are important: https://stackoverflow.com/questions/24377845/ctype-why-specify-argtypes


_rmr_init = rmr_c_lib.rmr_init
_rmr_init.argtypes = [c_char_p, c_int, c_int]
_rmr_init.restype = c_void_p


def rmr_init(uproto_port, max_msg_size, flags):
    """
    Refer to rmr C documentation for rmr_init
    extern void* rmr_init(char* uproto_port, int max_msg_size, int flags)
    """
    return _rmr_init(uproto_port, max_msg_size, flags)


_rmr_ready = rmr_c_lib.rmr_ready
_rmr_ready.argtypes = [c_void_p]
_rmr_ready.restype = c_int


def rmr_ready(vctx):
    """
    Refer to rmr C documentation for rmr_ready
    extern int rmr_ready(void* vctx)
    """
    return _rmr_ready(vctx)


_rmr_close = rmr_c_lib.rmr_close
_rmr_close.argtypes = [c_void_p]


def rmr_close(vctx):
    """
    Refer to rmr C documentation for rmr_close
    extern void rmr_close(void* vctx)
    """
    return _rmr_close(vctx)


_rmr_set_stimeout = rmr_c_lib.rmr_set_stimeout
_rmr_set_stimeout.argtypes = [c_void_p, c_int]
_rmr_set_stimeout.restype = c_int


def rmr_set_stimeout(vctx, time):
    """
    Refer to the rmr C documentation for rmr_set_stimeout
    extern int rmr_set_stimeout(void* vctx, int time)
    """
    return _rmr_set_stimeout(vctx, time)


_rmr_alloc_msg = rmr_c_lib.rmr_alloc_msg
_rmr_alloc_msg.argtypes = [c_void_p, c_int]
_rmr_alloc_msg.restype = POINTER(rmr_mbuf_t)


def rmr_alloc_msg(vctx, size):
    """
    Refer to the rmr C documentation for rmr_alloc_msg
    extern rmr_mbuf_t* rmr_alloc_msg(void* vctx, int size)
    """
    return _rmr_alloc_msg(vctx, size)


_rmr_payload_size = rmr_c_lib.rmr_payload_size
_rmr_payload_size.argtypes = [POINTER(rmr_mbuf_t)]
_rmr_payload_size.restype = c_int


def rmr_payload_size(ptr_mbuf):
    """
    Refer to the rmr C documentation for rmr_payload_size
    extern int rmr_payload_size(rmr_mbuf_t* msg)
    """
    return _rmr_payload_size(ptr_mbuf)


"""
The following functions all seem to have the same interface
"""

_rmr_send_msg = rmr_c_lib.rmr_send_msg
_rmr_send_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
_rmr_send_msg.restype = POINTER(rmr_mbuf_t)


def rmr_send_msg(vctx, ptr_mbuf):
    """
    Refer to the rmr C documentation for rmr_send_msg
    extern rmr_mbuf_t* rmr_send_msg(void* vctx, rmr_mbuf_t* msg)
    """
    return _rmr_send_msg(vctx, ptr_mbuf)


# TODO: the old message (Send param) is actually optional, but I don't know how to specify that in Ctypes.
_rmr_rcv_msg = rmr_c_lib.rmr_rcv_msg
_rmr_rcv_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
_rmr_rcv_msg.restype = POINTER(rmr_mbuf_t)


def rmr_rcv_msg(vctx, ptr_mbuf):
    """
    Refer to the rmr C documentation for rmr_rcv_msg
    extern rmr_mbuf_t* rmr_rcv_msg(void* vctx, rmr_mbuf_t* old_msg)
    """
    return _rmr_rcv_msg(vctx, ptr_mbuf)


_rmr_torcv_msg = rmr_c_lib.rmr_torcv_msg
_rmr_torcv_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t), c_int]
_rmr_torcv_msg.restype = POINTER(rmr_mbuf_t)


def rmr_torcv_msg(vctx, ptr_mbuf, ms_to):
    """
    Refer to the rmr C documentation for rmr_torcv_msg
    extern rmr_mbuf_t* rmr_torcv_msg(void* vctx, rmr_mbuf_t* old_msg, int ms_to)
    """
    return _rmr_torcv_msg(vctx, ptr_mbuf, ms_to)


_rmr_rts_msg = rmr_c_lib.rmr_rts_msg
_rmr_rts_msg.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
_rmr_rts_msg.restype = POINTER(rmr_mbuf_t)


def rmr_rts_msg(vctx, ptr_mbuf):
    """
    Refer to the rmr C documentation for rmr_rts_msg
    extern rmr_mbuf_t*  rmr_rts_msg(void* vctx, rmr_mbuf_t* msg)
    """
    return _rmr_rts_msg(vctx, ptr_mbuf)


_rmr_call = rmr_c_lib.rmr_call
_rmr_call.argtypes = [c_void_p, POINTER(rmr_mbuf_t)]
_rmr_call.restype = POINTER(rmr_mbuf_t)


def rmr_call(vctx, ptr_mbuf):
    """
    Refer to the rmr C documentation for rmr_call
    extern rmr_mbuf_t* rmr_call(void* vctx, rmr_mbuf_t* msg)
    """
    return _rmr_call(vctx, ptr_mbuf)


_rmr_bytes2meid = rmr_c_lib.rmr_bytes2meid
_rmr_bytes2meid.argtypes = [POINTER(rmr_mbuf_t), c_char_p, c_int]
_rmr_bytes2meid.restype = c_int


def rmr_bytes2meid(ptr_mbuf, src, length):
    """
    Refer to the rmr C documentation for rmr_bytes2meid
    extern int rmr_bytes2meid(rmr_mbuf_t* mbuf, unsigned char const* src, int len);
    """
    return _rmr_bytes2meid(ptr_mbuf, src, length)


# CAUTION:  Some of the C functions expect a mutable buffer to copy the bytes into;
#           if there is a get_* function below, use it to set up and return the
#           buffer properly.


_rmr_get_meid = rmr_c_lib.rmr_get_meid
_rmr_get_meid.argtypes = [POINTER(rmr_mbuf_t), c_char_p]
_rmr_get_meid.restype = c_char_p


def rmr_get_meid(ptr_mbuf, dest):
    """
    Refer to the rmr C documentation for rmr_get_meid
    extern unsigned char*  rmr_get_meid(rmr_mbuf_t* mbuf, unsigned char* dest);
    """
    return _rmr_get_meid(ptr_mbuf, dest)


_rmr_get_src = rmr_c_lib.rmr_get_src
_rmr_get_src.argtypes = [POINTER(rmr_mbuf_t), c_char_p]
_rmr_get_src.restype = c_char_p


def rmr_get_src(ptr_mbuf, dest):
    """
    Refer to the rmr C documentation for rmr_get_src
    extern unsigned char*  rmr_get_src(rmr_mbuf_t* mbuf, unsigned char* dest);
    """
    return _rmr_get_src(ptr_mbuf, dest)


# Methods that exist ONLY in rmr-python, and are not wrapped methods
# In hindsight, I wish i put these in a seperate module, but leaving this here to prevent api breakage.


def get_payload(ptr_mbuf):
    """
    Given a rmr_buf_t*, get it's binary payload as a bytes object

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer

    Returns
    -------
    bytes:
        the message payload
    """
    # Logic came from the answer here: https://stackoverflow.com/questions/55103298/python-ctypes-read-pointerc-char-in-python
    sz = ptr_mbuf.contents.len
    CharArr = c_char * sz
    return CharArr(*ptr_mbuf.contents.payload[:sz]).raw


def get_xaction(ptr_mbuf):
    """
    given a rmr_buf_t*, get it's transaction id

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer

    Returns
    -------
    bytes:
        the transaction id
    """
    val = cast(ptr_mbuf.contents.xaction, c_char_p).value
    sz = _get_constants().get("RMR_MAX_XID", 0)
    return val[:sz]


def message_summary(ptr_mbuf):
    """
    Returns a dict that contains the fields of a message

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer

    Returns
    -------
    dict:
        dict message summary
    """
    if ptr_mbuf.contents.len > RMR_MAX_RCV_BYTES:
        return "Malformed message: message length is greater than the maximum possible"

    meid = get_meid(ptr_mbuf)
    if meid == "\000" * _get_constants().get("RMR_MAX_MEID", 32):  # special case all nils
        meid = None

    return {
        "payload": get_payload(ptr_mbuf),
        "payload length": ptr_mbuf.contents.len,
        "message type": ptr_mbuf.contents.mtype,
        "subscription id": ptr_mbuf.contents.sub_id,
        "transaction id": get_xaction(ptr_mbuf),
        "message state": ptr_mbuf.contents.state,
        "message status": _state_to_status(ptr_mbuf.contents.state),
        "payload max size": rmr_payload_size(ptr_mbuf),
        "meid": meid,
        "message source": get_src(ptr_mbuf),
        "errno": ptr_mbuf.contents.tp_state,
    }


def set_payload_and_length(byte_str, ptr_mbuf):
    """
    | Set an rmr payload and content length
    | In place method, no return

    Parameters
    ----------
    byte_str: bytes
        the bytes to set the payload to
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer
    """
    memmove(ptr_mbuf.contents.payload, byte_str, len(byte_str))
    ptr_mbuf.contents.len = len(byte_str)


def generate_and_set_transaction_id(ptr_mbuf):
    """
    | Generate a UUID and Set an rmr transaction id to it
    | In place method, no return

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer
    """
    uu_id = uuid.uuid1().hex.encode("utf-8")
    sz = _get_constants().get("RMR_MAX_XID", 0)
    memmove(ptr_mbuf.contents.xaction, uu_id, sz)


def get_meid(ptr_mbuf):
    """
    | Get the managed equipment ID (meid) from the message header.
    | This is a 32 byte field and RMr returns all 32 bytes which if the sender did not set will be garbage.

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer

    Returns
    -------
    string:
        meid
    """
    sz = _get_constants().get("RMR_MAX_MEID", 64)  # size for buffer to fill
    buf = create_string_buffer(sz)
    rmr_get_meid(ptr_mbuf, buf)
    return buf.raw.decode()


def get_src(ptr_mbuf):
    """
    Get the message source (likely host:port)

    Parameters
    ----------
    ptr_mbuf: ctypes c_void_p
        Pointer to an rmr message buffer

    Returns
    -------
    string:
        meid
    """
    sz = _get_constants().get("RMR_MAX_SRC", 64)  # size to fill
    buf = create_string_buffer(sz)
    rmr_get_src(ptr_mbuf, buf)
    return buf.value.decode()

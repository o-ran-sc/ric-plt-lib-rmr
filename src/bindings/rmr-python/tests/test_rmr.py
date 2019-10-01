# vim: ts=4 sw=4 expandtab:
# =================================================================================2
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
import time
import pytest
import os
from rmr import rmr


SIZE = 256
MRC_SEND = None
MRC_RCV = None


def setup_module():
    """
    test_rmr module setup
    """
    os.environ["RMR_ASYNC_CONN"] = "0"      # force connections in NNG to be synchrnous
    global MRC_SEND
    MRC_SEND = rmr.rmr_init(b"4562", rmr.RMR_MAX_RCV_BYTES, 0x00)
    while rmr.rmr_ready(MRC_SEND) == 0:
        time.sleep(1)

    global MRC_RCV
    MRC_RCV = rmr.rmr_init(b"4563", rmr.RMR_MAX_RCV_BYTES, 0x00)
    while rmr.rmr_ready(MRC_RCV) == 0:
        time.sleep(1)


def teardown_module():
    """
    test rmr module teardown
    """
    rmr.rmr_close(MRC_SEND)
    rmr.rmr_close(MRC_RCV)


def _assert_new_sbuf(sbuf):
    """
    verify the initial state of an alloced message is what we expect
    """
    summary = rmr.message_summary(sbuf)
    assert summary["payload"] == b""
    assert summary["payload length"] == 0
    assert summary["transaction id"] == b""
    assert summary["message state"] == 0
    assert summary["message status"] == "RMR_OK"
    assert summary["meid"] == ""
    assert summary["errno"] == 0


def test_get_constants(expected_constants):
    """
    test getting constants. We don't care what values are returned as those
    should be meaningful only to RMR. We do care that all of the constants
    which are defined in expected_contents are returned.  Further, we don't
    consider it to be an error if the returned list has more constants than
    what are in our list.

    To avoid frustration, this should list all missing keys, not fail on the
    first missing key.
    """
    errors = 0
    econst = expected_constants
    rconst = rmr._get_constants()
    for key in econst:                   # test all expected constants
        if key not in rconst:            # expected value not listed by rmr
            errors += 1
            print( "did not find required constant in list from RMR: %s" % key )

    assert errors == 0


def test_get_mapping_dict(expected_states):
    """
    test getting mapping string
    """
    assert rmr._get_mapping_dict() == expected_states
    assert rmr._state_to_status(0) == "RMR_OK"
    assert rmr._state_to_status(12) == "RMR_ERR_TIMEOUT"
    assert rmr._state_to_status(666) == "UNKNOWN STATE"


def test_meid():
    """
    test meid stringification
    """
    sbuf = rmr.rmr_alloc_msg(MRC_SEND, SIZE)

    rmr.rmr_set_meid(sbuf, b"\x01\x02", 2)
    assert rmr.rmr_get_meid(sbuf) == rmr.message_summary(sbuf)["meid"] == "\x01\x02"
    assert len(rmr.rmr_get_meid(sbuf)) == 2

    rmr.rmr_set_meid(sbuf, b"\x00" * 32, 32)
    assert rmr.rmr_get_meid(sbuf) == rmr.message_summary(sbuf)["meid"] == ""  # NULL bytes get truncated

    rmr.rmr_set_meid(sbuf, b"6" * 32, 32)
    assert rmr.rmr_get_meid(sbuf) == rmr.message_summary(sbuf)["meid"] == "6" * 32  # string in string out

    rmr.rmr_set_meid(sbuf, b"\x01\x02", 2)
    assert (
        rmr.rmr_get_meid(sbuf) == rmr.message_summary(sbuf)["meid"] == "\x01\x02" + "6" * 30
    )  # bytes in string out, 6s left over
    assert len(rmr.rmr_get_meid(sbuf)) == 32


def test_rmr_set_get():
    """
    test set functions
    """
    sbuf = rmr.rmr_alloc_msg(MRC_SEND, SIZE)
    _assert_new_sbuf(sbuf)

    # test payload
    pay = b"\x01\x00\x80"
    rmr.set_payload_and_length(pay, sbuf)
    summary = rmr.message_summary(sbuf)
    assert summary["payload"] == pay
    assert summary["payload length"] == 3

    # test transid (note we cant test payload because it's randomly gen)
    assert summary["transaction id"] == b""
    assert len(summary["transaction id"]) == 0
    rmr.generate_and_set_transaction_id(sbuf)
    summary = rmr.message_summary(sbuf)
    assert summary["transaction id"] != b""
    assert len(summary["transaction id"]) == 32

    # test meid
    assert rmr.rmr_get_meid(sbuf) == summary["meid"] == ""
    rmr.rmr_set_meid(sbuf, b"666\x01\x00\x01", 6)
    summary = rmr.message_summary(sbuf)
    assert rmr.rmr_get_meid(sbuf) == summary["meid"] == "666\x01"
    assert (len(summary["meid"])) == 4


def test_rcv_timeout():
    """
    test torcv; this is a scary test because if it fails... it doesn't fail, it will run forever!
    We receive a message (though nothing has been sent) and make sure the function doesn't block forever.

    There is no unit test for rmr_rcv_msg; too dangerous, that is a blocking call that may never return.
    """
    sbuf_rcv = rmr.rmr_alloc_msg(MRC_RCV, SIZE)
    sbuf_rcv = rmr.rmr_torcv_msg(MRC_RCV, sbuf_rcv, 50)  # should time out after 50ms
    summary = rmr.message_summary(sbuf_rcv)
    assert summary["message state"] == 12
    assert summary["message status"] == "RMR_ERR_TIMEOUT"


def test_send_rcv():
    """
    test send and receive
    """
    pay = b"\x01\x00\x80"

    # send a message
    sbuf_send = rmr.rmr_alloc_msg(MRC_SEND, SIZE)
    _assert_new_sbuf(sbuf_send)
    rmr.set_payload_and_length(pay, sbuf_send)
    sbuf_send.contents.mtype = 0
    sbuf_send = rmr.rmr_send_msg(MRC_SEND, sbuf_send)
    send_summary = rmr.message_summary(sbuf_send)

    # receive it in other context
    sbuf_rcv = rmr.rmr_alloc_msg(MRC_RCV, SIZE)
    sbuf_rcv = rmr.rmr_torcv_msg(MRC_RCV, sbuf_rcv, 2000)
    rcv_summary = rmr.message_summary(sbuf_rcv)
    assert rcv_summary["payload"] == pay
    assert rcv_summary["message type"] == 0
    assert send_summary["message state"] == rcv_summary["message state"] == 0
    assert send_summary["message status"] == rcv_summary["message status"] == "RMR_OK"

    # send an ACK back
    ack_pay = b"message received"
    rmr.set_payload_and_length(ack_pay, sbuf_rcv)
    sbuf_rcv = rmr.rmr_rts_msg(MRC_RCV, sbuf_rcv)
    rcv_ack_summary = rmr.message_summary(sbuf_rcv)

    # have the sender receive it
    sbuf_send = rmr.rmr_torcv_msg(MRC_SEND, sbuf_send, 2000)
    send_ack_summary = rmr.message_summary(sbuf_send)

    assert send_ack_summary["payload"] == ack_pay
    assert send_ack_summary["message state"] == rcv_ack_summary["message state"] == 0
    assert send_ack_summary["message status"] == rcv_ack_summary["message status"] == "RMR_OK"

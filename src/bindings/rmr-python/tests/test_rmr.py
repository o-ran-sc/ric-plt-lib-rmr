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
import time
import pytest
from rmr import rmr


SIZE = 256
MRC = None


def setup_module():
    global MRC
    MRC = rmr.rmr_init(b"4562", rmr.RMR_MAX_RCV_BYTES, 0x00)
    while rmr.rmr_ready(MRC) == 0:
        time.sleep(1)


def teardown_module():
    rmr.rmr_close(MRC)


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
    assert summary["meid"] is None
    assert summary["errno"] == 0


def test_get_constants(expected_constants):
    """
    test getting constants
    """
    assert rmr._get_constants() == expected_constants


def test_get_mapping_dict(expected_states):
    """
    test getting mapping string
    """
    assert rmr._get_mapping_dict() == expected_states
    assert rmr._state_to_status(0) == "RMR_OK"
    assert rmr._state_to_status(12) == "RMR_ERR_TIMEOUT"
    assert rmr._state_to_status(666) == "UNKNOWN STATE"


def test_meid_prettify():
    """
    test the printing of meid based on it's value
    """
    # TODO?? weirdness: setting it takes bytes, but getting it returns a string. This does NOT happen for payload; bytes in, bytes come out.
    sbuf = rmr.rmr_alloc_msg(MRC, SIZE)
    rmr.rmr_set_meid(sbuf, b"\x00" * 32, 32)
    summary = rmr.message_summary(sbuf)
    assert summary["meid"] is None  # summary does a pretty print" of 32 null bytes
    assert rmr.get_meid(sbuf) == "\x00" * 32  # real underlying value


def test_rmr_set_get():
    """
    test set functions
    """
    sbuf = rmr.rmr_alloc_msg(MRC, SIZE)
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
    assert rmr.get_meid(sbuf) == "\x00" * 32
    assert summary["meid"] is None  # the summary printing function shows the above horridness as None
    rmr.rmr_set_meid(sbuf, b"666", 3)
    summary = rmr.message_summary(sbuf)
    # TODO?? weirdness: setting it takes bytes, but getting it returns a string. This does NOT happen for payload; bytes in, bytes come out.
    assert rmr.get_meid(sbuf) == summary["meid"] == "666" + "\x00" * 29

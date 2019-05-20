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
import pytest
import json
from rmr import rmr
from rmr.rmr_mocks import rmr_mocks


MRC = None
SIZE = 256


def test_get_mapping_dict(monkeypatch, fake_consts, expected_states):
    """
    test getting mapping string
    """

    def fake_rmr_get_consts():
        return json.dumps(fake_consts).encode("utf-8")

    monkeypatch.setattr("rmr.rmr._rmr_const", fake_rmr_get_consts)
    assert rmr._get_mapping_dict() == expected_states
    # do again, trigger cache line coverage
    assert rmr._get_mapping_dict() == expected_states

    assert rmr._state_to_status(0) == "RMR_OK"
    assert rmr._state_to_status(12) == "RMR_ERR_TIMEOUT"
    assert rmr._state_to_status(666) == "UNKNOWN STATE"


def test_meid_prettify(monkeypatch):
    rmr_mocks.patch_rmr(monkeypatch)

    # here we re-monkey get_meid
    monkeypatch.setattr("rmr.rmr.get_meid", lambda _: "yoooo")
    sbuf = rmr.rmr_alloc_msg(MRC, SIZE)
    summary = rmr.message_summary(sbuf)
    assert summary["meid"] == "yoooo"

    # test bytes
    monkeypatch.setattr("rmr.rmr.get_meid", lambda _: b"\x01\x00f\x80")
    sbuf = rmr.rmr_alloc_msg(MRC, SIZE)
    summary = rmr.message_summary(sbuf)
    assert summary["meid"] == b"\x01\x00f\x80"

    # test the cleanup of null bytes
    monkeypatch.setattr(
        "rmr.rmr.get_meid",
        lambda _: "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
    )
    sbuf = rmr.rmr_alloc_msg(MRC, SIZE)
    summary = rmr.message_summary(sbuf)
    assert summary["meid"] == None

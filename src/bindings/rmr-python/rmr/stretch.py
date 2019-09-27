# vim: ts=4 sw=4 expandtab:
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

#   Mnemonic:   convenience.py
#   Abstract:   This is a colleciton of extensions to the RMR base package
#               which are likely to be convenient for python programmes.
#   Date:       26 September 2019
# ---------------------------------------------------------------------------

from rmr import rmr

def rmr_rcvall_msgs( mrc, pass_filter=None ):
    """
        Assemble an array of all messages which can be received without
        blocking.  Effectively draining the message queue if RMR is started
        in mt-call mode, or draining any waiting TCP buffers.  If the
        pass_filter parameter is supplied it is treated as one or more message
        types to accept (pass through). The parm may be a single integer, or
        an array. Any message received that does not match a specified type
        is silently dropped.  If no pass filter is supplied, all messages pass.

        Parameters
        ----------
            mrc: ctypes c_void_p
                Pointer to the RMR context

            pass_filter: list | integer
                The message type(s) to capture; if more than one message type
                is to be captured their values must be supplied as an array.
    
        Returns
        -------
            list 
            List of message summaries, one for each message captured.
    """
    sz = _get_constants().get("RMR_MAX_MEID", 64)  # size for buffer to fill
    buf = create_string_buffer(sz)

    if pass_filter != None:
        if type( pass_filter ) is not list:
            pass_filter = [ pass_filter ]

    new_messages = []
    mbuf = rmr.rmr_alloc_msg( mrc, 4096)         # allocate buffer to have something for a return status

    while True:
        mbuf = rmr.rmr_torcv_msg( mrc, mbuf, 0)  # set the timeout to 0 so this doesn't block!!

        summary = rmr.message_summary(mbuf)
        if summary["message status"] != "RMR_OK":      # ok indicates msg received, stop on all other states
            break
        else:
            if (not pass_filter) or (summary["message type"] in pass_filter):      # no filter, or passes; capture it
                new_messages.append( summary )


    rmr.rmr_free_msg( mbuf )            # must free message to avoid leak
    return new_messages


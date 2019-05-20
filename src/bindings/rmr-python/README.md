# rmr-python

# Summary, Limitations
This is a CTYPES wrapper around the C rmr library. It requires you have rmr installed.

That is, it is not a native re-implementation of the rmr library. This seems to come with pros and cons. On the positive side, wrapping the library was much less work; we only need to wrap the function signatures.
Keeping up with the rmr spec is thus also less work, as when new functions are added into the C lib, we only need to again wrap the function signatures.

The downside is this seems to be Linux only currently. This wrapper immediately SIGABRT's on Mac, and no one yet seems to know why.
The other downside is that there are currently some functionality that needs to be "exported" from the C library for this to be fully operational. For example, CTYPES does not have access to C header files, and important
constants are defined in the C header files. Also, the C lib uses "errno" to propogate some error conditions, and those are not available "in-band" yet.

It could be questioned whether this was a good decision, or whether we should have natively reimplemented the API with the nano nng python bindings: https://pypi.org/project/pynng/

## Not Yet Implemented
At the time of this writing (March 28 2019) The following C functions are not yet implemented in this library (do we need them?):

    1. `extern void rmr_free_msg`
    2. `extern rmr_mbuf_t* rmr_mtosend_msg`
    3. `extern rmr_mbuf_t* rmr_call` (this has some problems AFAIU from Scott)
    4. `extern rmr_mbuf_t* rmr_rcv_specific`
    5. `extern int rmr_get_rcvfd`

# Higher order library

There is/was somewhat of a debate about what belongs here, and the current answer is that this is mostly a pure wrapper around the C rmr library (though it does come with one convenience function called `message_summary` which is quite useful)

There are some higher order send functions, for example functions that send and expect an ACK back of a specific message type, that might be useful to you, here: https://gitlab.research.att.com/tommy/ric-ons-a1-gevent/blob/master/a1/a1rmr.py

# Unit Testing

    tox
    open htmlcov/index.html

# Installation

## Prequisites

If rmr is *not* compiled on your system, see the below instructions for downloading and compiling rmr. This library expects that the rmr .so files are compiled and available.

## From PyPi

    pip install rmr==X.Y.Z

## From Source

    git clone "https://gerrit.o-ran-sc.org/r/ric-plt/lib/rmr"
    cd rmr/src/bindings/rmr-python/
    pip install .

# Examples

See the `examples` directory.

# Compiling rmr (if not already done on your system)
(Note, you may or may not need sudo in your final command, depending on permissions to `/usr/local`. I need it)

    git clone https://gerrit.oran-osc.org/r/ric-plt/lib/rmr
    cd rmr
    mkdir .build; cd .build; cmake ..; sudo make install

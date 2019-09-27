rmr-python
==========

Summary, Limitations
====================

This is a CTYPES wrapper around the C rmr library. It requires you have
rmr installed.

That is, it is not a native re-implementation of the rmr library. This
seems to come with pros and cons. On the positive side, wrapping the
library was much less work; we only need to wrap the function
signatures. Keeping up with the rmr spec is thus also less work, as when
new functions are added into the C lib, we only need to again wrap the
function signatures.

The downside is this seems to be Linux only currently. This wrapper
immediately SIGABRT's on Mac, and no one yet seems to know why. The
other downside is that there are currently some functionality that needs
to be 'exported' from the C library for this to be fully operational.
For example, CTYPES does not have access to C header files, and
important constants are defined in the C header files.

Possibly evaluate whether we could natively reimplement the API with the nano nng python
bindings: https://pypi.org/project/pynng/

Not Yet Implemented
-------------------

At the time of this writing (Aug 13 2019) The following C functions
are not yet implemented in this library (do we need them?):

::

   1. `extern void rmr_free_msg`
   2. `extern rmr_mbuf_t* rmr_mtosend_msg`
   3. `extern rmr_mbuf_t* rmr_call` (this has some problems AFAIU from Scott)
   4. `extern rmr_mbuf_t* rmr_rcv_specific`
   5. `extern int rmr_get_rcvfd`

Unit Testing
============
You can unit test in docker or outside of docker.
The preferred method (by far) is to use Docker, because it encapsulates rmr, as well as ensuring that ports that rmr opens do not conflict with the host machine

::

    docker build -t rmrunittestt:latest -f Dockerfile-Unit-Test   .

A coverage report will be shown in stdout.

It is possible to run tox locally provided that rmr is intalled, and you are prepared to have the test open ports (4562) as ``rmr_init`` must succeed for the tests to proceed.

::

   tox
   open htmlcov/index.html

The added benefit of the local option is that the coverage report can be viewed in html, which I find easier to read than the term coverage reort the docker build will print.

Installation
============

Prequisites
-----------

If rmr is *not* compiled on your system, see the below instructions for
downloading and compiling rmr. This library expects that the rmr .so
files are compiled and available.

From PyPi
---------

::

   pip install rmr==X.Y.Z

From Source
-----------

::

   git clone "https://gerrit.o-ran-sc.org/r/ric-plt/lib/rmr"
   cd rmr/src/bindings/rmr-python/
   pip install .

Examples
========

See the ``examples`` directory.

Compiling rmr (if not already done on your system)
==================================================

(Note, you may or may not need sudo in your final command, depending on
permissions to ``/usr/local``. The pack externals option to CMake is
needed only if the NNG libary is not already installed on the system,
and you do not wish to manually install it.)

::

   git clone https://gerrit.oran-osc.org/r/ric-plt/lib/rmr
   cd rmr
   mkdir .build; cd .build; cmake .. -DPACK_EXTERNALS=1; sudo make install

Change Log
==========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <http://keepachangelog.com/>`__
and this project adheres to `Semantic
Versioning <http://semver.org/>`__.

[1.0.1] - 10/30/2019
--------------------

::
    * The populate and set length function will reallocate the message buffer payload if the caller attempts to insert more bytes than the currrent message supports.


[1.0.0] - 10/24/2019
--------------------

::

    * It's been past due to bump this to 1.0.0 since people depend on it!
    * Add the ability to set sbuf attributes in the same call that it is allocated
    * (breaking) removes bytes2meid
    * (breaking) rmr_set_meid now infers length
    * (breaking) rmr_get_meid now returns bytes, to be symmetric with set_meid


[0.13.5] - 10/23/2019
--------------------

::

    * Add an exceptions module and raise a proper exception when an allocated buffer has a NULL pointer. Likely due to a bad rmr context.

[0.13.4] - 10/21/2019
--------------------

::

    * Correct cause of nil pointer exception in message summary.


[0.13.3] - 10/10/2019
--------------------

::

    * Add missing unit test for receive all.
    * Correct bug in summary function.

[0.13.2] - 10/2/2019
--------------------

::

    * Fix a constant name (RMRFL_MT_CALL)

[0.13.1] - 10/1/2019
--------------------

::

    * Correct unit test bug in rmr-python. With RMR 1.8.x connections are forced to be asynch by default to prevent kubernetes blocking the attempt for minutes. However, the asynch nature of connections makes unit tests concerned with the ability to send and receive messages non-deterministic as some connections are established before the first message is sent, and others are not. This change ensures that unit tests establish connections in a synchronous manner which ensures that the first send will not be rejected by NNG due to a pending connection.

[0.13.0] - 9/27/2019
--------------------

::
   * Add a helpers module to provide extensions and helper functions such as receive all queued messages.
   * Enhance unit test to check only for RMR constants which are needed.
   * Correct unprintable characters in documentation.


[0.12.0] - 8/23/2019
--------------------

::
   * Add final unit tests for rmr.py; unit test coverage for rmr python is about 95%. The remaining functions are dangerous to unit test directly, e.g., rcv which may block forever
   * Fix a bug where meid was being intepreted as bytes (but then cast into a string); the correct interpretation is a string, so now it will truncate after a null byte.
   * Removes access to the raw function rmr_get_meid(ptr, dest) in favor of just rmr_get_meid(ptr). Also get_meid is now rmr_get_meid since it wasn't consistent with the naming.


[0.11.0] - 8/21/2019
--------------------

::
   * Overhaul unit tests to remove mocking from the rmr tests, which gives much greater confidence in changing the code. More is still needed however, specifically test sends and test receives.
   * Adds an alias rmr_set_meid to rmr_bytes2meid for naming consistency.
   * Found a possible inconsistency/bug that requires further investigation later; setting meid takes bytes, but getting it returns a string.


[0.10.8] - 8/20/2019
--------------------

::
   * Fix invocation of _rmr_alloc function


[0.10.7] - 8/14/2019
--------------------

::
   * Finish sphinx documentation
   * Make public functions that wrap ctype declarions, allowing for docstrings
   * Fix a bug where rmr_set_stimeout was pointing to the wrong function


[0.10.6] - 8/13/2019
--------------------

::
   * Moves Changelog.md to this file, to be consistent with rst-ification
   * Sets up a Dockerfile to generate documentation for rmr-python using sphinx


[0.10.5] - 8/13/2019
--------------------

::

   * Make the PYPI page for rmr look nicer.

.. _section-1:

[0.10.4] - 8/08/2019
--------------------

::

   * Fix underlying problem getting errno from some environments; now references new RMR message field to get errno value.
   * Add /usr/local/lib64 to tox environment variable to support systems where libraries natually install in lib64 rather than lib.

.. _section-2:

[0.10.3] - 7/31/2019
--------------------

::

   * (Correctly) Include license here per Jira RICPLT-1855

.. _section-3:

[0.10.2] - 7/31/2019
--------------------

::

   * Include license here per Jira RICPLT-1855

.. _section-4:

[0.10.0] - 5/15/2019
--------------------

::

   * Fix a bug in rmr mock that prevented it for being used for rmr_rcv (was only usable for rmr_torcv)
   * Add more unit tests, esp for message summary
   * Remove meid truncation in the case where a nil is present mid string
   * Change the defaul mock of meid and get_src to something more useful

.. _section-5:

[0.9.0] - 5/13/2019
-------------------

::

   * Add a new module for mocking out rmr-python, useful for other packages that depend on rmr-python

.. _section-6:

[0.8.4] - 5/10/2019
-------------------

::

   * Add some unit tests; more to come

.. _section-7:

[0.8.3] - 5/8/2019
------------------

::

   * Better loop indexing in meid string handling

.. _section-8:

[0.8.2] - 5/8/2019
------------------

::

   * Fix examples bug
   * add liscneses for LF push

.. _section-9:

[0.8.1] - 5/7/2019
------------------

::

   * Better andling of meid in message summary

.. _section-10:

[0.8.0] - 5/7/2019
------------------

::

   * Refactor some code to be more functional
   * Put back RMR_MAX_RCV_BYTES as a constant
   * Add tox.ini, although right now it only LINTs

.. _section-11:

[0.7.0] - 5/6/2019
------------------

::

   * Add constant fetching from RMr library

.. _section-12:

[0.6.0] - 5/6/2019
------------------

::

   * Add a new field to rmr_mbuf_t: sub_id
   * Fix prior commits lint-ailing python style

.. _section-13:

[0.5.0] - 5/3/2019
------------------

::

   * Add errno access via new function: rmr.errno()
   * Add new functions to access new RMr header fields: get_src, get_meid, rmr_bytes2meid
   * Add new RMr constants for error states

.. _section-14:

[0.4.1] - 4/8/2019
------------------

::

   * Fix a non-ascii encoding issue

.. _section-15:

[0.4.0] - 3/28/2019
-------------------

::

   * Greatly imroved test sender/receiver
   * Three new functions implemented (rmr_close, rmr_set_stimeout, rmr_payload_size)

.. _section-16:

[0.3.0] - 3/26/2019
-------------------

::

   * Support a new receive function that (hurray!) has a timeout

.. _section-17:

[0.2.1] - 3/25/2019
-------------------

::

   * Add two new MR states

.. _section-18:

[0.2.0] - 3/25/2019
-------------------

::

   * Switch to NNG from nanomessage

.. _section-19:

[0.1.0] - 3/14/2019
-------------------

::

   * Initial Creation

.. This work is licensed under a Creative Commons Attribution 4.0 International License.
.. SPDX-License-Identifier: CC-BY-4.0
.. CAUTION: this document is generated from source in doc/src/rtd.
.. To make changes edit the source and recompile the document.
.. Do NOT make changes directly to .rst or .md files.

============================================================================================
RMR Release Notes
============================================================================================


RMR Release Notes
=================

The following is a list of release highlights for the RMR
library. At one point in time the RMR repo also housed a
wrapper library with a separate version and release cycle.
This resulted in *leap frogging* versions for each package;
the RMR core library was assigned odd major numbers (e.g.
3.1.0). When the wrapper code was moved to a different repo
the need to leap frog versions ceased, and beginning with
version 4.0.0, the RMR versions should no longer skip.


2020 August 4; Version 4.2.2
----------------------------

Correct bug in the rmr_probe support utility when -r option
is used on the command line (RIC-644)



2020 August 4; Version 4.2.1
----------------------------

Add additional environment variable dump if RMR_LOG_VLEVEL
set to 4 at start.



2020 August 3; Version 4.2.0
----------------------------

Add support for the RMR_RTREQ_FREQ environment variable to
control the request frequency for a new route table (default
5s if not supplied). (RIC-630)



2020 July 21; Version 4.1.4
---------------------------

Fix bug in SI95 -- possible use of pointer after free
(RIC-626).



2020 July 9; version 4.1.3
--------------------------

Allow RTS messages to be sent before the arrival of the
initial route table. Calls to RTS (heart beat responses)
prior to the initial route table load could cause a crash if
a framework blindly assumes that RTS is valid. (RIC-589)



2020 June 22; version 4.1.2
---------------------------

Fix typo in RIC Message header file.

Add document for message type constants and the scripts which
generate them.



2020 June 22; version 4.1.1
---------------------------

Add new message types to RIC header file for
    RIC_ALARM           (110)
    RIC_ALARM_QUERY     (111)
    RIC_METRICS         (120)
    RAN_E2_RESET_REQ    (12008)
    RAN_E2_RESET_RESP   (12009)



2020 June 18; version 4.1.0
---------------------------

Bump version minor to move away from 4.0.* which will bump
for any patches applied back to bronze.

Add magic C++ goo to symtab header file allowing C++ xAPPs to
use the symbol table directly.



Bronze Release
==============



2020 May 06; version 4.0.5
--------------------------

Fix the bug in SI95 receive message management semaphore
count issue. (RIC-355)



2020 April 29; version 4.0.4
----------------------------

Fix the traffic steering message type constants (again)
(RIC-342)



2020 April 28; version 4.0.3
----------------------------

Fix sonar flagged bugs (RIC-78)



2020 April 24; version 4.0.2
----------------------------

Correct bug in SI95 transport header length validation
(RIC-341)



2020 April 22; version 4.0.1
----------------------------

Correct message type constant for Traffic Steering
predication (RIC-342)



2020 April 21; version 4.0.0
----------------------------

The NNG based libraries are no longer included in the RMR
packages. This is considered a breaking change as NNG will
not be supported by default. It is still possible to build
with RMR-NNG libraries, but that is the exception. The API
between 3.8.2 and 4.0.0 is the SAME. Upgrading to 4.0.0 only
means that the underlying transport mechanism is limited only
to SI95.

The rmr_rcv_specific() function has been deprecated as it was
necessary only for NNG and Nanomsg support. Its use should be
discontinued.



2020 April 20; version 3.8.2
----------------------------

Fix bug which was preventing an instance receiving dynamic
route table updates. (RIC-336)



2020 April 20; version 3.8.1
----------------------------

Add user guide which replaces the concatenation of man pages
(RIC-328)



2020 April 17; version 3.8.0
----------------------------

Add safe connect to avoid potential connect bug on Linux
(RIC-332)

Change debugging in route table collector to avoid possible
segment fault when in level 2 debug (RIC-335)



2020 April 15; version 3.7.4
----------------------------

Add missing message type to header file (RIC-334)



2020 April 14; version 3.7.3
----------------------------

Fix bug in rmr_call() when using SI95 (RIC-333)



2020 April 10; version 3.7.2
----------------------------

Fix bug related to static route table only mode (RIC-331)



2020 April 9; version 3.7.1
---------------------------

The max length restriction for receiving messages when using
SI95 has been removed. The length supplied during
initialisation is used as the "normal maximum" and default
buffer allocation size, but messages arriving which are
larger are accepted. (RIC-309)



2020 April 7; version 3.7.0
---------------------------

The health check support programme was renamed to rmr_probe
(RIC-308).



2020 April 6; version 3.6.6
---------------------------

Correct bug in SI95 address conversion module (RIC-327)
Correct bug in SI initialisation module



2020 April 2; version 3.6.5
---------------------------

Correct potential nil pointer use when examining interfaces
for use as a listen target (RIC-307)



2020 April 1; version 3.6.4
---------------------------

Correct potential nil pointer use in the NNG interface
(RIC-303) Correct issue preventing CI build without a
container



2020 March 30; version 3.6.3
----------------------------

Correct the max receive message size constant in rmr.h
(RIC-301)



2020 March 23; version 3.6.2
----------------------------

Fix message initialisation bug when pulling a message from
the pool (RIC-295)



2020 March 19; version 3.6.1
----------------------------

Fix problem with RPM package install



2020 March 18; version 3.6.0
----------------------------

Add message types to support traffic steering



2020 March 16; version 3.5.2
----------------------------

Correct bug in the meid table parser that prevented the
ack/nack of meid tables (RIC-273)



2020 March 10; version 3.5.1
----------------------------

Add missing health check message types.



2020 March 9; version 3.5.0
---------------------------

Added new wormhole send function: rmr_wh_call().



2020 March 6; version 3.4.0
---------------------------

Add new wormhole state function: rmr_wh_state().



2020 March 5; Version 3.3.1
---------------------------

Correct several "bugs" identified by automatic code analysis.



2020 March 4; Version 3.3.0
---------------------------

Add SI95 based unit testing Health check support binary added
(reason for minor bump)



2020 February 26; version 3.2.5
-------------------------------

Fix source address bug in SI95 receive/send funcitons. Fix
threading issues involving session disconnection in SI95
Remove unused SI95 status variable.



2020 February 24; version 3.2.4
-------------------------------

Fix meid bug (RIC-220) causing core dump.



2020 February 21; version 3.2.3
-------------------------------

Add meid routing support to the SI95 interface.



2020 February 20; version 3.2.2
-------------------------------

Fix receive thread related core dump (ring early unlock).



2020 February 19; version 3.2.1
-------------------------------

Added missing message types (E2-Setup)



2020 February 18; version 3.2.0
-------------------------------

Added support for new Route Manager and it's ability to
accept a request for table update.



2020 February 14; version 3.1.3
-------------------------------

Fix bug in SIsend which was causing a core dump in some cases
where the application attempted to send on a connection that
had disconnected. (RIC-207).



2020 February 6; version 3.1.2
------------------------------

Fix disconnection detection bug in interface to SI95.



2020 January 31; verison 3.1.1
------------------------------

Allow route table thread logging to be completely disabled
when logging is turned off.



2020 January 26; verison 3.1.0
------------------------------

First step to allowing the user programme to control messages
written to standard error. Introduces the rmr_set_vlevel()
function, and related environment variable.



2020 January 24; verison 3.0.5
------------------------------

Fix bug in SI95 with receive buffer allocation.



2020 January 23; verison 3.0.4
------------------------------

Fix bug in SI95 causing excessive CPU usage on poll.



2020 January 22; verison 3.0.3
------------------------------

Enable thread support for multiple receive threads.



2020 January 21; verison 3.0.2
------------------------------

Fix bug in SI95 (missing reallocate payload function).



2020 January 20; verison 3.0.1
------------------------------

Enable support for dynamic route table updates via RMR
session.



2020 January 16; version 3.0.0
------------------------------

Introduce support for SI95 transport library to replace NNG.
(RMR library versions will use leading odd numbers to avoid
tag collisions with the wrapper tags which will use even
numbers.)



2019 December 9; version 1.13.1
-------------------------------

Correct documentation and missing rel-notes update for RTD.



2019 December 6; version 1.13.0
-------------------------------

Add ability to route messages based on the MEID in a message
combined with the message type/subscription-ID.



Amber Release
=============



2019 November 14; version 1.11.1
--------------------------------

Fix bug in payload reallocation function; correct length of
payload was not always copied.



2019 November 13; version 1.12.1
--------------------------------

New message type constants added to support A1.



2019 November 4; version 1.11.0
-------------------------------

Version bump to move away from the 1.10.* to distinguish
between release A and the trial.



2019 November 7; version 1.12.0
-------------------------------

Version cut to support continued development for next release
preserving the 1.11.* versions for release 1 (Amber) and
related fixes.



2019 October 31; version 1.10.2
-------------------------------

Provide the means to increase the payload size of a received
message without losing the data needed to use the
rmr_rts_msg() funciton.



2019 October 21; version 1.10.1
-------------------------------

Fix to prevent null message buffer from being returned by the
timeout receive function if the function is passed one to
reuse.



2019 October 21; version 1.10.1
-------------------------------

Add periodic dump of send count info to stderr.



2019 September 27; version 1.9.0
--------------------------------

Python bindings added receive all queued function and
corrected a unit test



2019 September 25; version 1.8.3
--------------------------------

Correct application level test issue causing timing problems
during jenkins verification testing at command and merge

Handle the NNG connection shutdown status which may now be
generated when a connection throug a proxy is reset.



2019 September 25; version 1.8.2
--------------------------------

Correct bug in rmr_torcv_msg() when timeout set to zero (0).



2019 September 19; version 1.8.1
--------------------------------

Correct missing constant for wrappers.



2019 September 19; version 1.8.0
--------------------------------

New message types added:
    RAN_CONNECTED, RAN_RESTARTED, RAN_RECONFIGURED



2019 September 17; version 1.7.0
--------------------------------

Initial connection mode now defaults to asynchronous. Set
RMR_ASYNC_CONN=0 in the environment before rmr_init() is
invoked to revert to synchronous first TCP connections.
(Recovery connection attempts have always been asynchronous).



2019 September 3; version 1.6.0
-------------------------------

Fix bug in the rmr_rts_msg() function. If a return to sender
message failed, the source IP address was not correctly
adjusted and could cause the message to be "reflected" back
to the sender on a retry.

Added the ability to set the source "ID" via an environment
var (RMR_SRC_ID). When present in the environment, the string
will be placed in to the message header as the source and
thus be used by an application calling rmr_rts_smg() to
return a response to the sender. If this environment variable
is not present, the host name (original behaviour) is used.



2019 August 26; version 1.4.0
-----------------------------

New message types were added.



2019 August 16; version 1.3.0
-----------------------------

New mesage types added.



2019 August 13; version 1.2.0 (API change, non-breaking)
--------------------------------------------------------

The function rmr_get_xact() was added to proide a convenient
way to extract the transaction field from a message.



2019 August 8; version 1.1.0 (API change)
-----------------------------------------

This change should be backward compatable/non-breaking A new
field has been added to the message buffer (rmr_mbuf_t). This
field (tp_state) is used to communicate the errno value that
the transport mechanism might set during send and/or receive
operations. C programmes should continue to use errno
directly, but in some environments wrappers may not be able
to access errno and this provides the value to them. See the
rmr_alloc_msg manual page for more details.



2019 August 6; version 1.0.45 (build changes)
---------------------------------------------

Support for the Nanomsg transport library has been dropped.
    The library librmr.* will no longer be included in packages.

Packages will install RMR libraries into the system preferred
    target directory. On some systems this is /usr/local/lib
    and on others it is /usr/local/lib64.  The diretory is
    determined by the sytem on which the package is built and
    NOT by the system installing the package, so it's possible
    that the RMR libraries end up in a strange location if the
    .deb or .rpm file was generated on a Linux flavour that
    has a different preference than the one where the package
    is installed.



2019 August 6; version 1.0.44 (API change)
------------------------------------------

Added a new message type constant.



2019 July 15; Version 1.0.39 (bug fix)
--------------------------------------

Prevent unnecessary usleep in retry loop.



2019 July 12; Version 1.0.38 (API change)
-----------------------------------------

Added new message types to RIC_message_types.h.



2019 July 11; Version 1.0.37
----------------------------


librmr and librmr_nng
    - Add message buffer API function rmr_trace_ref()
      (see rmr_trace_ref.3 manual page in dev package).



2020 April 8; Version n/a
-------------------------

RMR Python moved to Python Xapp Framework
(https://gerrit.o-ran-sc.org/r/admin/repos/ric-plt/xapp-frame-py)



2020 February 29; Version 2.4.0
-------------------------------

Add consolidated testing under CMake Add support binary for
health check (SI95 only)



2020 February 28; Version 2.3.6
-------------------------------

Fix bug in Rt. Mgr comm which prevented table ID from being
sent on ack message (RIC-232).

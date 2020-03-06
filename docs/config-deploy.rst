 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
RMR Configuration and Delpoyment 
============================================================================================ 
 
The RIC Message Router (RMR) is a library which applications 
use to send and receive messages where the message routing, 
endpoint selection, is based on the message type rather than 
on traditional DNS names or IP addresses. This document 
contains information regarding the configuration of RMR when 
it is embedded by a *user application* . RMR itself is not a 
deployable entity. 
 
Configuration 
-------------------------------------------------------------------------------------------- 
 
Several aspects of RMR behaviour is controlled via 
environment variables which are set at the time that a user 
application invokes the RMR initialisation function. This 
allows these variables to be set before the application is 
started as a function of the true environment, or set by the 
application as a means for the application to influence RMR's 
behaviour. The following is a list of RMR variables which RMR 
recognises (see the main RMR manual page in the development 
package for more details). ).** vim: ts=4 noet sw=42 
 
&ditem Allows the async connection mode to be turned off 
(by setting the value to 0. When set to 1, or missing from 
the environment, RMR will invoke the connection interface 
in the transport mechanism using the non-blocking (async) 
mode. This will likely result in many "soft failures" 
(retry) until the connection is established, but allows 
the application to continue unimpeeded should the 
connection be slow to set up. 
 
&ditem This provides the interface that RMR will bind 
listen ports to allowing for a single interface to be used 
rather than listening across all interfaces. This should 
be the IP address assigned to the interface that RMR 
should listen on, and if not defined RMR will listen on 
all interfaces. 
 
&ditem This variable defines the port that RMR should open 
for communications with Route Manager, and other RMR 
control applications. If not defined, the port 4561 is 
assumed. 
 
Previously, the RMR_RTG_SVC (route table generator service 
port) was used to define this port. However, a future 
version of Route Manager will require RMR to connect and 
request tables, thus that variable is now used to supply 
the Route Manager well known address and port. 
 
To maintain backwards compatablibility with the older 
Route Manager versions, the presence of this variable in 
the environment will shift RMR's behaviour with respect to 
the default value used when RMR_RTG_SVC is **not** 
defined. 
 
When RMR_CTL_PORT is **defined:** RMR assumes that Route 
Manager requires RMR to connect and request table updates 
is made, and the default well known address for Route 
manager is used (routemgr:4561). 
 
When RMR_CTL_PORT is **undefined:** RMR assumes that Route 
Manager will connect and push table updates, thus the 
default listen port (4561) is used. 
 
To avoid any possible misinterpretation and/or incorrect 
assumptions on the part of RMR, it is recommended that 
both the RMR_CTL_PORT and RMR_RTG_SVC be defined. In the 
case where both variables are defined, RMR will behave 
exactly as is communicated with the variable's values. 
 
&ditem The value of this variable depends on the Route 
Manager in use. 
 
When the Route Manager is expecting to connect to an xAPP 
and push route tables, this variable must indicate the 
port which RMR should use to listen for these connections. 
 
When the Route Manager is expecting RMR to connect and 
request a table update during initialisation, the variable 
should be the host of the Route Manager process. 
 
The RMR_CTL_PORT variable (added with the support of 
sending table update requests to Route manager), controls 
the behaviour if this variable is not set. See the 
description of that variable for details. 
 
&ditem By default RMR writes messages to standard error 
(incorrectly referred to as log messages) in human 
readable format. If this environment variable is set to 0, 
the format of standard error messages might be written in 
some format not easily read by humans. If missing, a value 
of 1 is assumed. 
 
&ditem This is a numeric value which corresponds to the 
verbosity level used to limit messages written to standard 
error. The lower the number the less chatty RMR functions 
are during execution. The following is the current 
relationship between the value set on this variable and 
the messages written: 
 
&ditem Off; no messages of any sort are written. 
 
&ditem Only critical messages are written (default if 
this variable does not exist) 
 
&ditem Errors and all messages written with a lower 
value. 
 
&ditem Warnings and all messages written with a lower 
value. 
 
&ditem Informational and all messages written with a 
lower value. 
 
&ditem Debugging mode -- all messages written, however 
this requires RMR to have been compiled with debugging 
support enabled. 
 
 
&ditem **Deprecated.** Should be set to 1 if the route 
table generator is sending "plain" messages (not using RMR 
to send messages, 0 if the rtg is using RMR to send. The 
default is 1 as we don't expect the rtg to use RMR. 
 
This variable is only recognised when using the NNG 
transport library as it is not possible to support NNG 
"raw" communications with other transport libraries. It is 
also necessary to match the value of this variable with 
the capabilities of the Route Manager; at some point in 
the future RMR will assume that all Route Manager messages 
will arrive via an RMR connection and will ignore this 
variable. &ditem This is used to supply a static route 
table which can be used for debugging, testing, or if no 
route table generator process is being used to supply the 
route table. If not defined, no static table is used and 
RMR will not report *ready* until a table is received. The 
static route table may contain both the route table 
(between newrt start and end records), and the MEID map 
(between meid_map start and end records) &ditem This is 
either the name or IP address which is placed into 
outbound messages as the message source. This will used 
when an RMR based application uses the rmr_rts_msg() 
function to return a response to the sender. If not 
supplied RMR will use the hostname which in some container 
environments might not be routable. 
 
The value of this variable is also used for Route Manager 
messages which are sent via an RMR connection. &ditem This 
supplies the name of a verbosity control file. The core 
RMR functions do not produce messages unless there is a 
critical failure. However, the route table collection 
thread, not a part of the main message processing 
component, can write additional messages to standard 
error. If this variable is set, RMR will extract the 
verbosity level for these messages (0 is silent) from the 
first line of the file. Changes to the file are detected 
and thus the level can be changed dynamically, however RMR 
will only suss out this variable during initialisation, so 
it is impossible to enable verbosity after startup. &ditem 
If set to 1, RMR will write some warnings which are 
non-performance impacting. If the variable is not defined, 
or set to 0, RMR will not write these additional warnings. 
 

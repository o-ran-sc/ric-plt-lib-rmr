 
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
package for more details). 
 
 
RMR_ASYNC_CONN 
   
  Allows the asynch connection mode to be turned off (by 
  setting the value to 0. When set to 1, or missing from the 
  environment, RMR will invoke the connection interface in 
  the transport mechanism using the non-blocking (asynch) 
  mode. This will likely result in many "soft failures" 
  (retry) until the connection is established, but allows 
  the application to continue unimpeeded should the 
  connection be slow to set up. 
 
RMR_BIND_IF 
   
  This provides the interface that RMr will bind listen 
  ports to allowing for a single interface to be used rather 
  than listening across all interfaces. This should be the 
  IP address assigned to the interface that RMr should 
  listen on, and if not defined RMr will listen on all 
  interfaces. 
 
RMR_RTG_SVC 
   
  RMr opens a TCP listen socket using the port defined by 
  this environment variable and expects that the route table 
  generator process will connect to this port. If not 
  supplied the port 4561 is used. 
 
RMR_RTG_ISRAW 
   
  Is set to 1 if the route table generator is sending 
  "plain" messages (not using RMr to send messages, 0 if the 
  rtg is using RMr to send. The default is 1 as we don't 
  expect the rtg to use RMr. 
 
RMR_SEED_RT 
   
  This is used to supply a static route table which can be 
  used for debugging, testing, or if no route table 
  generator process is being used to supply the route table. 
  If not defined, no static table is used and RMr will not 
  report *ready* until a table is received. 
 
RMR_SRC_ID 
   
  This is either the name or IP address which is placed into 
  outbound messages as the message source. This will used 
  when an RMR based application uses the rmr_rts_msg() 
  function to return a response to the sender. If not 
  supplied RMR will use the hostname which in some container 
  environments might not be routable. 
 
RMR_VCTL_FILE 
   
  This supplies the name of a verbosity control file. The 
  core RMR functions do not produce messages unless there is 
  a critical failure. However, the route table collection 
  thread, not a part of the main message processing 
  component, can write additional messages to standard 
  error. If this variable is set, RMR will extract the 
  verbosity level for these messages (0 is silent) from the 
  first line of the file. Changes to the file are detected 
  and thus the level can be changed dynamically, however RMR 
  will only suss out this variable during initialisation, so 
  it is impossible to enable verbosity after startup. 
 
RMR_WARNINGS 
   
  If set to 1, RMR will write some warnings which are 
  non-performance impacting. If the variable is not defined, 
  or set to 0, RMR will not write these additional warnings. 
 

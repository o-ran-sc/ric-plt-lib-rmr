.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 


OVERVIEW
========

Messages sent via the RIC Message Router (RMR) are directed 
to an endpoint (another application) based on a combination 
of the *message type* and *subscription ID,* or solely on the 
*message type* (MT) if the *subscription ID* (SID) is not 
supplied, which are supplied in the message. RMR determines 
the endpoint by matching the MT and SID combination to an 
entry in a route table which has been supplied dynamically by 
a *Route Manager* service, or as a static table loaded during 
RMR initialisation. It is also possible to route messages 
directly to an endpoint which is the *managed entity* 
"owner," using the *managed entity ID* (MEID). 
 
For most xAPP developers the format of the RMR route table is 
not important beyond understanding how to create a static 
table for local testing. For developers of a *Route Manager* 
service, the need is certainly a requirement. This document 
describes the overall syntax of a route table and the 
interface between the *Route Manager* service and RMR. 


CONTENTS OF A ROUTE TABLE
=========================

The table consists of a start record, one or more entry 
records, and an end record. Each entry record defines one 
message type, with an optional sender application, and the 
endpoint(s) which accept the indicated message type. All 
table records contain fields separated with vertical bars 
(|), and allow for trailing comments with the standard shell 
comment symbol (hash, #) provided that the start of the 
comment is separated from the last token on the record by one 
or more spaces. Leading, and trailing, white space in each 
field is ignored. 


Entry record syntax
-------------------

Two types of table entries are supported for compatibility 
with the original RMR implementation, but only the *mse* 
entry type is needed and that should be the entry used when 
creating new tables. The following shows the syntax for both 
entry types 
 
 
:: 
 
  rte | <msg-type>[,<sender-endpoint>] | <endpoint-group>[;<endpoint-group>;...]
  mse | <msg-type>[,<sender-endpoint>] | <sub-id> | <endpoint-group>[;<endpoint-group>;...]
 
 
Where: 
 
 
   .. list-table:: 
     :widths: 15,80 
     :header-rows: 0 
     :class: borderless 
      
     * - **mse, rte** 
       - 
         is the table entry type 
          
          
         | 
      
     * - **<msg-type>** 
       - 
         is the integer message type 
          
          
         | 
      
     * - **<sender-endpoint>** 
       - 
         is the endpoint description of the message sender; only that 
         sender will read the entry from the table, so a single table 
         may be used for all senders when a common message type is 
         delivered to varying endpoints based on senders. If the 
         sender endpoint is omitted from the entry, then the entry 
         will be used by all applications which read the table. 
          
          
         | 
      
     * - **<sub-id>** 
       - 
         is the subscription id (integer) for subscription based 
         messages, or -1 if the message type is not subscription 
         based. An *mse* entry with a sub-id of -1 is the **same** as 
         an *rte* entry with the same message type. 
          
          
         | 
      
     * - **<endpoint-group>** 
       - 
         is one or more, comma separated, endpoint descriptions. 
          
 
 
When an application sends a message with the indicated type, 
the message will be sent to one endpoint in the group in a 
round-robin ordering. If multiple endpoint groups are given, 
then the message is sent to a member selected from each 
group; 3 groups, then three messages will be sent. The first 
group is required. 


Line separation
---------------

Table entries **must** end with a newline character, and a 
final newline following the last line of the file is also 
expected. When using a text editor to generate a static route 
table ensure that "carriage return newline" pairs are not 
generated as line endings. 


Table framing
-------------

The route table parser within RMR assumes that route table 
entries are sent via RMR messages as a stream. Two ensure 
synchronisation, and prevent malformed tables because of 
broken sessions, or lost packets, each table must begin and 
end with an *newrt* record. Each *newrt* record has one of 
two possible syntax layouts as described below. 
 
 
:: 
 
   newrt | begin [ | table-id-string]
   newrt | end  [ | record-count]
 
Figure 1: 
 
The *table-id-string* is an optional string which is used by 
RMR when sending an acknowledgement back to the *Route 
Manager* service (see the *Route Manager Interface* section 
for more details). If a *record-count* is given as the final 
field on the *end* record, RMR will verify that the number of 
*mse* and *rte* entries in the table matches the count; if 
there is a mismatch in values the table is not used. 


Comments, spaces, and blank lines
---------------------------------

Comments may be placed to the right of any table entry line 
using the standard shell comment symbol (#). The start of a 
comment must be separated from the endpoint list by at least 
one space or tab. Complete lines are treated as comments when 
the first character of a line is a comment symbol; leading 
spaces and/or tabs are allowed. Blank lines are also ignored. 
 
Fields on table records are separated using the vertical bar 
(|) character. Any white space (tabs or spaces) which appear 
immediately before or after a separator are ignored. 


Endpoint Description
--------------------

The endpoint description is either the hostname or IP address 
followed by a port number; the two are separated by a single 
colon. Whether names or IP addresses are used in static 
tables during testing depends on the test environment. The 
illustration below assumes that host names (e.g. forwarder 
and app1) are defined; they also make the tables easier to 
read, but might not always be available depending on the 
environment used to test the applications. The port number 
given is the port number that the user application provides 
to RMR when the RMR initialisation function is invoked (and 
thus is the port that RMR is listening on). 


TABLE MECHANICS
===============

Creating a table from the two entry types is fairly straight 
forward, however there are some subtleties which should be 
pointed out to avoid unexpected behaviour. For this 
discussion the following, complete, table will be used. 
 
.. list-table:: 
  :widths: 75,10 
  :header-rows: 0 
  :class: borderless 
 
 
  * -  
        
       :: 
        
           newrt | start | rt-0928
           rte | 2000 | logger:30311
           mse | 1000 | 10 | forwarder:43086
           mse | 1000,forwarder:43086 | 10 | app2:43086
           mse | 1000 | -1 | app0:43086,app1:43086; logger:20311
           newrt | end | 3
        
    -  
        
       :: 
        
         (1)
         (2)
         (3)
         (4)
         (5)
         (6)
        
        
Figure 2: 


Table Entry Ordering
--------------------

Whether a table is read from a file on disk, or is received 
from a *Route Manager* service, RMR parses the records to 
build an internal route table keeping only the relevant 
information. Entries are read in the order they appear (from 
the file or in messages received), and RMR will use only one 
entry for each MT/SID pair. 
 
For most tables, the ordering of entries is not important, 
but when there are entries which duplicate the MT/SID pair 
ordering becomes significant. RMR will use the **last** valid 
entry for a MT/SID pair that it encounters. An entry is 
considered valid if there is no sender identified with the 
message type (line 3), and when the sender (host and port) 
match the the applications' location and the port provided to 
RMR for listening. 
 
Using the table in figure 2 as an example, there are two 
entries which match the MT/SID pair of 1000/10. When this 
table is parsed on any host, RMR will recognise and add the 
first entry (line 3) to the internal representation; this 
entry is valid for all applications. The second 1000/10 entry 
(line 4) is valid when the table is parsed on the *forwarder* 
host, and only by the application which is listening on port 
43086. When this entry is valid, it will be used in place of 
any previously captured entry. 
 
As a rule, the ordering of entries for a given MT/SID pair 
should be from most generic to most specific. 


ROUTE MANAGER COMMUNICATIONS
============================

During initialisation RMR will use the value of the 
``RMR_RTG_SVC`` environment variable, to connect to the 
*Route Manager* service in order to request a route table. 
The connection between RMR and the *Route Manager* is also an 
RMR session and thus RMR messages will be used to exchange 
requests and responses. 


Table Request
-------------

During initialisation, RMR will send a message to the & RMR 
expects to communicate with the *Route Manager* using an RMR 
session and will send a message type of 21 to request a new 
table. RMR will continue to send table requests until a table 
is received and accepted; in other words it is fine for the 
*Route Manager* to ignore the requests if it is not ready to 
respond. 


Sending Tables To RMR
---------------------

Table entry data is expected to arrive via RMR message with a 
message type of 20. The message may contain one or more 
entries provided that the entries are newline separated. 
Current versions of RMR support very large messages, however 
to ensure compatibility with an xAPP built using an older 
version of RMR (pre 3.8) messages should be limited to 4 KiB. 


Table Acceptance and Acknowledgement
------------------------------------

When RMR receives the table end entry (newrt|end), it will 
send a state message back to the &RTMGR to indicate the state 
of the received table. The message type is 22 and the payload 
will contain ASCII tokens which indicate the state. The 
second token will be the *table ID* supplied on the start 
record, or the string "<id-missing>." When the state is an 
error state, RMR might add a final set of tokens which 
contain the reason for the failure. 
 
Upon receipt of a status message which indicates an "OK" 
response, the &RTMGR can assume that the table has been 
installed and is in use. Any other response indicates that 
RMR did not use the table and has dropped it; the previous 
table is still in use. 


PROVIDING A STATIC TABLE
========================

For testing, or possibly bootstrapping purposes, a static 
route table can be supplied. During initialisation, RMR will 
check the ``RMR_SEED_RT`` environment variable. If it exists, 
and references a file, RMR will open and read the file 
expecting to find a static route table. This route table is 
used until an update is received from a &RTMGR. Normally, 
when the RMR initialisation function is invoked, a listener 
is started to receive route table information from a route 
manager process. During testing it is often useful to be able 
to supply a static table which is available should no route 
management process exist, or to provide a seed table to use 
before the first table is delivered. The environment variable 
RMR_SEED_RT can be set to provide the RMR initialisation 
function with the name of the static table to use. If a 
static table is provided, it will be loaded only once, and if 
a route manager subsequently connects and delivers a complete 
or updated table, it will override the static table. 


ROUTING USING MEID
==================

Starting with version 1.13.0, RMR provides the ability to 
select the endpoint for a message based on the MEID (managed 
entity ID) in the message, rather than selecting the endpoint 
from the round-robin list for the matching route table entry. 
To make use of this, there must be one or more route table 
entries which list the special endpoint name %meid instead of 
providing a round robin list. As an example, consider the 
following route table entry: 
 
 
:: 
 
   mse| 1000,forwarder:43086 | 10 | %meid
 
Figure 3: 
 
The final field of the entry doesn't specify a round-robin 
group which means that when an application attempts to send a 
message with type 1000, and the subscription ID of 10, the 
MEID in the message will be used to select the endpoint. 


MEID endpoint selection
-----------------------

To select an endpoint for the message based on the MEID in a 
message, RMR must know which endpoint owns the MEID. This 
information, known as an MEID map, is provided by the *Route 
Manager* over the same communication path as the route table 
is supplied. The following is the syntax for an MEID map. 
 
 
:: 
 
   meid_map | start | <table-id>
   mme_ar | <owner-endpoint> | <meid> [<meid>...]
   mme_del | <meid> [<meid>...]
   meid_map | end | <count> [| <md5sum> ]
 
Figure 4: 
 
The mme_ar records are add/update records and allow for the 
list of MEIDs to be associated with (owned by) the indicated 
endpoint. The <owner-endpoint> is the hostname:port, or IP 
address and port, of the application which owns the MEID and 
thus should receive any messages which are routed based on a 
route table entry with %meid as the round-robin group. The 
mme_del records allow for MEIDs to be deleted from RMR's 
view. Finally, the <count> is the number of add/replace and 
delete records which were sent; if RMR does not match the 
<count> value to the number of records, then it will not add 
the data to the table. Updates only need to list the 
ownership changes that are necessary; in other words, the 
*Route Manager* does not need to supply all of the MEID 
relationships with each update. 
 
The optional <md5sum> field on the end record should be the 
MD5 hash of all of the records between the start and end 
records. This allows for a more precise verification that the 
transmitted data was correctly received. 
 
If a static seed file is being used for the route table, a 
second section can be given which supplies the MEID map. The 
following is a small example of a seed file: 
 
 
:: 
 
  newrt|start | id-64306
  mse|0|-1| %meid
  mse|1|-1|172.19.0.2:4560
  mse|2|-1|172.19.0.2:4560
  mse|3|-1|172.19.0.2:4560
  mse|4|-1|172.19.0.2:4560
  mse|5|-1|172.19.0.2:4560
  newrt|end
  
  meid_map | start | id-028919
  mme_ar| 172.19.0.2:4560 | meid000 meid001 meid002 meid003 meid004 meid005
  mme_ar| 172.19.0.42:4560 | meid100 meid101 meid102 meid103
  mme_del | meid1000
  meid_map | end | 1
 
Figure 5: 
 
The tables above will route all messages with a message type 
of 0 based on the MEID. There are 10 meids which are owned by 
two different endpoints. The table also deletes the MEID 
meid1000 from RMR's view. 


RESERVED MESSAGE TYPES
======================

RMR is currently reserving message types in the range of 0 
through 99 (inclusive) for its own use. Please do not use 
these types in any production or test environment as the 
results may be undesired. 
 

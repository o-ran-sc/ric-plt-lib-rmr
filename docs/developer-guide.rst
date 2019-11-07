 
.. This work is licensed under a Creative Commons Attribution 4.0 International License. 
.. SPDX-License-Identifier: CC-BY-4.0 
.. CAUTION: this document is generated from source in doc/src/rtd. 
.. To make changes edit the source and recompile the document. 
.. Do NOT make changes directly to .rst or .md files. 
 
 
RMR Developer Guide 
============================================================================================ 
 
The RIC Message Router (RMR) is a library which applications 
use to send and receive messages where the message routing, 
endpoint selection, is based on the message type rather than 
on traditional DNS names or IP addresses. This document 
contains information that potential developers might need to 
know in order to contribute to the project 
 
Language 
-------------------------------------------------------------------------------------------- 
 
RMR is written in C, and thus a contributing developer to the 
core library should have an excellent working knowledge of C. 
There currently is one set of cross languages bindings 
supporting Python, and a developer wishing to contribute to 
the bindings source should be familiar with Python (version 
3.7+) and with the Python *ctypes* library. 
 
Code Structure 
-------------------------------------------------------------------------------------------- 
 
RMR is designed to provide an insulation layer between user 
applications and the actual transport mechanism. Initially 
RMR was built on top of Nanosmg, and shortly after was ported 
to NNG (Nanomsg Next Generation). Because RMR presents the 
same API to the user application regardless of the underlying 
transport library, the resulting output when compiling RMR is 
a transport specific library. As an example, librmr_nng.a is 
the library generated for use with the NNG transport. 
 
As such the library source is organised into multiple 
components: 
 
 
common 
   
  Source in the common directory is agnostic to the 
  underlying transport mechanism (Nanomsg or NNG), and thus 
  can be used when generating either library. 
 
nano 
   
  Source which is tightly coupled with the underlying 
  Nanomsg library. (Nanomsg has been deprecated, but the RMR 
  source remains as an example.) 
 
nng 
   
  Source which is tightly coupled with the underlying NNG 
  library. 
 
 
 
Internal Function Exposure 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 
The decision to limit as much as practical the exposure of 
truely internal RMR functions was made, and as a result most 
of the RMR functions carry a static label. In order to 
modularise the code as much as possible, this means that the 
primary module (e.g. rmr_nng.c) will directly include other 
RMR modules, rather than depending on referencing the 
internal functions during linking. While this is an 
infrequently used approach, it does mean that there are very 
few functions visible for the user application to reference, 
all of them having the prefix rmr\_, while allowing internal 
functions to have shorter names while still being meaningful. 
 
Coding Style 
-------------------------------------------------------------------------------------------- 
 
There is a list of coding style guidelines in the top level 
directory, and as such they are not expanded upon here. The 
general practice is to follow the style when editing an 
existing module, respect the author's choice where style 
alternatives are not frowned upon. When creating new modules, 
select a style that fits the guidelines and is easy for you 
to work with. There are a few things that are insisted on by 
the maintainers of RMR, but for the most part style is up to 
the creator of a module. 
 
Building 
-------------------------------------------------------------------------------------------- 
 
RMR is constructed using CMake. While CMake's project 
description can be more cumbersome than most typical 
Makefiles, the tool provides convenience especially when it 
comes to creating packages. 

#
#==================================================================================
#       Copyright (c) 2019 Nokia
#       Copyright (c) 2018-2019 AT&T Intellectual Property.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#==================================================================================
#


Building RMr

The RIC Message Router (RMr) is built with CMake, and requires
a modern gcc compiler and make to be installed on the build
system. Typically, installing the following list of packages
in a container (Ubuntu) is all that is needed to craft a
development environment (containerised builds are also the
recommended approach):

 gcc git make vim cmake g++ ksh bash

Kshell and vi are needed only if you wish to use the container
interactively. Bash is assumed necessary for CMake.


Build process
To build RMr, the usual CMake steps are followed:
	mkdir build
	cd build
	cmake .. [options]
	make package


This will create a .deb (provided the system supports this) in
the build directory.  It's that simple.

Continuous Integration Build
Use the Dockerfile in the ci/ subdirectory. This installs all
the required tools, then biulds RMr and executes the unit and
programm tests. If tests pass, then  an image is created in the 
local registry with both run-time and development packages.

To support the distribution of package(s) created during the
build by the CI process,  a YAML file is left in the /tmp
directory (build_packages.yml) which contains a list of the
packages available from the image.  Currently, both .deb and
.rpm packages are generated.


Alternatives
To build in a non-Linux environment, or to build with an
alternate install path (or both) read on.

Instead of using 'make package' as listed above, using
'make install'  will build and install on the local system.
By default, the target install is into /usr/local which may
not be desired.  To install into an alternate path add
these two options when the 'cmake ..' command is given:

  -DCMAKE_INSTALL_PREFIX=/path/to/dir
  -DMAN_PREFIX=/path/to/dir


The first will cause the make process to install into the named
directory, which can be in your home directory. The second
defines where manual pages are placed (if not defined
/usr/share/man is the target).   Manual pages are generally
NOT built as the required tool has yet to be incorporated into
the build process and generally is not available on most systems.


Compiling and Linking User Applications
Should the Rmr and NNG/Nano libraries be installed in a directory
outside of the normal system spots (e.g. not in /usr/local)
it might be necessary to define the specific directory for
libraries (.e.g -L) on the command line, or via environment
variables (e.g.. C_INCLUDE_PATH, LD_LIBRARY_PATH, LIBRARY_PATH).
It may also be necessary to have the library directory defined
in the environment at run time.  It is difficult to know what
each system needs, but the following linker ooptions  work when
libraries are installed in the system spots:

	-lrmr_nng -lnng -lpthread

Adding -L is one way to compensate when libraries are installed
a different spot (e.g. in $HOME/usr):

	-L $HOME/usr -lrmr_nng -lnng -lpthread


Libraries
RMr supports both NNG and Nanomsg as underlying transport. They
are separate beasts, and while an NNG based programme can
communicate with a Nanomsg based programme, their APIs are NOT
compatible.  For this reason, and others, RMr generates two
libraries and requires that the underlying transport be selected
at link time rather than run time.  The RMr API for both underlying
mechanisms is the same, so generating a NNG and Nanomsg version
of a programme should require no extra work; other than adding
a second link statement and giving it a different name.

Nanomsg is on its way out with respect to community support. RMr
will continue to support Nanomsg for a short period of time, but
new programmes should NOT use Nanomsg.


Manual Pages
By default the deb created does not include the manual pages. To
enable their creation, and subsequent inclusion in the deb, add
the following option to the cmake command:

	-DBUILD_DOC=1

This will cause the {X}fm text formatting package to be fetched
(github) and built at cmake time (must exist before building)
and will trigger the generation of the man pages in both postscript
and troff format.  The troff pages are placed into the deb and
the postscript pages are left in the build directory for the
developer to convert to PDF, or otherwise use.



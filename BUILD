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


Building RMR

The RIC Message Router (RMR) is built with CMake, and requires
a modern gcc compiler and make to be installed on the build
system. Typically, installing the following list of packages
in a container (Ubuntu) is all that is needed to craft a
development environment (containerised builds are also the
recommended approach):

 gcc git make vim cmake g++ ksh bash

Kshell and vi are needed only if you wish to use the container
interactively. Bash is assumed necessary for CMake.


Build process
To build RMR, the usual CMake steps are followed:
	mkdir build
	cd build
	cmake .. [options]
	make package


This will create a .deb (provided the system supports this) in
the build directory.  It's that simple.

The following flags may be given on the 'cmake' command line
(options) which are outside of "normal" CMake flags and affect
the configuration:

  -DBUILD_DOC=1         Man pages generated
  -DDEV_PKG=1			Development package configuration
  -DIGNORE_LIBDIR		Ignore the system preferred library directory and install in /usr/local/lib
  -DMAN_PREFIX=<path>	Supply a path where man pages are installed (default: /usr/share/man)
  -DPACK_EXTERNALS=1	Include external libraries used to build in the run-time package
  -DPRESERVE_PTYPE=1	Do not change the processor type when naming deb packages
  -DSKIP_EXTERNALS=1	Do not use Nano/NNG submodules when building; uee installed packages


Packages
The build can be configured to generate either a run-time or
development package. The run-time generation is the default and
the -DDEV_PKG=1 option must be given to generate the development
package.  The run-time package contains only the shared library
files (*.so), and the development package contains the headers,
man pages (if the man option is set) and archive (.a) files.
Resulting package names are illustrated in the CI section below.


Continuous Integration Build
Use the Dockerfile in the ci/ subdirectory. This installs all
the required tools, then builds RMR and executes the unit and
programm tests. If tests pass, then  an image is created in the
local registry with both run-time and development packages.

To support the distribution of package(s) created during the
build by the CI process,  a YAML file is left in the /tmp
directory (build_packages.yml) which contains a list of the
packages available from the image.  Currently, both .deb and
.rpm packages are generated.

The following is a sample YAML file generated during this process:

   ---
   files:
     - /tmp/rmr-dev_1.0.34_x86_64.deb
     - /tmp/rmr-dev-1.0.34-x86_64.rpm
     - /tmp/rmr_1.0.34_x86_64.deb
     - /tmp/rmr-1.0.34-x86_64.rpm
   ...



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
RMR supports only NNG as the underlying transport. Nanomsg
support was dropped in starting with version 1.0.45 as Nanomsg
has reached end of life. The package generation and install
will produce a single RMR library:  librmr_nng.  RMR is designed
to support different underlying transport mechanisms, which
might require separate libraries, and thus the library name is
given a suffix of _nng to reflect the transport mechanism
in use.

Regardless of transport mechanism supported by an RMR library,
the RMR API will be identical, thus it is possible for an appliction
to shift mechanisms simply by referencing a differnt library (should
multiple RMR libraries become available).


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



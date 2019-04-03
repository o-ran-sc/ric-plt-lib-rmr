
Building RMr

The RIC Message Router (RMr) is built with CMake, and requires
a modern gcc compiler and make to be installed on the  build
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


Compiling and Linking
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



# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build

# Include any dependencies generated for this target.
include tests/CMakeFiles/wsstream.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/wsstream.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/wsstream.dir/flags.make

tests/CMakeFiles/wsstream.dir/wsstream.c.o: tests/CMakeFiles/wsstream.dir/flags.make
tests/CMakeFiles/wsstream.dir/wsstream.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/wsstream.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/wsstream.dir/wsstream.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/wsstream.dir/wsstream.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/wsstream.c

tests/CMakeFiles/wsstream.dir/wsstream.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/wsstream.dir/wsstream.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/wsstream.c > CMakeFiles/wsstream.dir/wsstream.c.i

tests/CMakeFiles/wsstream.dir/wsstream.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/wsstream.dir/wsstream.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/wsstream.c -o CMakeFiles/wsstream.dir/wsstream.c.s

tests/CMakeFiles/wsstream.dir/wsstream.c.o.requires:

.PHONY : tests/CMakeFiles/wsstream.dir/wsstream.c.o.requires

tests/CMakeFiles/wsstream.dir/wsstream.c.o.provides: tests/CMakeFiles/wsstream.dir/wsstream.c.o.requires
	$(MAKE) -f tests/CMakeFiles/wsstream.dir/build.make tests/CMakeFiles/wsstream.dir/wsstream.c.o.provides.build
.PHONY : tests/CMakeFiles/wsstream.dir/wsstream.c.o.provides

tests/CMakeFiles/wsstream.dir/wsstream.c.o.provides.build: tests/CMakeFiles/wsstream.dir/wsstream.c.o


tests/CMakeFiles/wsstream.dir/convey.c.o: tests/CMakeFiles/wsstream.dir/flags.make
tests/CMakeFiles/wsstream.dir/convey.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/wsstream.dir/convey.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/wsstream.dir/convey.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c

tests/CMakeFiles/wsstream.dir/convey.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/wsstream.dir/convey.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c > CMakeFiles/wsstream.dir/convey.c.i

tests/CMakeFiles/wsstream.dir/convey.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/wsstream.dir/convey.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c -o CMakeFiles/wsstream.dir/convey.c.s

tests/CMakeFiles/wsstream.dir/convey.c.o.requires:

.PHONY : tests/CMakeFiles/wsstream.dir/convey.c.o.requires

tests/CMakeFiles/wsstream.dir/convey.c.o.provides: tests/CMakeFiles/wsstream.dir/convey.c.o.requires
	$(MAKE) -f tests/CMakeFiles/wsstream.dir/build.make tests/CMakeFiles/wsstream.dir/convey.c.o.provides.build
.PHONY : tests/CMakeFiles/wsstream.dir/convey.c.o.provides

tests/CMakeFiles/wsstream.dir/convey.c.o.provides.build: tests/CMakeFiles/wsstream.dir/convey.c.o


# Object files for target wsstream
wsstream_OBJECTS = \
"CMakeFiles/wsstream.dir/wsstream.c.o" \
"CMakeFiles/wsstream.dir/convey.c.o"

# External object files for target wsstream
wsstream_EXTERNAL_OBJECTS =

tests/wsstream: tests/CMakeFiles/wsstream.dir/wsstream.c.o
tests/wsstream: tests/CMakeFiles/wsstream.dir/convey.c.o
tests/wsstream: tests/CMakeFiles/wsstream.dir/build.make
tests/wsstream: libnng.so.1.1.0
tests/wsstream: tests/CMakeFiles/wsstream.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable wsstream"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/wsstream.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/wsstream.dir/build: tests/wsstream

.PHONY : tests/CMakeFiles/wsstream.dir/build

tests/CMakeFiles/wsstream.dir/requires: tests/CMakeFiles/wsstream.dir/wsstream.c.o.requires
tests/CMakeFiles/wsstream.dir/requires: tests/CMakeFiles/wsstream.dir/convey.c.o.requires

.PHONY : tests/CMakeFiles/wsstream.dir/requires

tests/CMakeFiles/wsstream.dir/clean:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/wsstream.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/wsstream.dir/clean

tests/CMakeFiles/wsstream.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/CMakeFiles/wsstream.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/wsstream.dir/depend


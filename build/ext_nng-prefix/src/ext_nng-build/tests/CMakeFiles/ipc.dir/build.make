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
include tests/CMakeFiles/ipc.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/ipc.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/ipc.dir/flags.make

tests/CMakeFiles/ipc.dir/ipc.c.o: tests/CMakeFiles/ipc.dir/flags.make
tests/CMakeFiles/ipc.dir/ipc.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/ipc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/ipc.dir/ipc.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ipc.dir/ipc.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/ipc.c

tests/CMakeFiles/ipc.dir/ipc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ipc.dir/ipc.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/ipc.c > CMakeFiles/ipc.dir/ipc.c.i

tests/CMakeFiles/ipc.dir/ipc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ipc.dir/ipc.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/ipc.c -o CMakeFiles/ipc.dir/ipc.c.s

tests/CMakeFiles/ipc.dir/ipc.c.o.requires:

.PHONY : tests/CMakeFiles/ipc.dir/ipc.c.o.requires

tests/CMakeFiles/ipc.dir/ipc.c.o.provides: tests/CMakeFiles/ipc.dir/ipc.c.o.requires
	$(MAKE) -f tests/CMakeFiles/ipc.dir/build.make tests/CMakeFiles/ipc.dir/ipc.c.o.provides.build
.PHONY : tests/CMakeFiles/ipc.dir/ipc.c.o.provides

tests/CMakeFiles/ipc.dir/ipc.c.o.provides.build: tests/CMakeFiles/ipc.dir/ipc.c.o


tests/CMakeFiles/ipc.dir/convey.c.o: tests/CMakeFiles/ipc.dir/flags.make
tests/CMakeFiles/ipc.dir/convey.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/ipc.dir/convey.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ipc.dir/convey.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c

tests/CMakeFiles/ipc.dir/convey.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ipc.dir/convey.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c > CMakeFiles/ipc.dir/convey.c.i

tests/CMakeFiles/ipc.dir/convey.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ipc.dir/convey.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c -o CMakeFiles/ipc.dir/convey.c.s

tests/CMakeFiles/ipc.dir/convey.c.o.requires:

.PHONY : tests/CMakeFiles/ipc.dir/convey.c.o.requires

tests/CMakeFiles/ipc.dir/convey.c.o.provides: tests/CMakeFiles/ipc.dir/convey.c.o.requires
	$(MAKE) -f tests/CMakeFiles/ipc.dir/build.make tests/CMakeFiles/ipc.dir/convey.c.o.provides.build
.PHONY : tests/CMakeFiles/ipc.dir/convey.c.o.provides

tests/CMakeFiles/ipc.dir/convey.c.o.provides.build: tests/CMakeFiles/ipc.dir/convey.c.o


# Object files for target ipc
ipc_OBJECTS = \
"CMakeFiles/ipc.dir/ipc.c.o" \
"CMakeFiles/ipc.dir/convey.c.o"

# External object files for target ipc
ipc_EXTERNAL_OBJECTS =

tests/ipc: tests/CMakeFiles/ipc.dir/ipc.c.o
tests/ipc: tests/CMakeFiles/ipc.dir/convey.c.o
tests/ipc: tests/CMakeFiles/ipc.dir/build.make
tests/ipc: libnng.so.1.1.0
tests/ipc: tests/CMakeFiles/ipc.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ipc"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ipc.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/ipc.dir/build: tests/ipc

.PHONY : tests/CMakeFiles/ipc.dir/build

tests/CMakeFiles/ipc.dir/requires: tests/CMakeFiles/ipc.dir/ipc.c.o.requires
tests/CMakeFiles/ipc.dir/requires: tests/CMakeFiles/ipc.dir/convey.c.o.requires

.PHONY : tests/CMakeFiles/ipc.dir/requires

tests/CMakeFiles/ipc.dir/clean:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/ipc.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/ipc.dir/clean

tests/CMakeFiles/ipc.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/CMakeFiles/ipc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/ipc.dir/depend


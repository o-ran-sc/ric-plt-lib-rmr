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
include tests/CMakeFiles/nonblock.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/nonblock.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/nonblock.dir/flags.make

tests/CMakeFiles/nonblock.dir/nonblock.c.o: tests/CMakeFiles/nonblock.dir/flags.make
tests/CMakeFiles/nonblock.dir/nonblock.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/nonblock.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/nonblock.dir/nonblock.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nonblock.dir/nonblock.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/nonblock.c

tests/CMakeFiles/nonblock.dir/nonblock.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nonblock.dir/nonblock.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/nonblock.c > CMakeFiles/nonblock.dir/nonblock.c.i

tests/CMakeFiles/nonblock.dir/nonblock.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nonblock.dir/nonblock.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/nonblock.c -o CMakeFiles/nonblock.dir/nonblock.c.s

tests/CMakeFiles/nonblock.dir/nonblock.c.o.requires:

.PHONY : tests/CMakeFiles/nonblock.dir/nonblock.c.o.requires

tests/CMakeFiles/nonblock.dir/nonblock.c.o.provides: tests/CMakeFiles/nonblock.dir/nonblock.c.o.requires
	$(MAKE) -f tests/CMakeFiles/nonblock.dir/build.make tests/CMakeFiles/nonblock.dir/nonblock.c.o.provides.build
.PHONY : tests/CMakeFiles/nonblock.dir/nonblock.c.o.provides

tests/CMakeFiles/nonblock.dir/nonblock.c.o.provides.build: tests/CMakeFiles/nonblock.dir/nonblock.c.o


tests/CMakeFiles/nonblock.dir/convey.c.o: tests/CMakeFiles/nonblock.dir/flags.make
tests/CMakeFiles/nonblock.dir/convey.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/nonblock.dir/convey.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nonblock.dir/convey.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c

tests/CMakeFiles/nonblock.dir/convey.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nonblock.dir/convey.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c > CMakeFiles/nonblock.dir/convey.c.i

tests/CMakeFiles/nonblock.dir/convey.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nonblock.dir/convey.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/convey.c -o CMakeFiles/nonblock.dir/convey.c.s

tests/CMakeFiles/nonblock.dir/convey.c.o.requires:

.PHONY : tests/CMakeFiles/nonblock.dir/convey.c.o.requires

tests/CMakeFiles/nonblock.dir/convey.c.o.provides: tests/CMakeFiles/nonblock.dir/convey.c.o.requires
	$(MAKE) -f tests/CMakeFiles/nonblock.dir/build.make tests/CMakeFiles/nonblock.dir/convey.c.o.provides.build
.PHONY : tests/CMakeFiles/nonblock.dir/convey.c.o.provides

tests/CMakeFiles/nonblock.dir/convey.c.o.provides.build: tests/CMakeFiles/nonblock.dir/convey.c.o


# Object files for target nonblock
nonblock_OBJECTS = \
"CMakeFiles/nonblock.dir/nonblock.c.o" \
"CMakeFiles/nonblock.dir/convey.c.o"

# External object files for target nonblock
nonblock_EXTERNAL_OBJECTS =

tests/nonblock: tests/CMakeFiles/nonblock.dir/nonblock.c.o
tests/nonblock: tests/CMakeFiles/nonblock.dir/convey.c.o
tests/nonblock: tests/CMakeFiles/nonblock.dir/build.make
tests/nonblock: libnng.so.1.1.0
tests/nonblock: tests/CMakeFiles/nonblock.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable nonblock"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nonblock.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/nonblock.dir/build: tests/nonblock

.PHONY : tests/CMakeFiles/nonblock.dir/build

tests/CMakeFiles/nonblock.dir/requires: tests/CMakeFiles/nonblock.dir/nonblock.c.o.requires
tests/CMakeFiles/nonblock.dir/requires: tests/CMakeFiles/nonblock.dir/convey.c.o.requires

.PHONY : tests/CMakeFiles/nonblock.dir/requires

tests/CMakeFiles/nonblock.dir/clean:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/nonblock.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/nonblock.dir/clean

tests/CMakeFiles/nonblock.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/CMakeFiles/nonblock.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/nonblock.dir/depend


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
CMAKE_SOURCE_DIR = /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build

# Include any dependencies generated for this target.
include CMakeFiles/domain.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/domain.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/domain.dir/flags.make

CMakeFiles/domain.dir/tests/domain.c.o: CMakeFiles/domain.dir/flags.make
CMakeFiles/domain.dir/tests/domain.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/domain.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/domain.dir/tests/domain.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/domain.dir/tests/domain.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/domain.c

CMakeFiles/domain.dir/tests/domain.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/domain.dir/tests/domain.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/domain.c > CMakeFiles/domain.dir/tests/domain.c.i

CMakeFiles/domain.dir/tests/domain.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/domain.dir/tests/domain.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/domain.c -o CMakeFiles/domain.dir/tests/domain.c.s

CMakeFiles/domain.dir/tests/domain.c.o.requires:

.PHONY : CMakeFiles/domain.dir/tests/domain.c.o.requires

CMakeFiles/domain.dir/tests/domain.c.o.provides: CMakeFiles/domain.dir/tests/domain.c.o.requires
	$(MAKE) -f CMakeFiles/domain.dir/build.make CMakeFiles/domain.dir/tests/domain.c.o.provides.build
.PHONY : CMakeFiles/domain.dir/tests/domain.c.o.provides

CMakeFiles/domain.dir/tests/domain.c.o.provides.build: CMakeFiles/domain.dir/tests/domain.c.o


# Object files for target domain
domain_OBJECTS = \
"CMakeFiles/domain.dir/tests/domain.c.o"

# External object files for target domain
domain_EXTERNAL_OBJECTS =

domain: CMakeFiles/domain.dir/tests/domain.c.o
domain: CMakeFiles/domain.dir/build.make
domain: libnanomsg.so.5.1.0
domain: CMakeFiles/domain.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable domain"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/domain.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/domain.dir/build: domain

.PHONY : CMakeFiles/domain.dir/build

CMakeFiles/domain.dir/requires: CMakeFiles/domain.dir/tests/domain.c.o.requires

.PHONY : CMakeFiles/domain.dir/requires

CMakeFiles/domain.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/domain.dir/cmake_clean.cmake
.PHONY : CMakeFiles/domain.dir/clean

CMakeFiles/domain.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles/domain.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/domain.dir/depend


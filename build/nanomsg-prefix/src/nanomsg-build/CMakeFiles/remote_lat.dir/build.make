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
include CMakeFiles/remote_lat.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/remote_lat.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/remote_lat.dir/flags.make

CMakeFiles/remote_lat.dir/perf/remote_lat.c.o: CMakeFiles/remote_lat.dir/flags.make
CMakeFiles/remote_lat.dir/perf/remote_lat.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/perf/remote_lat.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/remote_lat.dir/perf/remote_lat.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/remote_lat.dir/perf/remote_lat.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/perf/remote_lat.c

CMakeFiles/remote_lat.dir/perf/remote_lat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/remote_lat.dir/perf/remote_lat.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/perf/remote_lat.c > CMakeFiles/remote_lat.dir/perf/remote_lat.c.i

CMakeFiles/remote_lat.dir/perf/remote_lat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/remote_lat.dir/perf/remote_lat.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/perf/remote_lat.c -o CMakeFiles/remote_lat.dir/perf/remote_lat.c.s

CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.requires:

.PHONY : CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.requires

CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.provides: CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.requires
	$(MAKE) -f CMakeFiles/remote_lat.dir/build.make CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.provides.build
.PHONY : CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.provides

CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.provides.build: CMakeFiles/remote_lat.dir/perf/remote_lat.c.o


# Object files for target remote_lat
remote_lat_OBJECTS = \
"CMakeFiles/remote_lat.dir/perf/remote_lat.c.o"

# External object files for target remote_lat
remote_lat_EXTERNAL_OBJECTS =

remote_lat: CMakeFiles/remote_lat.dir/perf/remote_lat.c.o
remote_lat: CMakeFiles/remote_lat.dir/build.make
remote_lat: libnanomsg.so.5.1.0
remote_lat: CMakeFiles/remote_lat.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable remote_lat"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/remote_lat.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/remote_lat.dir/build: remote_lat

.PHONY : CMakeFiles/remote_lat.dir/build

CMakeFiles/remote_lat.dir/requires: CMakeFiles/remote_lat.dir/perf/remote_lat.c.o.requires

.PHONY : CMakeFiles/remote_lat.dir/requires

CMakeFiles/remote_lat.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/remote_lat.dir/cmake_clean.cmake
.PHONY : CMakeFiles/remote_lat.dir/clean

CMakeFiles/remote_lat.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles/remote_lat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/remote_lat.dir/depend


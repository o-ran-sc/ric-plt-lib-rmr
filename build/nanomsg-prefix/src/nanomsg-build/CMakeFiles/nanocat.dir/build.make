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
include CMakeFiles/nanocat.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/nanocat.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/nanocat.dir/flags.make

CMakeFiles/nanocat.dir/tools/nanocat.c.o: CMakeFiles/nanocat.dir/flags.make
CMakeFiles/nanocat.dir/tools/nanocat.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/nanocat.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/nanocat.dir/tools/nanocat.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nanocat.dir/tools/nanocat.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/nanocat.c

CMakeFiles/nanocat.dir/tools/nanocat.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nanocat.dir/tools/nanocat.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/nanocat.c > CMakeFiles/nanocat.dir/tools/nanocat.c.i

CMakeFiles/nanocat.dir/tools/nanocat.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nanocat.dir/tools/nanocat.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/nanocat.c -o CMakeFiles/nanocat.dir/tools/nanocat.c.s

CMakeFiles/nanocat.dir/tools/nanocat.c.o.requires:

.PHONY : CMakeFiles/nanocat.dir/tools/nanocat.c.o.requires

CMakeFiles/nanocat.dir/tools/nanocat.c.o.provides: CMakeFiles/nanocat.dir/tools/nanocat.c.o.requires
	$(MAKE) -f CMakeFiles/nanocat.dir/build.make CMakeFiles/nanocat.dir/tools/nanocat.c.o.provides.build
.PHONY : CMakeFiles/nanocat.dir/tools/nanocat.c.o.provides

CMakeFiles/nanocat.dir/tools/nanocat.c.o.provides.build: CMakeFiles/nanocat.dir/tools/nanocat.c.o


CMakeFiles/nanocat.dir/tools/options.c.o: CMakeFiles/nanocat.dir/flags.make
CMakeFiles/nanocat.dir/tools/options.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/options.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/nanocat.dir/tools/options.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/nanocat.dir/tools/options.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/options.c

CMakeFiles/nanocat.dir/tools/options.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/nanocat.dir/tools/options.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/options.c > CMakeFiles/nanocat.dir/tools/options.c.i

CMakeFiles/nanocat.dir/tools/options.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/nanocat.dir/tools/options.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tools/options.c -o CMakeFiles/nanocat.dir/tools/options.c.s

CMakeFiles/nanocat.dir/tools/options.c.o.requires:

.PHONY : CMakeFiles/nanocat.dir/tools/options.c.o.requires

CMakeFiles/nanocat.dir/tools/options.c.o.provides: CMakeFiles/nanocat.dir/tools/options.c.o.requires
	$(MAKE) -f CMakeFiles/nanocat.dir/build.make CMakeFiles/nanocat.dir/tools/options.c.o.provides.build
.PHONY : CMakeFiles/nanocat.dir/tools/options.c.o.provides

CMakeFiles/nanocat.dir/tools/options.c.o.provides.build: CMakeFiles/nanocat.dir/tools/options.c.o


# Object files for target nanocat
nanocat_OBJECTS = \
"CMakeFiles/nanocat.dir/tools/nanocat.c.o" \
"CMakeFiles/nanocat.dir/tools/options.c.o"

# External object files for target nanocat
nanocat_EXTERNAL_OBJECTS =

nanocat: CMakeFiles/nanocat.dir/tools/nanocat.c.o
nanocat: CMakeFiles/nanocat.dir/tools/options.c.o
nanocat: CMakeFiles/nanocat.dir/build.make
nanocat: libnanomsg.so.5.1.0
nanocat: CMakeFiles/nanocat.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable nanocat"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nanocat.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/nanocat.dir/build: nanocat

.PHONY : CMakeFiles/nanocat.dir/build

CMakeFiles/nanocat.dir/requires: CMakeFiles/nanocat.dir/tools/nanocat.c.o.requires
CMakeFiles/nanocat.dir/requires: CMakeFiles/nanocat.dir/tools/options.c.o.requires

.PHONY : CMakeFiles/nanocat.dir/requires

CMakeFiles/nanocat.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/nanocat.dir/cmake_clean.cmake
.PHONY : CMakeFiles/nanocat.dir/clean

CMakeFiles/nanocat.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles/nanocat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/nanocat.dir/depend


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
include CMakeFiles/symbol.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/symbol.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/symbol.dir/flags.make

CMakeFiles/symbol.dir/tests/symbol.c.o: CMakeFiles/symbol.dir/flags.make
CMakeFiles/symbol.dir/tests/symbol.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/symbol.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/symbol.dir/tests/symbol.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/symbol.dir/tests/symbol.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/symbol.c

CMakeFiles/symbol.dir/tests/symbol.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/symbol.dir/tests/symbol.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/symbol.c > CMakeFiles/symbol.dir/tests/symbol.c.i

CMakeFiles/symbol.dir/tests/symbol.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/symbol.dir/tests/symbol.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/symbol.c -o CMakeFiles/symbol.dir/tests/symbol.c.s

CMakeFiles/symbol.dir/tests/symbol.c.o.requires:

.PHONY : CMakeFiles/symbol.dir/tests/symbol.c.o.requires

CMakeFiles/symbol.dir/tests/symbol.c.o.provides: CMakeFiles/symbol.dir/tests/symbol.c.o.requires
	$(MAKE) -f CMakeFiles/symbol.dir/build.make CMakeFiles/symbol.dir/tests/symbol.c.o.provides.build
.PHONY : CMakeFiles/symbol.dir/tests/symbol.c.o.provides

CMakeFiles/symbol.dir/tests/symbol.c.o.provides.build: CMakeFiles/symbol.dir/tests/symbol.c.o


# Object files for target symbol
symbol_OBJECTS = \
"CMakeFiles/symbol.dir/tests/symbol.c.o"

# External object files for target symbol
symbol_EXTERNAL_OBJECTS =

symbol: CMakeFiles/symbol.dir/tests/symbol.c.o
symbol: CMakeFiles/symbol.dir/build.make
symbol: libnanomsg.so.5.1.0
symbol: CMakeFiles/symbol.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable symbol"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/symbol.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/symbol.dir/build: symbol

.PHONY : CMakeFiles/symbol.dir/build

CMakeFiles/symbol.dir/requires: CMakeFiles/symbol.dir/tests/symbol.c.o.requires

.PHONY : CMakeFiles/symbol.dir/requires

CMakeFiles/symbol.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/symbol.dir/cmake_clean.cmake
.PHONY : CMakeFiles/symbol.dir/clean

CMakeFiles/symbol.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles/symbol.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/symbol.dir/depend


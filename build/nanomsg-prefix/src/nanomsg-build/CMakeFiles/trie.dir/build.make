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
include CMakeFiles/trie.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/trie.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/trie.dir/flags.make

CMakeFiles/trie.dir/tests/trie.c.o: CMakeFiles/trie.dir/flags.make
CMakeFiles/trie.dir/tests/trie.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/trie.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/trie.dir/tests/trie.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/trie.dir/tests/trie.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/trie.c

CMakeFiles/trie.dir/tests/trie.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/trie.dir/tests/trie.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/trie.c > CMakeFiles/trie.dir/tests/trie.c.i

CMakeFiles/trie.dir/tests/trie.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/trie.dir/tests/trie.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg/tests/trie.c -o CMakeFiles/trie.dir/tests/trie.c.s

CMakeFiles/trie.dir/tests/trie.c.o.requires:

.PHONY : CMakeFiles/trie.dir/tests/trie.c.o.requires

CMakeFiles/trie.dir/tests/trie.c.o.provides: CMakeFiles/trie.dir/tests/trie.c.o.requires
	$(MAKE) -f CMakeFiles/trie.dir/build.make CMakeFiles/trie.dir/tests/trie.c.o.provides.build
.PHONY : CMakeFiles/trie.dir/tests/trie.c.o.provides

CMakeFiles/trie.dir/tests/trie.c.o.provides.build: CMakeFiles/trie.dir/tests/trie.c.o


# Object files for target trie
trie_OBJECTS = \
"CMakeFiles/trie.dir/tests/trie.c.o"

# External object files for target trie
trie_EXTERNAL_OBJECTS =

trie: CMakeFiles/trie.dir/tests/trie.c.o
trie: CMakeFiles/trie.dir/build.make
trie: libnanomsg.so.5.1.0
trie: CMakeFiles/trie.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable trie"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/trie.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/trie.dir/build: trie

.PHONY : CMakeFiles/trie.dir/build

CMakeFiles/trie.dir/requires: CMakeFiles/trie.dir/tests/trie.c.o.requires

.PHONY : CMakeFiles/trie.dir/requires

CMakeFiles/trie.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/trie.dir/cmake_clean.cmake
.PHONY : CMakeFiles/trie.dir/clean

CMakeFiles/trie.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nanomsg /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/nanomsg-prefix/src/nanomsg-build/CMakeFiles/trie.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/trie.dir/depend


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
include tests/CMakeFiles/compat_pipeline.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/compat_pipeline.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/compat_pipeline.dir/flags.make

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o: tests/CMakeFiles/compat_pipeline.dir/flags.make
tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_pipeline.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_pipeline.c

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/compat_pipeline.dir/compat_pipeline.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_pipeline.c > CMakeFiles/compat_pipeline.dir/compat_pipeline.c.i

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/compat_pipeline.dir/compat_pipeline.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_pipeline.c -o CMakeFiles/compat_pipeline.dir/compat_pipeline.c.s

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.requires:

.PHONY : tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.requires

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.provides: tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.requires
	$(MAKE) -f tests/CMakeFiles/compat_pipeline.dir/build.make tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.provides.build
.PHONY : tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.provides

tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.provides.build: tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o


tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o: tests/CMakeFiles/compat_pipeline.dir/flags.make
tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o: /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_testutil.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/compat_pipeline.dir/compat_testutil.c.o   -c /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_testutil.c

tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/compat_pipeline.dir/compat_testutil.c.i"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_testutil.c > CMakeFiles/compat_pipeline.dir/compat_testutil.c.i

tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/compat_pipeline.dir/compat_testutil.c.s"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests/compat_testutil.c -o CMakeFiles/compat_pipeline.dir/compat_testutil.c.s

tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.requires:

.PHONY : tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.requires

tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.provides: tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.requires
	$(MAKE) -f tests/CMakeFiles/compat_pipeline.dir/build.make tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.provides.build
.PHONY : tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.provides

tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.provides.build: tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o


# Object files for target compat_pipeline
compat_pipeline_OBJECTS = \
"CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o" \
"CMakeFiles/compat_pipeline.dir/compat_testutil.c.o"

# External object files for target compat_pipeline
compat_pipeline_EXTERNAL_OBJECTS =

tests/compat_pipeline: tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o
tests/compat_pipeline: tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o
tests/compat_pipeline: tests/CMakeFiles/compat_pipeline.dir/build.make
tests/compat_pipeline: libnng.so.1.1.0
tests/compat_pipeline: tests/CMakeFiles/compat_pipeline.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable compat_pipeline"
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/compat_pipeline.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/compat_pipeline.dir/build: tests/compat_pipeline

.PHONY : tests/CMakeFiles/compat_pipeline.dir/build

tests/CMakeFiles/compat_pipeline.dir/requires: tests/CMakeFiles/compat_pipeline.dir/compat_pipeline.c.o.requires
tests/CMakeFiles/compat_pipeline.dir/requires: tests/CMakeFiles/compat_pipeline.dir/compat_testutil.c.o.requires

.PHONY : tests/CMakeFiles/compat_pipeline.dir/requires

tests/CMakeFiles/compat_pipeline.dir/clean:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/compat_pipeline.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/compat_pipeline.dir/clean

tests/CMakeFiles/compat_pipeline.dir/depend:
	cd /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng /home/asridharan/projects/ric-co/ric-plt/lib/rmr/ext/nng/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests /home/asridharan/projects/ric-co/ric-plt/lib/rmr/build/ext_nng-prefix/src/ext_nng-build/tests/CMakeFiles/compat_pipeline.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/compat_pipeline.dir/depend


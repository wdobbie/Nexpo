# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = "/Applications/CMake 2.8-12.app/Contents/bin/cmake"

# The command to remove a file.
RM = "/Applications/CMake 2.8-12.app/Contents/bin/cmake" -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = "/Applications/CMake 2.8-12.app/Contents/bin/ccmake"

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/will/Dropbox/gfxlib/gfxlib/glfw

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/will/Dropbox/gfxlib/gfxlib/glfw-build

# Include any dependencies generated for this target.
include examples/CMakeFiles/Boing.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/Boing.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/Boing.dir/flags.make

examples/CMakeFiles/Boing.dir/boing.c.o: examples/CMakeFiles/Boing.dir/flags.make
examples/CMakeFiles/Boing.dir/boing.c.o: /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/boing.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/CMakeFiles/Boing.dir/boing.c.o"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/Boing.dir/boing.c.o   -c /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/boing.c

examples/CMakeFiles/Boing.dir/boing.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Boing.dir/boing.c.i"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/boing.c > CMakeFiles/Boing.dir/boing.c.i

examples/CMakeFiles/Boing.dir/boing.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Boing.dir/boing.c.s"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/boing.c -o CMakeFiles/Boing.dir/boing.c.s

examples/CMakeFiles/Boing.dir/boing.c.o.requires:
.PHONY : examples/CMakeFiles/Boing.dir/boing.c.o.requires

examples/CMakeFiles/Boing.dir/boing.c.o.provides: examples/CMakeFiles/Boing.dir/boing.c.o.requires
	$(MAKE) -f examples/CMakeFiles/Boing.dir/build.make examples/CMakeFiles/Boing.dir/boing.c.o.provides.build
.PHONY : examples/CMakeFiles/Boing.dir/boing.c.o.provides

examples/CMakeFiles/Boing.dir/boing.c.o.provides.build: examples/CMakeFiles/Boing.dir/boing.c.o

# Object files for target Boing
Boing_OBJECTS = \
"CMakeFiles/Boing.dir/boing.c.o"

# External object files for target Boing
Boing_EXTERNAL_OBJECTS =

examples/Boing.app/Contents/MacOS/Boing: examples/CMakeFiles/Boing.dir/boing.c.o
examples/Boing.app/Contents/MacOS/Boing: examples/CMakeFiles/Boing.dir/build.make
examples/Boing.app/Contents/MacOS/Boing: src/libglfw3.a
examples/Boing.app/Contents/MacOS/Boing: examples/CMakeFiles/Boing.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable Boing.app/Contents/MacOS/Boing"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Boing.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/Boing.dir/build: examples/Boing.app/Contents/MacOS/Boing
.PHONY : examples/CMakeFiles/Boing.dir/build

examples/CMakeFiles/Boing.dir/requires: examples/CMakeFiles/Boing.dir/boing.c.o.requires
.PHONY : examples/CMakeFiles/Boing.dir/requires

examples/CMakeFiles/Boing.dir/clean:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && $(CMAKE_COMMAND) -P CMakeFiles/Boing.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/Boing.dir/clean

examples/CMakeFiles/Boing.dir/depend:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/will/Dropbox/gfxlib/gfxlib/glfw /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples /Users/will/Dropbox/gfxlib/gfxlib/glfw-build /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples/CMakeFiles/Boing.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/Boing.dir/depend


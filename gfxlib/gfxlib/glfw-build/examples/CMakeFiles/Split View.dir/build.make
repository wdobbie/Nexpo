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
include examples/CMakeFiles/Split\ View.dir/depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/Split\ View.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/Split\ View.dir/flags.make

examples/CMakeFiles/Split\ View.dir/splitview.c.o: examples/CMakeFiles/Split\ View.dir/flags.make
examples/CMakeFiles/Split\ View.dir/splitview.c.o: /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/splitview.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object examples/CMakeFiles/Split View.dir/splitview.c.o"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o "CMakeFiles/Split View.dir/splitview.c.o"   -c /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/splitview.c

examples/CMakeFiles/Split\ View.dir/splitview.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Split View.dir/splitview.c.i"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/splitview.c > "CMakeFiles/Split View.dir/splitview.c.i"

examples/CMakeFiles/Split\ View.dir/splitview.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Split View.dir/splitview.c.s"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples/splitview.c -o "CMakeFiles/Split View.dir/splitview.c.s"

examples/CMakeFiles/Split\ View.dir/splitview.c.o.requires:
.PHONY : examples/CMakeFiles/Split\ View.dir/splitview.c.o.requires

examples/CMakeFiles/Split\ View.dir/splitview.c.o.provides: examples/CMakeFiles/Split\ View.dir/splitview.c.o.requires
	$(MAKE) -f "examples/CMakeFiles/Split View.dir/build.make" "examples/CMakeFiles/Split View.dir/splitview.c.o.provides.build"
.PHONY : examples/CMakeFiles/Split\ View.dir/splitview.c.o.provides

examples/CMakeFiles/Split\ View.dir/splitview.c.o.provides.build: examples/CMakeFiles/Split\ View.dir/splitview.c.o

# Object files for target Split View
Split View_OBJECTS = \
"CMakeFiles/Split View.dir/splitview.c.o"

# External object files for target Split View
Split View_EXTERNAL_OBJECTS =

examples/Split\ View.app/Contents/MacOS/Split\ View: examples/CMakeFiles/Split\ View.dir/splitview.c.o
examples/Split\ View.app/Contents/MacOS/Split\ View: examples/CMakeFiles/Split\ View.dir/build.make
examples/Split\ View.app/Contents/MacOS/Split\ View: src/libglfw3.a
examples/Split\ View.app/Contents/MacOS/Split\ View: examples/CMakeFiles/Split\ View.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable \"Split View.app/Contents/MacOS/Split View\""
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && $(CMAKE_COMMAND) -E cmake_link_script "CMakeFiles/Split View.dir/link.txt" --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/Split\ View.dir/build: examples/Split\ View.app/Contents/MacOS/Split\ View
.PHONY : examples/CMakeFiles/Split\ View.dir/build

examples/CMakeFiles/Split\ View.dir/requires: examples/CMakeFiles/Split\ View.dir/splitview.c.o.requires
.PHONY : examples/CMakeFiles/Split\ View.dir/requires

examples/CMakeFiles/Split\ View.dir/clean:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples && $(CMAKE_COMMAND) -P "CMakeFiles/Split View.dir/cmake_clean.cmake"
.PHONY : examples/CMakeFiles/Split\ View.dir/clean

examples/CMakeFiles/Split\ View.dir/depend:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/will/Dropbox/gfxlib/gfxlib/glfw /Users/will/Dropbox/gfxlib/gfxlib/glfw/examples /Users/will/Dropbox/gfxlib/gfxlib/glfw-build /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples "/Users/will/Dropbox/gfxlib/gfxlib/glfw-build/examples/CMakeFiles/Split View.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : examples/CMakeFiles/Split\ View.dir/depend

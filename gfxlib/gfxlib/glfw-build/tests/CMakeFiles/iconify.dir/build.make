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
include tests/CMakeFiles/iconify.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/iconify.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/iconify.dir/flags.make

tests/CMakeFiles/iconify.dir/iconify.c.o: tests/CMakeFiles/iconify.dir/flags.make
tests/CMakeFiles/iconify.dir/iconify.c.o: /Users/will/Dropbox/gfxlib/gfxlib/glfw/tests/iconify.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/iconify.dir/iconify.c.o"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/iconify.dir/iconify.c.o   -c /Users/will/Dropbox/gfxlib/gfxlib/glfw/tests/iconify.c

tests/CMakeFiles/iconify.dir/iconify.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iconify.dir/iconify.c.i"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/will/Dropbox/gfxlib/gfxlib/glfw/tests/iconify.c > CMakeFiles/iconify.dir/iconify.c.i

tests/CMakeFiles/iconify.dir/iconify.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iconify.dir/iconify.c.s"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/will/Dropbox/gfxlib/gfxlib/glfw/tests/iconify.c -o CMakeFiles/iconify.dir/iconify.c.s

tests/CMakeFiles/iconify.dir/iconify.c.o.requires:
.PHONY : tests/CMakeFiles/iconify.dir/iconify.c.o.requires

tests/CMakeFiles/iconify.dir/iconify.c.o.provides: tests/CMakeFiles/iconify.dir/iconify.c.o.requires
	$(MAKE) -f tests/CMakeFiles/iconify.dir/build.make tests/CMakeFiles/iconify.dir/iconify.c.o.provides.build
.PHONY : tests/CMakeFiles/iconify.dir/iconify.c.o.provides

tests/CMakeFiles/iconify.dir/iconify.c.o.provides.build: tests/CMakeFiles/iconify.dir/iconify.c.o

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o: tests/CMakeFiles/iconify.dir/flags.make
tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o: /Users/will/Dropbox/gfxlib/gfxlib/glfw/deps/getopt.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/iconify.dir/__/deps/getopt.c.o   -c /Users/will/Dropbox/gfxlib/gfxlib/glfw/deps/getopt.c

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/iconify.dir/__/deps/getopt.c.i"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/will/Dropbox/gfxlib/gfxlib/glfw/deps/getopt.c > CMakeFiles/iconify.dir/__/deps/getopt.c.i

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/iconify.dir/__/deps/getopt.c.s"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/will/Dropbox/gfxlib/gfxlib/glfw/deps/getopt.c -o CMakeFiles/iconify.dir/__/deps/getopt.c.s

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.requires:
.PHONY : tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.requires

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.provides: tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.requires
	$(MAKE) -f tests/CMakeFiles/iconify.dir/build.make tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.provides.build
.PHONY : tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.provides

tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.provides.build: tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o

# Object files for target iconify
iconify_OBJECTS = \
"CMakeFiles/iconify.dir/iconify.c.o" \
"CMakeFiles/iconify.dir/__/deps/getopt.c.o"

# External object files for target iconify
iconify_EXTERNAL_OBJECTS =

tests/iconify: tests/CMakeFiles/iconify.dir/iconify.c.o
tests/iconify: tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o
tests/iconify: tests/CMakeFiles/iconify.dir/build.make
tests/iconify: src/libglfw3.a
tests/iconify: tests/CMakeFiles/iconify.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable iconify"
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iconify.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/iconify.dir/build: tests/iconify
.PHONY : tests/CMakeFiles/iconify.dir/build

tests/CMakeFiles/iconify.dir/requires: tests/CMakeFiles/iconify.dir/iconify.c.o.requires
tests/CMakeFiles/iconify.dir/requires: tests/CMakeFiles/iconify.dir/__/deps/getopt.c.o.requires
.PHONY : tests/CMakeFiles/iconify.dir/requires

tests/CMakeFiles/iconify.dir/clean:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/iconify.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/iconify.dir/clean

tests/CMakeFiles/iconify.dir/depend:
	cd /Users/will/Dropbox/gfxlib/gfxlib/glfw-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/will/Dropbox/gfxlib/gfxlib/glfw /Users/will/Dropbox/gfxlib/gfxlib/glfw/tests /Users/will/Dropbox/gfxlib/gfxlib/glfw-build /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests /Users/will/Dropbox/gfxlib/gfxlib/glfw-build/tests/CMakeFiles/iconify.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/iconify.dir/depend

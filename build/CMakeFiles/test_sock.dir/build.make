# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/aldd/project/workspace4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/aldd/project/workspace4/build

# Include any dependencies generated for this target.
include CMakeFiles/test_sock.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_sock.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_sock.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_sock.dir/flags.make

CMakeFiles/test_sock.dir/tests/test_sock.cpp.o: CMakeFiles/test_sock.dir/flags.make
CMakeFiles/test_sock.dir/tests/test_sock.cpp.o: ../tests/test_sock.cpp
CMakeFiles/test_sock.dir/tests/test_sock.cpp.o: CMakeFiles/test_sock.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aldd/project/workspace4/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_sock.dir/tests/test_sock.cpp.o"
	g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_sock.dir/tests/test_sock.cpp.o -MF CMakeFiles/test_sock.dir/tests/test_sock.cpp.o.d -o CMakeFiles/test_sock.dir/tests/test_sock.cpp.o -c /home/aldd/project/workspace4/tests/test_sock.cpp

CMakeFiles/test_sock.dir/tests/test_sock.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_sock.dir/tests/test_sock.cpp.i"
	g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aldd/project/workspace4/tests/test_sock.cpp > CMakeFiles/test_sock.dir/tests/test_sock.cpp.i

CMakeFiles/test_sock.dir/tests/test_sock.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_sock.dir/tests/test_sock.cpp.s"
	g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aldd/project/workspace4/tests/test_sock.cpp -o CMakeFiles/test_sock.dir/tests/test_sock.cpp.s

# Object files for target test_sock
test_sock_OBJECTS = \
"CMakeFiles/test_sock.dir/tests/test_sock.cpp.o"

# External object files for target test_sock
test_sock_EXTERNAL_OBJECTS =

../bin/test_sock: CMakeFiles/test_sock.dir/tests/test_sock.cpp.o
../bin/test_sock: CMakeFiles/test_sock.dir/build.make
../bin/test_sock: ../lib/libsylar.so
../bin/test_sock: /usr/local/lib/libyaml-cpp.so.0.8.0
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libboost_coroutine.so.1.74.0
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libssl.so
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libcrypto.so
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libboost_chrono.so.1.74.0
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libboost_context.so.1.74.0
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libboost_thread.so.1.74.0
../bin/test_sock: /usr/lib/aarch64-linux-gnu/libboost_atomic.so.1.74.0
../bin/test_sock: CMakeFiles/test_sock.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/aldd/project/workspace4/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test_sock"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_sock.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_sock.dir/build: ../bin/test_sock
.PHONY : CMakeFiles/test_sock.dir/build

CMakeFiles/test_sock.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_sock.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_sock.dir/clean

CMakeFiles/test_sock.dir/depend:
	cd /home/aldd/project/workspace4/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/aldd/project/workspace4 /home/aldd/project/workspace4 /home/aldd/project/workspace4/build /home/aldd/project/workspace4/build /home/aldd/project/workspace4/build/CMakeFiles/test_sock.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_sock.dir/depend


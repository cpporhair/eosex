# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.12.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.12.1/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/yangwenyu/workspace/eosex/eosio_database

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/yangwenyu/workspace/eosex/eosio_database/build

# Include any dependencies generated for this target.
include src/rpc_service/CMakeFiles/rpc_service.dir/depend.make

# Include the progress variables for this target.
include src/rpc_service/CMakeFiles/rpc_service.dir/progress.make

# Include the compile flags for this target's objects.
include src/rpc_service/CMakeFiles/rpc_service.dir/flags.make

src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.o: src/rpc_service/CMakeFiles/rpc_service.dir/flags.make
src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.o: ../src/rpc_service/rpc_service.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/yangwenyu/workspace/eosex/eosio_database/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.o"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rpc_service.dir/rpc_service.cpp.o -c /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/rpc_service.cpp

src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rpc_service.dir/rpc_service.cpp.i"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/rpc_service.cpp > CMakeFiles/rpc_service.dir/rpc_service.cpp.i

src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rpc_service.dir/rpc_service.cpp.s"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/rpc_service.cpp -o CMakeFiles/rpc_service.dir/rpc_service.cpp.s

src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.o: src/rpc_service/CMakeFiles/rpc_service.dir/flags.make
src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.o: ../src/rpc_service/service_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/yangwenyu/workspace/eosex/eosio_database/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.o"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rpc_service.dir/service_manager.cpp.o -c /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/service_manager.cpp

src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rpc_service.dir/service_manager.cpp.i"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/service_manager.cpp > CMakeFiles/rpc_service.dir/service_manager.cpp.i

src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rpc_service.dir/service_manager.cpp.s"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service/service_manager.cpp -o CMakeFiles/rpc_service.dir/service_manager.cpp.s

# Object files for target rpc_service
rpc_service_OBJECTS = \
"CMakeFiles/rpc_service.dir/rpc_service.cpp.o" \
"CMakeFiles/rpc_service.dir/service_manager.cpp.o"

# External object files for target rpc_service
rpc_service_EXTERNAL_OBJECTS =

src/rpc_service/librpc_service.a: src/rpc_service/CMakeFiles/rpc_service.dir/rpc_service.cpp.o
src/rpc_service/librpc_service.a: src/rpc_service/CMakeFiles/rpc_service.dir/service_manager.cpp.o
src/rpc_service/librpc_service.a: src/rpc_service/CMakeFiles/rpc_service.dir/build.make
src/rpc_service/librpc_service.a: src/rpc_service/CMakeFiles/rpc_service.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/yangwenyu/workspace/eosex/eosio_database/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library librpc_service.a"
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && $(CMAKE_COMMAND) -P CMakeFiles/rpc_service.dir/cmake_clean_target.cmake
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rpc_service.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/rpc_service/CMakeFiles/rpc_service.dir/build: src/rpc_service/librpc_service.a

.PHONY : src/rpc_service/CMakeFiles/rpc_service.dir/build

src/rpc_service/CMakeFiles/rpc_service.dir/clean:
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service && $(CMAKE_COMMAND) -P CMakeFiles/rpc_service.dir/cmake_clean.cmake
.PHONY : src/rpc_service/CMakeFiles/rpc_service.dir/clean

src/rpc_service/CMakeFiles/rpc_service.dir/depend:
	cd /Users/yangwenyu/workspace/eosex/eosio_database/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/yangwenyu/workspace/eosex/eosio_database /Users/yangwenyu/workspace/eosex/eosio_database/src/rpc_service /Users/yangwenyu/workspace/eosex/eosio_database/build /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service /Users/yangwenyu/workspace/eosex/eosio_database/build/src/rpc_service/CMakeFiles/rpc_service.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/rpc_service/CMakeFiles/rpc_service.dir/depend

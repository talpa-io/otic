# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

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
CMAKE_COMMAND = /home/talpaadmin/Softwares/clion-2019.1.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/talpaadmin/Softwares/clion-2019.1.4/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/talpaadmin/Dokumente/GIT/oticRev

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/talpaadmin/Dokumente/GIT/oticRev/build

# Include any dependencies generated for this target.
include tests/CMakeFiles/pack_unpack_test.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/pack_unpack_test.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/pack_unpack_test.dir/flags.make

tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o: tests/CMakeFiles/pack_unpack_test.dir/flags.make
tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o: ../tests/pack_unpack/pack_unpack.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o   -c /home/talpaadmin/Dokumente/GIT/oticRev/tests/pack_unpack/pack_unpack.c

tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.i"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/talpaadmin/Dokumente/GIT/oticRev/tests/pack_unpack/pack_unpack.c > CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.i

tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.s"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/talpaadmin/Dokumente/GIT/oticRev/tests/pack_unpack/pack_unpack.c -o CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.s

tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o: tests/CMakeFiles/pack_unpack_test.dir/flags.make
tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o: ../include/errHand/errHand.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o   -c /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c

tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.i"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c > CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.i

tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.s"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c -o CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.s

# Object files for target pack_unpack_test
pack_unpack_test_OBJECTS = \
"CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o" \
"CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o"

# External object files for target pack_unpack_test
pack_unpack_test_EXTERNAL_OBJECTS =

../bin/pack_unpack_test: tests/CMakeFiles/pack_unpack_test.dir/pack_unpack/pack_unpack.c.o
../bin/pack_unpack_test: tests/CMakeFiles/pack_unpack_test.dir/__/include/errHand/errHand.c.o
../bin/pack_unpack_test: tests/CMakeFiles/pack_unpack_test.dir/build.make
../bin/pack_unpack_test: ../lib/libotic.so
../bin/pack_unpack_test: tests/CMakeFiles/pack_unpack_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ../../bin/pack_unpack_test"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pack_unpack_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/pack_unpack_test.dir/build: ../bin/pack_unpack_test

.PHONY : tests/CMakeFiles/pack_unpack_test.dir/build

tests/CMakeFiles/pack_unpack_test.dir/clean:
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/tests && $(CMAKE_COMMAND) -P CMakeFiles/pack_unpack_test.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/pack_unpack_test.dir/clean

tests/CMakeFiles/pack_unpack_test.dir/depend:
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/talpaadmin/Dokumente/GIT/oticRev /home/talpaadmin/Dokumente/GIT/oticRev/tests /home/talpaadmin/Dokumente/GIT/oticRev/build /home/talpaadmin/Dokumente/GIT/oticRev/build/tests /home/talpaadmin/Dokumente/GIT/oticRev/build/tests/CMakeFiles/pack_unpack_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/pack_unpack_test.dir/depend

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
include demo/CMakeFiles/pack_demo.dir/depend.make

# Include the progress variables for this target.
include demo/CMakeFiles/pack_demo.dir/progress.make

# Include the compile flags for this target's objects.
include demo/CMakeFiles/pack_demo.dir/flags.make

demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o: demo/CMakeFiles/pack_demo.dir/flags.make
demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o: ../demo/pack_demo/pack_demo.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o   -c /home/talpaadmin/Dokumente/GIT/oticRev/demo/pack_demo/pack_demo.c

demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.i"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/talpaadmin/Dokumente/GIT/oticRev/demo/pack_demo/pack_demo.c > CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.i

demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.s"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/talpaadmin/Dokumente/GIT/oticRev/demo/pack_demo/pack_demo.c -o CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.s

demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o: demo/CMakeFiles/pack_demo.dir/flags.make
demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o: ../include/errHand/errHand.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o   -c /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c

demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.i"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c > CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.i

demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.s"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/talpaadmin/Dokumente/GIT/oticRev/include/errHand/errHand.c -o CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.s

# Object files for target pack_demo
pack_demo_OBJECTS = \
"CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o" \
"CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o"

# External object files for target pack_demo
pack_demo_EXTERNAL_OBJECTS =

../bin/pack_demo: demo/CMakeFiles/pack_demo.dir/pack_demo/pack_demo.c.o
../bin/pack_demo: demo/CMakeFiles/pack_demo.dir/__/include/errHand/errHand.c.o
../bin/pack_demo: demo/CMakeFiles/pack_demo.dir/build.make
../bin/pack_demo: ../lib/libotic.so
../bin/pack_demo: demo/CMakeFiles/pack_demo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/talpaadmin/Dokumente/GIT/oticRev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ../../bin/pack_demo"
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pack_demo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
demo/CMakeFiles/pack_demo.dir/build: ../bin/pack_demo

.PHONY : demo/CMakeFiles/pack_demo.dir/build

demo/CMakeFiles/pack_demo.dir/clean:
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build/demo && $(CMAKE_COMMAND) -P CMakeFiles/pack_demo.dir/cmake_clean.cmake
.PHONY : demo/CMakeFiles/pack_demo.dir/clean

demo/CMakeFiles/pack_demo.dir/depend:
	cd /home/talpaadmin/Dokumente/GIT/oticRev/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/talpaadmin/Dokumente/GIT/oticRev /home/talpaadmin/Dokumente/GIT/oticRev/demo /home/talpaadmin/Dokumente/GIT/oticRev/build /home/talpaadmin/Dokumente/GIT/oticRev/build/demo /home/talpaadmin/Dokumente/GIT/oticRev/build/demo/CMakeFiles/pack_demo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : demo/CMakeFiles/pack_demo.dir/depend

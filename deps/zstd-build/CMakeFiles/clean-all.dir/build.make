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
CMAKE_SOURCE_DIR = /home/talpaadmin/Dokumente/GIT/otic

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/talpaadmin/Dokumente/GIT/otic/build

# Utility rule file for clean-all.

# Include the progress variables for this target.
include ../deps/zstd-build/CMakeFiles/clean-all.dir/progress.make

../deps/zstd-build/CMakeFiles/clean-all:
	cd /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-build && /usr/bin/make clean
	cd /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-build && rm -rf /home/talpaadmin/Dokumente/GIT/otic/build/

clean-all: ../deps/zstd-build/CMakeFiles/clean-all
clean-all: ../deps/zstd-build/CMakeFiles/clean-all.dir/build.make

.PHONY : clean-all

# Rule to build all files generated by this target.
../deps/zstd-build/CMakeFiles/clean-all.dir/build: clean-all

.PHONY : ../deps/zstd-build/CMakeFiles/clean-all.dir/build

../deps/zstd-build/CMakeFiles/clean-all.dir/clean:
	cd /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-build && $(CMAKE_COMMAND) -P CMakeFiles/clean-all.dir/cmake_clean.cmake
.PHONY : ../deps/zstd-build/CMakeFiles/clean-all.dir/clean

../deps/zstd-build/CMakeFiles/clean-all.dir/depend:
	cd /home/talpaadmin/Dokumente/GIT/otic/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/talpaadmin/Dokumente/GIT/otic /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-src/build/cmake /home/talpaadmin/Dokumente/GIT/otic/build /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-build /home/talpaadmin/Dokumente/GIT/otic/deps/zstd-build/CMakeFiles/clean-all.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : ../deps/zstd-build/CMakeFiles/clean-all.dir/depend


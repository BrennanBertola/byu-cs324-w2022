# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.19

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


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.2.2\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.2.2\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/lab_shell_pipeline.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lab_shell_pipeline.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lab_shell_pipeline.dir/flags.make

CMakeFiles/lab_shell_pipeline.dir/tsh.c.obj: CMakeFiles/lab_shell_pipeline.dir/flags.make
CMakeFiles/lab_shell_pipeline.dir/tsh.c.obj: ../tsh.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/lab_shell_pipeline.dir/tsh.c.obj"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\lab_shell_pipeline.dir\tsh.c.obj -c C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\tsh.c

CMakeFiles/lab_shell_pipeline.dir/tsh.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lab_shell_pipeline.dir/tsh.c.i"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\tsh.c > CMakeFiles\lab_shell_pipeline.dir\tsh.c.i

CMakeFiles/lab_shell_pipeline.dir/tsh.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lab_shell_pipeline.dir/tsh.c.s"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\tsh.c -o CMakeFiles\lab_shell_pipeline.dir\tsh.c.s

# Object files for target lab_shell_pipeline
lab_shell_pipeline_OBJECTS = \
"CMakeFiles/lab_shell_pipeline.dir/tsh.c.obj"

# External object files for target lab_shell_pipeline
lab_shell_pipeline_EXTERNAL_OBJECTS =

lab_shell_pipeline.exe: CMakeFiles/lab_shell_pipeline.dir/tsh.c.obj
lab_shell_pipeline.exe: CMakeFiles/lab_shell_pipeline.dir/build.make
lab_shell_pipeline.exe: CMakeFiles/lab_shell_pipeline.dir/linklibs.rsp
lab_shell_pipeline.exe: CMakeFiles/lab_shell_pipeline.dir/objects1.rsp
lab_shell_pipeline.exe: CMakeFiles/lab_shell_pipeline.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable lab_shell_pipeline.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\lab_shell_pipeline.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lab_shell_pipeline.dir/build: lab_shell_pipeline.exe

.PHONY : CMakeFiles/lab_shell_pipeline.dir/build

CMakeFiles/lab_shell_pipeline.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\lab_shell_pipeline.dir\cmake_clean.cmake
.PHONY : CMakeFiles/lab_shell_pipeline.dir/clean

CMakeFiles/lab_shell_pipeline.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug C:\Users\brenn\cs324\byu-cs324-w2022\lab-shell-pipeline\cmake-build-debug\CMakeFiles\lab_shell_pipeline.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lab_shell_pipeline.dir/depend


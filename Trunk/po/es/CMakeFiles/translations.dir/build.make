# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.6

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jrios/Projects/guayadeque

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jrios/Projects/guayadeque

# Utility rule file for translations.

po/es/CMakeFiles/translations: po/es/guayadeque.mo

po/es/guayadeque.mo: po/es/guayadeque.mo
po/es/guayadeque.mo: po/es/guayadeque.po
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jrios/Projects/guayadeque/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating guayadeque.mo"
	cd /home/jrios/Projects/guayadeque/po/es && /usr/bin/msgfmt -o /home/jrios/Projects/guayadeque/po/es/guayadeque.mo /home/jrios/Projects/guayadeque/po/es/guayadeque.po

translations: po/es/CMakeFiles/translations
translations: po/es/guayadeque.mo
translations: po/es/CMakeFiles/translations.dir/build.make
.PHONY : translations

# Rule to build all files generated by this target.
po/es/CMakeFiles/translations.dir/build: translations
.PHONY : po/es/CMakeFiles/translations.dir/build

po/es/CMakeFiles/translations.dir/clean:
	cd /home/jrios/Projects/guayadeque/po/es && $(CMAKE_COMMAND) -P CMakeFiles/translations.dir/cmake_clean.cmake
.PHONY : po/es/CMakeFiles/translations.dir/clean

po/es/CMakeFiles/translations.dir/depend:
	cd /home/jrios/Projects/guayadeque && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jrios/Projects/guayadeque /home/jrios/Projects/guayadeque/po/es /home/jrios/Projects/guayadeque /home/jrios/Projects/guayadeque/po/es /home/jrios/Projects/guayadeque/po/es/CMakeFiles/translations.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : po/es/CMakeFiles/translations.dir/depend


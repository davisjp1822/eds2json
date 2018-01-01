# At LEAST 3.5 but newer is better
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(eds2json VERSION 1.0 LANGUAGES C CXX)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

# Targets that we develop here
add_subdirectory(libeds)
add_subdirectory(examples)
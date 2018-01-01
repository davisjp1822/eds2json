# Define library. Only source files here!
project(libeds2json VERSION 1.0 LANGUAGES C CXX)

add_library(eds2json
    SHARED
    src/libeds.c)

set_target_properties(eds2json PROPERTIES SOVERSION 1)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(eds2json
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

# If we have compiler requirements for this library, list them
# here
target_compile_features(eds2json
    PUBLIC c_function_prototypes
    c_restrict
    c_static_assert
    c_variadic_macros)

set_target_properties(eds2json PROPERTIES COMPILE_FLAGS "-Wextra -Wall -Wpedantic")

# 'make install' to the correct locations (provided by GNUInstallDirs).
install(TARGETS eds2json EXPORT libeds2jsonConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT libeds2jsonConfig DESTINATION share/libeds2json/cmake)

# This makes the project importable from the build directory
export(TARGETS eds2json FILE libeds2jsonConfig.cmake)

# install GoogleTest so that we can unit test our code
# We need thread support
find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# Create a libgmock target to be used as a dependency by test programs
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# Set libgmock properties
set_target_properties(libgmock PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# I couldn't make it work with INTERFACE_INCLUDE_DIRECTORIES
include_directories("${source_dir}/googletest/include"
                    "${source_dir}/googlemock/include")

# finally, execute the tests
add_executable(libeds2json_tests
    test/libeds_tests.cpp)

target_link_libraries(libeds2json_tests
    eds2json
    libgtest
    libgmock)

add_test(libeds2json_tests eds2json_tests)
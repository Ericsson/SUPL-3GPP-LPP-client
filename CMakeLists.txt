cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

project(example LANGUAGES C CXX)

include(cmake/color.cmake)
include(cmake/vars.cmake)
include(cmake/compiler.cmake)
include(cmake/target.cmake)

add_definitions(-D_POSIX_C_SOURCE=200809L)
add_definitions(-DCLIENT_VERSION="4.0.14")
add_definitions(-DCLIENT_VERSION_INT=0x04000E)

find_package(Threads REQUIRED)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG=1)
endif()

if(${DISABLE_STRERRORNAME_NP})
    add_definitions(-DDISABLE_STRERRORNAME_NP=1)
endif()

add_subdirectory("dependency")
add_subdirectory("external")

if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

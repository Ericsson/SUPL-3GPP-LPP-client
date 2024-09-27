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
add_definitions(-DCLIENT_VERSION="3.4.11")
add_definitions(-DCLIENT_VERSION_INT=0x030411)

find_package(Threads REQUIRED)

add_subdirectory("libs")

add_subdirectory("dependency")
add_subdirectory("external")

if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

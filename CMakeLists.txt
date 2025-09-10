cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

if(CMAKE_CXX_STANDARD EQUAL 11)
    set(USE_CXX11 ON)
    add_definitions(-DUSE_CXX11_COMPAT=1)
else()
    set(USE_CXX11 OFF)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(example LANGUAGES C CXX)

include(cmake/color.cmake)
include(cmake/vars.cmake)
include(cmake/compiler.cmake)
include(cmake/target.cmake)

add_definitions(-D_POSIX_C_SOURCE=200809L)
add_definitions(-DCLIENT_VERSION="4.0.22")
add_definitions(-DCLIENT_VERSION_INT=0x040016)

find_package(Threads REQUIRED)

if(USE_CPM)
    include(cmake/CPM.cmake)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG=1)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_definitions(-DCOMPILER_GCC=1)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-DCOMPILER_CLANG=1)
endif()

if(DISABLE_STRERRORNAME_NP)
    add_definitions(-DDISABLE_STRERRORNAME_NP=1)
endif()

add_subdirectory("dependency")
add_subdirectory("external")

if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

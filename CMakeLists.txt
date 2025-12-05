cmake_minimum_required(VERSION 3.14)

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

set(CLIENT_VERSION "4.0.22")
set(CLIENT_VERSION_INT "0x040016")

include(cmake/clang-tidy.cmake)
include(cmake/cppcheck.cmake)
include(cmake/iwyu.cmake)
include(cmake/color.cmake)
include(cmake/vars.cmake)
include(cmake/compiler.cmake)
include(cmake/target.cmake)
include(cmake/git-version.cmake)
include(cmake/CPM.cmake)
include(external/eigen3.cmake)
include(external/args.cmake)
include(cmake/splice.cmake)
include(cmake/timetrace.cmake)

include_directories(${CMAKE_BINARY_DIR}/generated)

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

if(NOT INCLUDE_GENERATOR_RTCM)
    if(INCLUDE_FORMAT_RINEX)
        message(WARNING "RINEX format requires RTCM generator to be enabled")
        set(INCLUDE_FORMAT_RINEX OFF)
    endif()
    if(INCLUDE_FORMAT_ANTEX)
        message(WARNING "ANTEX format requires RTCM generator to be enabled")
        set(INCLUDE_FORMAT_ANTEX OFF)
    endif()
endif()


add_subdirectory("dependency")
add_subdirectory("external")

if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory("tests")
endif()

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

project(example LANGUAGES C CXX)

option(USE_OPENSSL "USE_OPENSSL" ON)
option(USE_ASAN "USE_ASAN" OFF)

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (FORCE_COLORED_OUTPUT)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif (FORCE_COLORED_OUTPUT)

if (USE_OPENSSL)
find_package(OpenSSL REQUIRED)
endif (USE_OPENSSL)

find_package(Threads REQUIRED)

add_compile_options("-Wall" "-Wno-unused-function" "-Wno-unused-variable" "-Wno-sign-compare")

add_subdirectory("libs")
add_subdirectory("src")

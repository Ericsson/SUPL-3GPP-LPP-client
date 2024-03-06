cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

project(example LANGUAGES C CXX)

option(USE_OPENSSL "USE_OPENSSL" ON)
option(USE_ASAN "USE_ASAN" OFF)
option(ASN_DEBUG "ASN_DEBUG" OFF)
option(INTERFACE_FD_DEBUG "Print FD debug information" OFF)
option(RECEIVER_UBLOX_THREADED "Print Receiver u-blox (threaded) debug information" OFF)
option(SPARTN_DEBUG_PRINT "Print SPARTN debug information" OFF)
option(RECEIVER_NMEA_DEBUG "Print Receiver NMEA debug information" OFF)

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

add_definitions(-D_POSIX_C_SOURCE=200809L)
add_definitions(-DCLIENT_VERSION="3.4.4")
add_definitions(-DCLIENT_VERSION_INT=0x030404)

if(${ASN_DEBUG})
   add_definitions(-DASN_EMIT_DEBUG=1)
endif(${ASN_DEBUG})

find_package(Threads REQUIRED)

add_compile_options("-Wall" "-Wno-unused-function" "-Wno-unused-variable" "-Wno-sign-compare")

add_subdirectory("libs")
add_subdirectory("interface")
add_subdirectory("asn.1")
add_subdirectory("generator/rtcm")
add_subdirectory("generator/spartn")
add_subdirectory("generator/spartn2")
add_subdirectory("receiver/ublox")
add_subdirectory("receiver/nmea")
add_subdirectory("examples/ublox")
add_subdirectory("examples/nmea")
add_subdirectory("examples/lpp")
add_subdirectory("examples/lpp2spartn")
add_subdirectory("examples/ntrip")


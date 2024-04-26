cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

project(example LANGUAGES C CXX)

option(USE_OPENSSL "USE_OPENSSL" OFF)
option(USE_ASAN "USE_ASAN" OFF)
option(ASN_DEBUG "ASN_DEBUG" OFF)
option(INTERFACE_FD_DEBUG "Print FD debug information" OFF)
option(RECEIVER_UBLOX_THREADED "Print Receiver u-blox (threaded) debug information" OFF)
option(SPARTN_DEBUG_PRINT "Print SPARTN debug information" OFF)
option(RECEIVER_NMEA_DEBUG "Print Receiver NMEA debug information" OFF)

option(INCLUDE_GENERATOR_RTCM "Include RTCM generator" ON)
option(INCLUDE_GENERATOR_SPARTN "Include SPARTN generator" ON)
option(INCLUDE_GENERATOR_SPARTN_OLD "Include old SPARTN generator" OFF)

option(BUILD_WITH_ALL_WARNINGS "BUILD_WITH_ALL_WARNINGS" OFF)
option(WARNINGS_AS_ERRORS "WARNINGS_AS_ERRORS" OFF)
option(EXPERIMENTAL "EXPERIMENTAL" OFF)

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
add_definitions(-DCLIENT_VERSION="3.4.8")
add_definitions(-DCLIENT_VERSION_INT=0x030408)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_definitions(-DCOMPILER_CANNOT_DEDUCE_UNREACHABLE=1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_definitions(-DCOMPILER_CANNOT_DEDUCE_UNREACHABLE=0)
endif()

if(${ASN_DEBUG})
   add_definitions(-DASN_EMIT_DEBUG=1)
endif(${ASN_DEBUG})

find_package(Threads REQUIRED)

function(setup_target target)
    target_compile_options(${target} PRIVATE
        "-Wall"
        "-Wextra"
        "-Wpedantic"
    )

    if(${WARNINGS_AS_ERRORS})
        target_compile_options(${target} PRIVATE
            "-Werror"
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(${BUILD_WITH_ALL_WARNINGS})
            target_compile_options(${target} PRIVATE
                "-Weverything"
                "-Wno-c++98-compat"
                "-Wno-c++98-compat-pedantic"
                "-Wno-c++98-c++11-compat-pedantic"
                "-Wno-nested-anon-types"
                "-Wno-gnu-anonymous-struct"
                "-Wno-missing-prototypes"
                "-Wno-documentation"
                "-Wno-documentation-unknown-command"
                "-Wno-weak-vtables"
                "-Wno-unused-const-variable"
                "-Wno-format-nonliteral"
                "-Wno-global-constructors"
                "-Wno-exit-time-destructors"
                "-Wno-padded"
            )
        endif()

        target_compile_options(${target} PRIVATE
            "-Wno-c++17-extensions" 
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE
            "-Wno-missing-field-initializers"
            "-Wno-error=pragmas"
            "-Wno-pragmas"
        )
    endif()

    if (USE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined,leak)
        target_link_libraries(${target} PRIVATE -fsanitize=address,undefined,leak)
    endif (USE_ASAN)
endfunction()


add_subdirectory("libs")
add_subdirectory("asn.1")
add_subdirectory("interface")
add_subdirectory("receiver")
add_subdirectory("generator")
add_subdirectory("examples")



add_library(receiver_nmea STATIC 
    "message.cpp"
    "parser.cpp"
    "receiver.cpp"
    "threaded_receiver.cpp"
    "gga.cpp"
    "vtg.cpp"
    "gst.cpp"
)
add_library(receiver::nmea ALIAS receiver_nmea)

target_include_directories(receiver_nmea PRIVATE "./" "include/receiver/nmea/")
target_include_directories(receiver_nmea PUBLIC "include/")
target_link_libraries(receiver_nmea PRIVATE dependency::interface)
target_link_libraries(receiver_nmea PRIVATE utility)

if (USE_ASAN)
target_compile_options(receiver_nmea PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(receiver_nmea PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

target_compile_options(receiver_nmea PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(receiver_nmea PRIVATE
        "-Weverything"
        "-Wno-switch-enum"
        "-Wno-signed-enum-bitfield"
        "-Wno-deprecated-register"
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
        "-Wno-unsafe-buffer-usage"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(receiver_nmea PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()

if(${RECEIVER_NMEA_THREADED})
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_THREADED=1)
else()
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_THREADED=0)
endif()

if(${RECEIVER_NMEA_DEBUG})
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_DEBUG=1)
else()
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_DEBUG=0)
endif()

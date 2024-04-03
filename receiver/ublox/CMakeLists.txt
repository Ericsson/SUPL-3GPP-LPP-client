
add_library(receiver_ublox STATIC 
    "encoder.cpp"
    "decoder.cpp"
    "parser.cpp"
    "message.cpp"
    "receiver.cpp"
    "threaded_receiver.cpp"
    "ubx_nav_pvt.cpp"
    "ubx_mon_ver.cpp"
    "ubx_cfg.cpp"
    "ubx_cfg_valget.cpp"
    "ubx_cfg_valset.cpp"
    "ubx_ack_ack.cpp"
    "ubx_ack_nak.cpp"
)
add_library(receiver::ublox ALIAS receiver_ublox)

target_include_directories(receiver_ublox PRIVATE "./" "include/receiver/ublox/")
target_include_directories(receiver_ublox PUBLIC "include/")
target_link_libraries(receiver_ublox PRIVATE dependency::interface)
target_link_libraries(receiver_ublox PRIVATE utility)

if (USE_ASAN)
target_compile_options(receiver_ublox PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(receiver_ublox PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

target_compile_options(receiver_ublox PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(receiver_ublox PRIVATE
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
    target_compile_options(receiver_ublox PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()

if(${RECEIVER_UBLOX_THREADED})
    target_compile_definitions(receiver_ublox PUBLIC RECEIVER_UBLOX_THREADED=1)
endif()

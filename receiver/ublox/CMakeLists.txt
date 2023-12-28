
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
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wcast-align"
    "-Woverloaded-virtual"
    "-Wsign-conversion"
    "-Wno-conversion"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(receiver_ublox PRIVATE
        "-Wmisleading-indentation"
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

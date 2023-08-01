
add_library(receiver_ublox STATIC 
    "encoder.cpp"
    "decoder.cpp"
    "parser.cpp"
    "message.cpp"
    "receiver.cpp"
    "ubx_nav_pvt.cpp"
    "ubx_mon_ver.cpp"
)
add_library(receiver::ublox ALIAS receiver_ublox)

target_include_directories(receiver_ublox PRIVATE "./" "include/receiver/ublox/")
target_include_directories(receiver_ublox PUBLIC "include/")
target_link_libraries(receiver_ublox PRIVATE dependency::interface)

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
    "-Wmisleading-indentation"
    "-Wduplicated-cond"
    "-Wduplicated-branches"
)

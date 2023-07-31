
add_library(receiver_ublox STATIC 
    "decoder.cpp"
    "parser.cpp"
    "message.cpp"
    "ubx_nav_pvt.cpp"
)
add_library(receiver::ublox ALIAS receiver_ublox)

target_include_directories(receiver_ublox PRIVATE "./" "include/receiver/ublox/")
target_include_directories(receiver_ublox PUBLIC "include/")

if (USE_ASAN)
target_compile_options(receiver_ublox PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(receiver_ublox PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)


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
    "ubx_rxm_rtcm.cpp"
    "ubx_rxm_spartn.cpp"
    "ubx_rxm_rawx.cpp"
)
add_library(receiver::ublox ALIAS receiver_ublox)

target_include_directories(receiver_ublox PRIVATE "./" "include/receiver/ublox/")
target_include_directories(receiver_ublox PUBLIC "include/")
target_link_libraries(receiver_ublox PRIVATE dependency::interface)
target_link_libraries(receiver_ublox PRIVATE utility)

setup_target(receiver_ublox)

if(${RECEIVER_UBLOX_THREADED})
    target_compile_definitions(receiver_ublox PUBLIC RECEIVER_UBLOX_THREADED=1)
endif()

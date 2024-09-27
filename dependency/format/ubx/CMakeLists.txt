
add_library(dependency_format_ubx STATIC 
    "encoder.cpp"
    "decoder.cpp"
    "parser.cpp"
    "message.cpp"
    "cfg.cpp"
    "messages/nav_pvt.cpp"
    "messages/mon_ver.cpp"
    "messages/cfg_valget.cpp"
    "messages/cfg_valset.cpp"
    "messages/ack_ack.cpp"
    "messages/ack_nak.cpp"
    "messages/rxm_rtcm.cpp"
    "messages/rxm_spartn.cpp"
    "messages/rxm_sfrbx.cpp"
    "messages/rxm_rawx.cpp"
)
add_library(dependency::format::ubx ALIAS dependency_format_ubx)

target_include_directories(dependency_format_ubx PRIVATE "./" "include/format/ubx/")
target_include_directories(dependency_format_ubx PUBLIC "include/")
target_link_libraries(dependency_format_ubx PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_ubx PUBLIC dependency::loglet)
target_link_libraries(dependency_format_ubx PUBLIC dependency::time)
target_link_libraries(dependency_format_ubx PUBLIC dependency::core)

setup_target(dependency_format_ubx)

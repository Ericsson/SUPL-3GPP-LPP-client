
add_library(receiver_nmea STATIC 
    "message.cpp"
    "parser.cpp"
    "receiver.cpp"
    "threaded_receiver.cpp"
    "gga.cpp"
    "vtg.cpp"
    "gst.cpp"
    "epe.cpp"
)
add_library(receiver::nmea ALIAS receiver_nmea)

target_include_directories(receiver_nmea PRIVATE "./" "include/receiver/nmea/")
target_include_directories(receiver_nmea PUBLIC "include/")
target_link_libraries(receiver_nmea PRIVATE dependency::interface)
target_link_libraries(receiver_nmea PRIVATE utility)

if(${RECEIVER_NMEA_THREADED})
target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_THREADED=1)
endif()

if(${RECEIVER_NMEA_DEBUG})
target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_DEBUG=1)
endif()

setup_target(receiver_nmea)

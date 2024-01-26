
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
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wcast-align"
    "-Woverloaded-virtual"
    "-Wsign-conversion"
    "-Wno-conversion"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(receiver_nmea PRIVATE
        "-Wmisleading-indentation"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(receiver_nmea PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()

if(${RECEIVER_NMEA_THREADED})
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_THREADED=1)
endif()

if(${RECEIVER_NMEA_DEBUG})
    target_compile_definitions(receiver_nmea PUBLIC RECEIVER_NMEA_DEBUG=1)
endif()

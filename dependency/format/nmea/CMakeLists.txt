
add_library(dependency_format_nmea STATIC 
    "message.cpp"
    "parser.cpp"
    "messages/gga.cpp"
    "messages/vtg.cpp"
    "messages/gst.cpp"
)
add_library(dependency::format::nmea ALIAS dependency_format_nmea)

target_include_directories(dependency_format_nmea PRIVATE "./" "include/format/nmea/")
target_include_directories(dependency_format_nmea PUBLIC "include/")
target_link_libraries(dependency_format_nmea PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_nmea PUBLIC dependency::loglet)
target_link_libraries(dependency_format_nmea PUBLIC dependency::time)
target_link_libraries(dependency_format_nmea PUBLIC dependency::core)

setup_target(dependency_format_nmea)

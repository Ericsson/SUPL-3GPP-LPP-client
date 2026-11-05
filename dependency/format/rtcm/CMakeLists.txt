
add_library(dependency_format_rtcm STATIC 
    "message.cpp"
    "parser.cpp"
    "datafields.cpp"
    "messages/1019.cpp"
    "messages/1042.cpp"
    "messages/1046.cpp"
    "messages/ephemeris.cpp"
)
add_library(dependency::format::rtcm ALIAS dependency_format_rtcm)

target_include_directories(dependency_format_rtcm PRIVATE "./" "include/format/rtcm/")
target_include_directories(dependency_format_rtcm PUBLIC "include/")
target_link_libraries(dependency_format_rtcm PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::loglet)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::time)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::core)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::ephemeris)

setup_target(dependency_format_rtcm)

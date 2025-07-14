
add_library(dependency_format_rtcm STATIC 
    "message.cpp"
    "parser.cpp"
    "messages/1019.cpp"
)
add_library(dependency::format::rtcm ALIAS dependency_format_rtcm)

target_include_directories(dependency_format_rtcm PRIVATE "./" "include/format/rtcm/")
target_include_directories(dependency_format_rtcm PUBLIC "include/")
target_link_libraries(dependency_format_rtcm PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::loglet)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::time)
target_link_libraries(dependency_format_rtcm PUBLIC dependency::core)

setup_target(dependency_format_rtcm)

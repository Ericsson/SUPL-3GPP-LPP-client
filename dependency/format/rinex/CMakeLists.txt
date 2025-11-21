
add_library(dependency_format_rinex STATIC 
    "builder.cpp"
)
add_library(dependency::format::rinex ALIAS dependency_format_rinex)

target_include_directories(dependency_format_rinex PRIVATE "./" "include/format/rinex/")
target_include_directories(dependency_format_rinex PUBLIC "include/")
target_link_libraries(dependency_format_rinex PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_rinex PUBLIC dependency::loglet)
target_link_libraries(dependency_format_rinex PUBLIC dependency::time)
target_link_libraries(dependency_format_rinex PUBLIC dependency::core)
target_link_libraries(dependency_format_rinex PUBLIC dependency::maths)
target_link_libraries(dependency_format_rinex PUBLIC dependency::generator::rtcm)

target_compile_definitions(dependency_format_rinex PUBLIC "INCLUDE_FORMAT_RINEX=1")

setup_target(dependency_format_rinex)

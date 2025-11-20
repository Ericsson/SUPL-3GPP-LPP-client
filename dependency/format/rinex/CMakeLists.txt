
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

if(INCLUDE_GENERATOR_RTCM)
    target_compile_definitions(dependency_format_rinex PRIVATE "INCLUDE_GENERATOR_RTCM=1")
    target_link_libraries(dependency_format_rinex PUBLIC dependency::generator::rtcm)
else()
    message(FATAL_ERROR "dependency_format_rinex requires INCLUDE_GENERATOR_RTCM=ON")
endif()

setup_target(dependency_format_rinex)

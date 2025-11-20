
add_library(dependency_format_antex STATIC 
    "antex.cpp"
)
add_library(dependency::format::antex ALIAS dependency_format_antex)

target_include_directories(dependency_format_antex PRIVATE "./" "include/format/antex/")
target_include_directories(dependency_format_antex PUBLIC "include/")
target_link_libraries(dependency_format_antex PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_antex PUBLIC dependency::loglet)
target_link_libraries(dependency_format_antex PUBLIC dependency::time)
target_link_libraries(dependency_format_antex PUBLIC dependency::core)
target_link_libraries(dependency_format_antex PUBLIC dependency::maths)

if(INCLUDE_GENERATOR_RTCM)
    target_compile_definitions(dependency_format_antex PRIVATE "INCLUDE_GENERATOR_RTCM=1")
    target_link_libraries(dependency_format_antex PUBLIC dependency::generator::rtcm)
else()
    message(FATAL_ERROR "dependency_format_antex requires INCLUDE_GENERATOR_RTCM=ON")
endif()

setup_target(dependency_format_antex)

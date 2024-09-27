
add_library(dependency_format_lpp STATIC 
    "uper_parser.cpp"
)
add_library(dependency::format::lpp ALIAS dependency_format_lpp)

target_include_directories(dependency_format_lpp PRIVATE "./" "include/format/lpp/")
target_include_directories(dependency_format_lpp PUBLIC "include/")
target_link_libraries(dependency_format_lpp PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_lpp PUBLIC dependency::loglet)
target_link_libraries(dependency_format_lpp PUBLIC dependency::time)
target_link_libraries(dependency_format_lpp PUBLIC dependency::core)
target_link_libraries(dependency_format_lpp PUBLIC asn1::generated::lpp asn1::helper)

setup_target(dependency_format_lpp)

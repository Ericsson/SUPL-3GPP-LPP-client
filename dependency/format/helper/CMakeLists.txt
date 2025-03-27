
add_library(dependency_format_helper STATIC 
    "format.cpp"
    "parser.cpp"
)
add_library(dependency::format::helper ALIAS dependency_format_helper)

target_include_directories(dependency_format_helper PRIVATE "./" "include/format/helper/")
target_include_directories(dependency_format_helper PUBLIC "include/")
target_link_libraries(dependency_format_helper PUBLIC dependency::core)
target_link_libraries(dependency_format_helper PUBLIC dependency::loglet)

setup_target(dependency_format_helper)

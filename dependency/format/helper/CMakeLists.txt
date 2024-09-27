
add_library(dependency_format_helper STATIC 
    "parser.cpp"
)
add_library(dependency::format::helper ALIAS dependency_format_helper)

target_include_directories(dependency_format_helper PRIVATE "./" "include/format/helper/")
target_include_directories(dependency_format_helper PUBLIC "include/")
target_link_libraries(dependency_format_helper PUBLIC dependency::core)

setup_target(dependency_format_helper)

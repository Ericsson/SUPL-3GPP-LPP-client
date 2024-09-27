
add_library(dependency_format_at STATIC 
    "parser.cpp"
)
add_library(dependency::format::at ALIAS dependency_format_at)

target_include_directories(dependency_format_at PRIVATE "./" "include/format/at/")
target_include_directories(dependency_format_at PUBLIC "include/")
target_link_libraries(dependency_format_at PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_at PUBLIC dependency::loglet)
target_link_libraries(dependency_format_at PUBLIC dependency::core)

setup_target(dependency_format_at)

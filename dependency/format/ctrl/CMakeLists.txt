
add_library(dependency_format_ctrl STATIC 
    "message.cpp"
    "parser.cpp"
    "cid.cpp"
    "identity.cpp"
)
add_library(dependency::format::ctrl ALIAS dependency_format_ctrl)

target_include_directories(dependency_format_ctrl PRIVATE "./" "include/format/ctrl/")
target_include_directories(dependency_format_ctrl PUBLIC "include/")
target_link_libraries(dependency_format_ctrl PUBLIC dependency_format_helper)
target_link_libraries(dependency_format_ctrl PUBLIC dependency::loglet)

setup_target(dependency_format_ctrl)

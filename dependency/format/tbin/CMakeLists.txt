add_library(dependency_format_tbin STATIC
    reader.cpp
    writer.cpp
)
add_library(dependency::format::tbin ALIAS dependency_format_tbin)
target_include_directories(dependency_format_tbin PUBLIC include/)
setup_target(dependency_format_tbin)

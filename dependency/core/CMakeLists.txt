
add_library(dependency_core INTERFACE )
add_library(dependency::core ALIAS dependency_core)

target_include_directories(dependency_core INTERFACE "include/")



add_library(dependency_core
    error.cpp
)

add_library(dependency::core ALIAS dependency_core)

target_include_directories(dependency_core PUBLIC include)

setup_target(dependency_core)


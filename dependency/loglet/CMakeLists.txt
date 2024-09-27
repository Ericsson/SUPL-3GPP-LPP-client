
add_library(dependency_loglet 
    "loglet.cpp"
)
add_library(dependency::loglet ALIAS dependency_loglet)
target_include_directories(dependency_loglet PUBLIC "include/")
target_link_libraries(dependency_loglet PRIVATE Threads::Threads atomic)
target_link_libraries(dependency_loglet PUBLIC dependency::core)

setup_target(dependency_loglet)

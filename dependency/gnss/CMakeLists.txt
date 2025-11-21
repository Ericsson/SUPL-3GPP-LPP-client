
add_library(dependency_gnss STATIC 
    "satellite_id.cpp"
    "signal_id.cpp"
)
add_library(dependency::gnss ALIAS dependency_gnss)

target_include_directories(dependency_gnss PRIVATE "./")
target_include_directories(dependency_gnss PUBLIC "include/")
target_link_libraries(dependency_gnss PUBLIC dependency::core)

setup_target(dependency_gnss)

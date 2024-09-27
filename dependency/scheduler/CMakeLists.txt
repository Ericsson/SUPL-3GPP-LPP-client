
add_library(dependency_scheduler 
    "scheduler.cpp"
    "timeout.cpp"
    "periodic.cpp"
    "stream.cpp"
    "file_descriptor.cpp"
    "socket.cpp"
)
add_library(dependency::scheduler ALIAS dependency_scheduler)
target_include_directories(dependency_scheduler PRIVATE "./" "include/scheduler/")
target_include_directories(dependency_scheduler PUBLIC "include/")
target_link_libraries(dependency_scheduler PRIVATE dependency::loglet)
target_link_libraries(dependency_scheduler PUBLIC dependency::core)

setup_target(dependency_scheduler)


add_library(dependency_io STATIC 
    "io.cpp"
    "input.cpp"
    "output.cpp"
    "file.cpp"
    "stdin.cpp"
    "stdout.cpp"
    "serial.cpp"
    "tcp.cpp"
    "udp.cpp"
)
add_library(dependency::io ALIAS dependency_io)
target_include_directories(dependency_io PRIVATE "./" "include/io/")
target_include_directories(dependency_io PUBLIC "include/")
target_link_libraries(dependency_io PRIVATE dependency::loglet)
target_link_libraries(dependency_io PRIVATE dependency::scheduler)
target_link_libraries(dependency_io PUBLIC dependency::core)

setup_target(dependency_io)

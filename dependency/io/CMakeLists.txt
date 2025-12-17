
add_library(dependency_io STATIC 
    "io.cpp"
    "input.cpp"
    "output.cpp"
    "buffer.cpp"
    "file.cpp"
    "stdin.cpp"
    "stdout.cpp"
    "serial.cpp"
    "tcp.cpp"
    "udp.cpp"
    "write_buffer.cpp"
    "stream.cpp"
    "registry.cpp"
    "adapters.cpp"
    "stream/serial.cpp"
    "stream/tcp_client.cpp"
    "stream/tcp_server.cpp"
    "stream/udp_client.cpp"
    "stream/udp_server.cpp"
    "stream/fd.cpp"
    "stream/pty.cpp"
    "stream/stdio.cpp"
    "stream/file.cpp"
)
add_library(dependency::io ALIAS dependency_io)
target_include_directories(dependency_io PRIVATE "./" "include/io/")
target_include_directories(dependency_io PUBLIC "include/")
target_link_libraries(dependency_io PRIVATE dependency::loglet)
target_link_libraries(dependency_io PRIVATE dependency::scheduler)
target_link_libraries(dependency_io PUBLIC dependency::core)

setup_target(dependency_io)

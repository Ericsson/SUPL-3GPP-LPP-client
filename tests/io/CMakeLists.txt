add_executable(io_tests
    main.cpp
    buffer.cpp
    write_buffer.cpp
    fd_stream.cpp
    pty_stream.cpp
    serial_stream.cpp
    tcp_stream.cpp
    udp_stream.cpp
    file_stream.cpp
    registry.cpp
    adapters.cpp
)
target_link_libraries(io_tests PRIVATE 
    dependency::io
    dependency::scheduler
    dependency::core
    dependency::loglet
    doctest::doctest
)
target_compile_options(io_tests PRIVATE -fsanitize=address -g)
target_link_options(io_tests PRIVATE -fsanitize=address)

add_test(NAME io_tests COMMAND io_tests --no-skip)
set_tests_properties(io_tests PROPERTIES LABELS "io")

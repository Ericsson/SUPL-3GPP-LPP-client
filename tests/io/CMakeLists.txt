add_executable(io_tests
    main.cpp
    buffer.cpp
)
target_link_libraries(io_tests PRIVATE 
    dependency::io
    dependency::scheduler
    dependency::core
    doctest::doctest
)
target_compile_options(io_tests PRIVATE -fsanitize=address -g)
target_link_options(io_tests PRIVATE -fsanitize=address)

add_test(NAME io_tests COMMAND io_tests --no-skip)
set_tests_properties(io_tests PROPERTIES LABELS "io")

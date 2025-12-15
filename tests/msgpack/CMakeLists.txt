add_executable(msgpack_tests
    main.cpp
    basic.cpp
    advanced.cpp
)
target_link_libraries(msgpack_tests PRIVATE 
    dependency::msgpack
    dependency::core
    doctest::doctest
)
target_compile_options(msgpack_tests PRIVATE -fsanitize=address -g)
target_link_options(msgpack_tests PRIVATE -fsanitize=address)

add_test(NAME msgpack_tests COMMAND msgpack_tests --no-skip)
set_tests_properties(msgpack_tests PROPERTIES LABELS "msgpack")

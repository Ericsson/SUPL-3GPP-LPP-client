add_executable(error_tests
    main.cpp
    basic.cpp
    usage.cpp
    result.cpp
    registration.cpp
)
target_link_libraries(error_tests PRIVATE 
    dependency::core
    doctest::doctest
)
target_compile_options(error_tests PRIVATE -fsanitize=address -g)
target_link_options(error_tests PRIVATE -fsanitize=address)

add_test(NAME error_tests COMMAND error_tests --no-skip)
set_tests_properties(error_tests PROPERTIES LABELS "error")

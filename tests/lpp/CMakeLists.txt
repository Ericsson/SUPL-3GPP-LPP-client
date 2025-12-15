add_executable(lpp_tests
    main.cpp
    horizontal_accuracy.cpp
)
target_link_libraries(lpp_tests PRIVATE 
    dependency::lpp
    dependency::core
    doctest::doctest
)
target_compile_options(lpp_tests PRIVATE -fsanitize=address -g)
target_link_options(lpp_tests PRIVATE -fsanitize=address)

add_test(NAME lpp_tests COMMAND lpp_tests --no-skip)
set_tests_properties(lpp_tests PROPERTIES LABELS "lpp")

add_executable(time_tests 
    main.cpp
    timestamp.cpp
    gps.cpp
    glo.cpp
    conversions.cpp
)
target_link_libraries(time_tests PRIVATE dependency::time dependency::core doctest::doctest)
target_compile_options(time_tests PRIVATE -fsanitize=address -g)
target_link_options(time_tests PRIVATE -fsanitize=address)

add_test(NAME time_tests COMMAND time_tests --no-skip)
set_tests_properties(time_tests PROPERTIES LABELS "time")

CPMAddPackage("gh:doctest/doctest@2.4.11")

add_executable(time_tests 
    time/main.cpp
    time/timestamp.cpp
    time/gps.cpp
    time/conversions.cpp
)
target_link_libraries(time_tests PRIVATE dependency::time dependency::core doctest::doctest)

add_test(NAME time_tests COMMAND time_tests --no-skip)
set_tests_properties(time_tests PROPERTIES LABELS "time")

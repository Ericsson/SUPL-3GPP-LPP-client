add_executable(gnss_tests
    main.cpp
    phase_alignment.cpp
)
target_link_libraries(gnss_tests PRIVATE dependency::gnss doctest::doctest)
setup_target(gnss_tests)
add_test(NAME gnss_tests COMMAND gnss_tests)

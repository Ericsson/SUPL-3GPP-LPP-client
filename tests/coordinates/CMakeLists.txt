add_executable(coordinates_tests
    main.cpp
    test_ecef_llh.cpp
    test_ecef_enu_ned_aer.cpp
    test_edge_cases.cpp
    test_regression.cpp
    test_transform.cpp
    test_sweref99.cpp
)
target_link_libraries(coordinates_tests PRIVATE 
    dependency::coordinates
    dependency::maths
    doctest::doctest
)
if(INCLUDE_GENERATOR_TOKORO)
    target_link_libraries(coordinates_tests PRIVATE dependency::generator::tokoro)
endif()
if(INCLUDE_GENERATOR_IDOKEIDO)
    target_link_libraries(coordinates_tests PRIVATE dependency::generator::idokeido)
endif()
target_compile_options(coordinates_tests PRIVATE -fsanitize=address -g)
target_link_options(coordinates_tests PRIVATE -fsanitize=address)

add_test(NAME coordinates_tests COMMAND coordinates_tests --no-skip)
set_tests_properties(coordinates_tests PROPERTIES LABELS "coordinates")

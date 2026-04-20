add_executable(generator_spartn_tests
    main.cpp
    time.cpp
)
target_include_directories(generator_spartn_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/dependency/generator/spartn
)
target_link_libraries(generator_spartn_tests PRIVATE
    dependency::generator::spartn2
    dependency::time
    dependency::core
    doctest::doctest
    asn1::generated::lpp asn1::helper
)
target_compile_options(generator_spartn_tests PRIVATE -fsanitize=address -g)
target_link_options(generator_spartn_tests PRIVATE -fsanitize=address)

add_test(NAME generator_spartn_tests COMMAND generator_spartn_tests --no-skip)
set_tests_properties(generator_spartn_tests PROPERTIES LABELS "generator_spartn")

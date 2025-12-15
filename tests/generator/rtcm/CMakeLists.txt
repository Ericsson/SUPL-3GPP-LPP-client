add_executable(generator_rtcm_tests
    main.cpp
    qzss.cpp
    generator.cpp
)
target_link_libraries(generator_rtcm_tests PRIVATE 
    dependency::generator::rtcm
    dependency::core
    test_utils
    doctest::doctest
    asn1::generated::lpp asn1::helper
)
target_compile_options(generator_rtcm_tests PRIVATE -fsanitize=address -g)
target_link_options(generator_rtcm_tests PRIVATE -fsanitize=address)

add_test(NAME generator_rtcm_tests COMMAND generator_rtcm_tests --no-skip)
set_tests_properties(generator_rtcm_tests PROPERTIES LABELS "generator_rtcm")

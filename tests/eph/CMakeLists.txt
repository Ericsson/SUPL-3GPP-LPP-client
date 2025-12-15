add_executable(eph_tests
    main.cpp
    gps.cpp
    gal.cpp
    bds.cpp
    qzss.cpp
    glo.cpp
)
target_link_libraries(eph_tests PRIVATE 
    dependency::ephemeris
    dependency::msgpack
    dependency::time
    dependency::core
    dependency::loglet
    test_utils
    doctest::doctest
)
target_compile_options(eph_tests PRIVATE -fsanitize=address -g)
target_link_options(eph_tests PRIVATE -fsanitize=address)

add_test(NAME eph_tests COMMAND eph_tests --no-skip)
set_tests_properties(eph_tests PROPERTIES LABELS "eph")

add_executable(generator_tokoro_tests
    main.cpp
    coordinates.cpp
)
target_link_libraries(generator_tokoro_tests PRIVATE 
    dependency::generator::tokoro
    dependency::core
    doctest::doctest
)
target_compile_options(generator_tokoro_tests PRIVATE -fsanitize=address -g)
target_link_options(generator_tokoro_tests PRIVATE -fsanitize=address)

add_test(NAME generator_tokoro_tests COMMAND generator_tokoro_tests --no-skip)
set_tests_properties(generator_tokoro_tests PROPERTIES LABELS "generator_tokoro")

if(ENABLE_TOKORO_SNAPSHOT)
    add_executable(generator_tokoro_snapshot_tests
        main.cpp
        snapshot.cpp
    )
    target_link_libraries(generator_tokoro_snapshot_tests PRIVATE 
        dependency::generator::tokoro
        dependency::ephemeris
        dependency::msgpack
        dependency::core
        test_utils
        doctest::doctest
    )
    target_compile_options(generator_tokoro_snapshot_tests PRIVATE -fsanitize=address -g)
    target_link_options(generator_tokoro_snapshot_tests PRIVATE -fsanitize=address)

    add_test(NAME generator_tokoro_snapshot_tests COMMAND generator_tokoro_snapshot_tests --no-skip)
    set_tests_properties(generator_tokoro_snapshot_tests PROPERTIES LABELS "generator_tokoro")
endif()

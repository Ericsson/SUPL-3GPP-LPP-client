add_executable(scheduler_tests
    main.cpp
    basic.cpp
    socket.cpp
    stream.cpp
    stress.cpp
    integration.cpp
)
target_link_libraries(scheduler_tests PRIVATE 
    dependency::scheduler
    dependency::core
    dependency::loglet
    doctest::doctest
)
target_compile_options(scheduler_tests PRIVATE -fsanitize=address -g)
target_link_options(scheduler_tests PRIVATE -fsanitize=address)

add_test(NAME scheduler_tests COMMAND scheduler_tests --no-skip)
set_tests_properties(scheduler_tests PROPERTIES LABELS "scheduler")

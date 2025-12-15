add_executable(format_tests
    main.cpp
    nmea/parser.cpp
    ubx/parser.cpp
    rtcm/parser.cpp
    at/parser.cpp
)
target_link_libraries(format_tests PRIVATE 
    dependency::format::nmea 
    dependency::format::ubx 
    dependency::format::rtcm
    dependency::format::at
    dependency::core 
    doctest::doctest
)

if(INCLUDE_FORMAT_ANTEX)
    target_sources(format_tests PRIVATE antex/parser.cpp)
    target_link_libraries(format_tests PRIVATE dependency::format::antex)
endif()

target_compile_options(format_tests PRIVATE -fsanitize=address -g)
target_link_options(format_tests PRIVATE -fsanitize=address)

add_test(NAME format_tests COMMAND format_tests --no-skip)
set_tests_properties(format_tests PROPERTIES LABELS "format")

if(ENABLE_FUZZING)
    add_executable(fuzz_nmea nmea/fuzz_target.cpp)
    target_link_libraries(fuzz_nmea PRIVATE dependency::format::nmea dependency::core)
    target_compile_options(fuzz_nmea PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_nmea PRIVATE -fsanitize=fuzzer,address)
    
    add_executable(fuzz_ubx ubx/fuzz_target.cpp)
    target_link_libraries(fuzz_ubx PRIVATE dependency::format::ubx dependency::core)
    target_compile_options(fuzz_ubx PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_ubx PRIVATE -fsanitize=fuzzer,address)
    
    add_executable(fuzz_rtcm rtcm/fuzz_target.cpp)
    target_link_libraries(fuzz_rtcm PRIVATE dependency::format::rtcm dependency::core)
    target_compile_options(fuzz_rtcm PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_rtcm PRIVATE -fsanitize=fuzzer,address)
    
    add_executable(fuzz_at at/fuzz_target.cpp)
    target_link_libraries(fuzz_at PRIVATE dependency::format::at dependency::core)
    target_compile_options(fuzz_at PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_at PRIVATE -fsanitize=fuzzer,address)
    
    if(INCLUDE_FORMAT_ANTEX)
        add_executable(fuzz_antex antex/fuzz_target.cpp)
        target_link_libraries(fuzz_antex PRIVATE dependency::format::antex dependency::core)
        target_compile_options(fuzz_antex PRIVATE -fsanitize=fuzzer,address -g)
        target_link_options(fuzz_antex PRIVATE -fsanitize=fuzzer,address)
    endif()
endif()

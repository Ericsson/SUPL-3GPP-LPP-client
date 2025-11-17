CPMAddPackage("gh:doctest/doctest@2.4.11")

add_executable(time_tests 
    time/main.cpp
    time/timestamp.cpp
    time/gps.cpp
    time/conversions.cpp
)
target_link_libraries(time_tests PRIVATE dependency::time dependency::core doctest::doctest)

add_executable(format_tests
    format/main.cpp
    format/nmea/parser.cpp
    format/ubx/parser.cpp
    format/rtcm/parser.cpp
)
target_link_libraries(format_tests PRIVATE 
    dependency::format::nmea 
    dependency::format::ubx 
    dependency::format::rtcm
    dependency::core 
    doctest::doctest
)

add_test(NAME time_tests COMMAND time_tests --no-skip)
set_tests_properties(time_tests PROPERTIES LABELS "time")

add_test(NAME format_tests COMMAND format_tests --no-skip)
set_tests_properties(format_tests PROPERTIES LABELS "format")

if(ENABLE_FUZZING)
    add_executable(fuzz_nmea format/nmea/fuzz_target.cpp)
    target_link_libraries(fuzz_nmea PRIVATE dependency::format::nmea dependency::core)
    target_compile_options(fuzz_nmea PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_nmea PRIVATE -fsanitize=fuzzer,address)
    
    add_executable(fuzz_ubx format/ubx/fuzz_target.cpp)
    target_link_libraries(fuzz_ubx PRIVATE dependency::format::ubx dependency::core)
    target_compile_options(fuzz_ubx PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_ubx PRIVATE -fsanitize=fuzzer,address)
    
    add_executable(fuzz_rtcm format/rtcm/fuzz_target.cpp)
    target_link_libraries(fuzz_rtcm PRIVATE dependency::format::rtcm dependency::core)
    target_compile_options(fuzz_rtcm PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_rtcm PRIVATE -fsanitize=fuzzer,address)
endif()

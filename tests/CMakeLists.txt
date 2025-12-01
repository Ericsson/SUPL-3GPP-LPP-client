CPMAddPackage("gh:doctest/doctest@2.4.11")

add_executable(coordinates_tests
    coordinates/main.cpp
    coordinates/test_ecef_llh.cpp
    coordinates/test_ecef_enu_ned_aer.cpp
    coordinates/test_edge_cases.cpp
    coordinates/test_regression.cpp
)
target_link_libraries(coordinates_tests PRIVATE 
    coordinates 
    dependency::maths
    dependency::generator::tokoro
    dependency::generator::idokeido
    doctest::doctest
)
target_compile_options(coordinates_tests PRIVATE -fsanitize=address -g)
target_link_options(coordinates_tests PRIVATE -fsanitize=address)

add_test(NAME coordinates_tests COMMAND coordinates_tests --no-skip)
set_tests_properties(coordinates_tests PROPERTIES LABELS "coordinates")

add_executable(time_tests 
    time/main.cpp
    time/timestamp.cpp
    time/gps.cpp
    time/glo.cpp
    time/conversions.cpp
)
target_link_libraries(time_tests PRIVATE dependency::time dependency::core doctest::doctest)
target_compile_options(time_tests PRIVATE -fsanitize=address -g)
target_link_options(time_tests PRIVATE -fsanitize=address)

add_executable(format_tests
    format/main.cpp
    format/nmea/parser.cpp
    format/ubx/parser.cpp
    format/rtcm/parser.cpp
    format/at/parser.cpp
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
target_sources(format_tests PRIVATE format/antex/parser.cpp)
target_link_libraries(format_tests PRIVATE dependency::format::antex)
endif()

target_compile_options(format_tests PRIVATE -fsanitize=address -g)
target_link_options(format_tests PRIVATE -fsanitize=address)

add_test(NAME time_tests COMMAND time_tests --no-skip)
set_tests_properties(time_tests PROPERTIES LABELS "time")

add_test(NAME format_tests COMMAND format_tests --no-skip)
set_tests_properties(format_tests PROPERTIES LABELS "format")

add_executable(io_tests
    io/main.cpp
    io/buffer.cpp
)
target_link_libraries(io_tests PRIVATE 
    dependency::io
    dependency::scheduler
    dependency::core
    doctest::doctest
)
target_compile_options(io_tests PRIVATE -fsanitize=address -g)
target_link_options(io_tests PRIVATE -fsanitize=address)

add_test(NAME io_tests COMMAND io_tests --no-skip)
set_tests_properties(io_tests PROPERTIES LABELS "io")

add_executable(eph_tests
    eph/main.cpp
    eph/gps.cpp
    eph/gal.cpp
    eph/bds.cpp
    eph/qzss.cpp
    eph/glo.cpp
)
target_link_libraries(eph_tests PRIVATE 
    dependency::ephemeris
    dependency::msgpack
    dependency::time
    dependency::core
    dependency::loglet
    doctest::doctest
)
target_compile_options(eph_tests PRIVATE -fsanitize=address -g)
target_link_options(eph_tests PRIVATE -fsanitize=address)

add_test(NAME eph_tests COMMAND eph_tests --no-skip)
set_tests_properties(eph_tests PROPERTIES LABELS "eph")

add_executable(error_tests
    error/main.cpp
    error/basic.cpp
    error/usage.cpp
    error/result.cpp
    error/registration.cpp
)
target_link_libraries(error_tests PRIVATE 
    dependency::core
    dependency::core
    doctest::doctest
)
target_compile_options(error_tests PRIVATE -fsanitize=address -g)
target_link_options(error_tests PRIVATE -fsanitize=address)

add_test(NAME error_tests COMMAND error_tests --no-skip)
set_tests_properties(error_tests PROPERTIES LABELS "error")

add_executable(scheduler_tests
    scheduler/main.cpp
    scheduler/basic.cpp
    scheduler/socket.cpp
    scheduler/stream.cpp
    scheduler/stress.cpp
    scheduler/integration.cpp
    # scheduler/write_buffer.cpp  # Enable when FileDescriptorTask has write buffering
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

add_executable(msgpack_tests
    msgpack/main.cpp
    msgpack/basic.cpp
    msgpack/advanced.cpp
)
target_link_libraries(msgpack_tests PRIVATE 
    dependency::msgpack
    dependency::core
    doctest::doctest
)
target_compile_options(msgpack_tests PRIVATE -fsanitize=address -g)
target_link_options(msgpack_tests PRIVATE -fsanitize=address)

add_test(NAME msgpack_tests COMMAND msgpack_tests --no-skip)
set_tests_properties(msgpack_tests PROPERTIES LABELS "msgpack")

if(INCLUDE_GENERATOR_RTCM)
add_executable(generator_rtcm_tests
    generator/rtcm/main.cpp
    generator/rtcm/qzss.cpp
)
target_link_libraries(generator_rtcm_tests PRIVATE 
    dependency::generator::rtcm
    dependency::core
    doctest::doctest
)
target_compile_options(generator_rtcm_tests PRIVATE -fsanitize=address -g)
target_link_options(generator_rtcm_tests PRIVATE -fsanitize=address)

add_test(NAME generator_rtcm_tests COMMAND generator_rtcm_tests --no-skip)
set_tests_properties(generator_rtcm_tests PROPERTIES LABELS "generator_rtcm")
endif()

if(INCLUDE_GENERATOR_TOKORO)
add_executable(generator_tokoro_tests
    generator/tokoro/main.cpp
    generator/tokoro/coordinates.cpp
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
endif()

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
    
    add_executable(fuzz_at format/at/fuzz_target.cpp)
    target_link_libraries(fuzz_at PRIVATE dependency::format::at dependency::core)
    target_compile_options(fuzz_at PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_at PRIVATE -fsanitize=fuzzer,address)
    
    if(INCLUDE_FORMAT_ANTEX)
    add_executable(fuzz_antex format/antex/fuzz_target.cpp)
    target_link_libraries(fuzz_antex PRIVATE dependency::format::antex dependency::core)
    target_compile_options(fuzz_antex PRIVATE -fsanitize=fuzzer,address -g)
    target_link_options(fuzz_antex PRIVATE -fsanitize=fuzzer,address)
    endif()
endif()

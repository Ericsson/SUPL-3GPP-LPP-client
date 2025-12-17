CPMAddPackage("gh:doctest/doctest@2.4.11")

add_library(test_utils INTERFACE)
target_include_directories(test_utils INTERFACE test_utils)
target_compile_definitions(test_utils INTERFACE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_subdirectory(coordinates)
add_subdirectory(time)
add_subdirectory(format)
add_subdirectory(io)
add_subdirectory(lpp)
add_subdirectory(eph)
add_subdirectory(error)
add_subdirectory(scheduler)
add_subdirectory(msgpack)
add_subdirectory(gnss)
add_subdirectory(generator)

if(INCLUDE_GENERATOR_TOKORO AND ENABLE_TOKORO_SNAPSHOT)
    add_executable(generate_tokoro_snapshot generate_tokoro_snapshot.cpp)
    target_link_libraries(generate_tokoro_snapshot PRIVATE
        dependency::generator::tokoro
        dependency::loglet
        dependency::msgpack
        args
    )
    setup_target(generate_tokoro_snapshot)
endif()

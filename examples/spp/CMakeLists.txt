add_executable(example_spp "main.cpp")
add_executable(example::spp ALIAS example_spp)

target_link_libraries(example_spp PRIVATE dependency::generator::idokeido)
target_link_libraries(example_spp PRIVATE dependency::format::rinex)
target_link_libraries(example_spp PRIVATE dependency::ephemeris)
target_link_libraries(example_spp PRIVATE dependency::loglet)
target_include_directories(example_spp PRIVATE ${CMAKE_SOURCE_DIR}/external/args/include)

setup_target(example_spp)

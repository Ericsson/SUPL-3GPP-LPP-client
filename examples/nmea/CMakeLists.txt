add_executable(example_nmea 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::nmea ALIAS example_nmea)

target_include_directories(example_nmea PRIVATE "./")
target_link_libraries(example_nmea PRIVATE args)
target_link_libraries(example_nmea PRIVATE receiver::nmea)
target_link_libraries(example_nmea PRIVATE dependency::interface)

set_target_properties(example_nmea PROPERTIES OUTPUT_NAME "example-nmea")
set_target_properties(example_nmea PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_nmea)

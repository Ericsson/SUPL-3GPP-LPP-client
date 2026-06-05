add_executable(example_relay main.cpp)
add_executable(examples::relay ALIAS example_relay)

target_link_libraries(example_relay PRIVATE dependency::client-io)
target_link_libraries(example_relay PRIVATE dependency::scheduler)
target_link_libraries(example_relay PRIVATE dependency::loglet)

set_target_properties(example_relay PROPERTIES OUTPUT_NAME "example-relay")
set_target_properties(example_relay PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_relay)

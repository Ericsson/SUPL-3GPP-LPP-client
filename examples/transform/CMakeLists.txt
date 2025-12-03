add_executable(example_transform "main.cpp")
add_executable(examples::transform ALIAS example_transform)

target_link_libraries(example_transform PRIVATE args)
target_link_libraries(example_transform PRIVATE dependency::coordinates)
target_link_libraries(example_transform PRIVATE dependency::core)
target_link_libraries(example_transform PRIVATE dependency::loglet)

set_target_properties(example_transform PROPERTIES OUTPUT_NAME "example-transform")
set_target_properties(example_transform PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_transform)

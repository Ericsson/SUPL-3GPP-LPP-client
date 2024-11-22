add_executable(example_crs 
    "main.cpp"
)
add_executable(examples::crs ALIAS example_crs)

target_include_directories(example_crs PRIVATE "./")
target_link_libraries(example_crs PRIVATE args)
target_link_libraries(example_crs PRIVATE dependency::generator::tokoro)
target_link_libraries(example_crs PRIVATE dependency::maths)

set_target_properties(example_crs PROPERTIES OUTPUT_NAME "example-crs")
set_target_properties(example_crs PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_crs)

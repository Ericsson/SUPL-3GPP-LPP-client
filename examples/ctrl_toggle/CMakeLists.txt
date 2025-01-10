

add_executable(example_ctrl_toggle 
    "main.cpp" 
)
add_executable(examples::ctrl_toggle ALIAS example_ctrl_toggle)

target_include_directories(example_ctrl_toggle PRIVATE "./")
target_link_libraries(example_ctrl_toggle PRIVATE args)
target_link_libraries(example_ctrl_toggle PRIVATE dependency::loglet)

set_target_properties(example_ctrl_toggle PROPERTIES OUTPUT_NAME "example-ctrl-toggle")
set_target_properties(example_ctrl_toggle PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_ctrl_toggle)


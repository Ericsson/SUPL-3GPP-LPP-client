

add_executable(example_ctrl_switch 
    "main.cpp" 
    "options.cpp" 
)
add_executable(examples::ctrl_switch ALIAS example_ctrl_switch)

target_include_directories(example_ctrl_switch PRIVATE "./")
target_link_libraries(example_ctrl_switch PRIVATE utility modem lpplib Threads::Threads)
target_link_libraries(example_ctrl_switch PRIVATE args)

set_target_properties(example_ctrl_switch PROPERTIES OUTPUT_NAME "example-ctrl-switch")
set_target_properties(example_ctrl_switch PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_ctrl_switch)


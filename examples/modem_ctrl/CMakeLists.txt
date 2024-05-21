

add_executable(example_modem_ctrl 
    "main.cpp" 
    "options.cpp" 
)
add_executable(examples::modem_ctrl ALIAS example_modem_ctrl)

target_include_directories(example_modem_ctrl PRIVATE "./")
target_link_libraries(example_modem_ctrl PRIVATE utility modem lpplib Threads::Threads)
target_link_libraries(example_modem_ctrl PRIVATE args)
target_link_libraries(example_modem_ctrl PRIVATE dependency::interface)

set_target_properties(example_modem_ctrl PROPERTIES OUTPUT_NAME "example-modem-ctrl")
set_target_properties(example_modem_ctrl PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_modem_ctrl)




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

if (USE_OPENSSL)
target_link_libraries(example_ctrl_switch PRIVATE OpenSSL::SSL)
target_compile_definitions(example_ctrl_switch PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(example_ctrl_switch PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_ctrl_switch PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

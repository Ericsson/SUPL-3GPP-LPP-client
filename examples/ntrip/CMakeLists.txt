add_executable(example_ntrip 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::ntrip ALIAS example_ntrip)

target_include_directories(example_ntrip PRIVATE "./")
target_link_libraries(example_ntrip PRIVATE args)
target_link_libraries(example_ntrip PRIVATE dependency::interface)

set_target_properties(example_ntrip PROPERTIES OUTPUT_NAME "example-ntrip")
set_target_properties(example_ntrip PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_ASAN)
target_compile_options(example_ntrip PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_ntrip PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

add_executable(example_ublox 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::ublox ALIAS example_ublox)

target_include_directories(example_ublox PRIVATE "./")
target_link_libraries(example_ublox PRIVATE args)
target_link_libraries(example_ublox PRIVATE receiver::ublox)
target_link_libraries(example_ublox PRIVATE dependency::interface)

set_target_properties(example_ublox PROPERTIES OUTPUT_NAME "example-ublox")
set_target_properties(example_ublox PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_ASAN)
target_compile_options(example_ublox PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_ublox PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

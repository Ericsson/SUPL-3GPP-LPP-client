add_executable(example_lpp2spartn 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::lpp2spartn ALIAS example_lpp2spartn)

target_include_directories(example_lpp2spartn PRIVATE "./")
target_link_libraries(example_lpp2spartn PRIVATE args)
target_link_libraries(example_lpp2spartn PRIVATE generator::spartn)
target_link_libraries(example_lpp2spartn PRIVATE generator::spartn2)
target_link_libraries(example_lpp2spartn PRIVATE asn1::generated asn1::helper)
target_link_libraries(example_lpp2spartn PRIVATE dependency::interface)

set_target_properties(example_lpp2spartn PROPERTIES OUTPUT_NAME "example-lpp2spartn")
set_target_properties(example_lpp2spartn PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_ASAN)
target_compile_options(example_lpp2spartn PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_lpp2spartn PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

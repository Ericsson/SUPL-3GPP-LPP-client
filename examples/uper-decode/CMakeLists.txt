add_executable(example_decode_uper 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::decode-uper ALIAS example_decode_uper)

target_include_directories(example_decode_uper PRIVATE "./")
target_link_libraries(example_decode_uper PRIVATE args)
target_link_libraries(example_decode_uper PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(example_decode_uper PRIVATE dependency::interface)

set_target_properties(example_decode_uper PROPERTIES OUTPUT_NAME "example-decode-uper")
set_target_properties(example_decode_uper PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_decode_uper)


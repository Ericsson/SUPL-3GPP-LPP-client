add_executable(example_ntrip_caster
    "main.cpp"
    "options.cpp"
)
add_executable(examples::ntrip-caster ALIAS example_ntrip_caster)

target_include_directories(example_ntrip_caster PRIVATE "./")
target_link_libraries(example_ntrip_caster PRIVATE dependency::client-io)
target_link_libraries(example_ntrip_caster PRIVATE dependency::scheduler)
target_link_libraries(example_ntrip_caster PRIVATE dependency::io)

set_target_properties(example_ntrip_caster PROPERTIES OUTPUT_NAME "example-ntrip-caster")
set_target_properties(example_ntrip_caster PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_ntrip_caster)

add_executable(example_lpp2spartn 
    "main.cpp"
    "options.cpp"
)
add_executable(examples::lpp2spartn ALIAS example_lpp2spartn)

target_include_directories(example_lpp2spartn PRIVATE "./")
target_link_libraries(example_lpp2spartn PRIVATE args)
target_link_libraries(example_lpp2spartn PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(example_lpp2spartn PRIVATE dependency::interface)

if(${INCLUDE_GENERATOR_SPARTN})
    target_compile_definitions(example_lpp2spartn PRIVATE "INCLUDE_GENERATOR_SPARTN=1")
    target_link_libraries(example_lpp2spartn PRIVATE generator::spartn2)
endif()

if(${INCLUDE_GENERATOR_SPARTN_OLD})
    target_compile_definitions(example_lpp2spartn PRIVATE "INCLUDE_GENERATOR_SPARTN_OLD=1")
    target_link_libraries(example_lpp2spartn PRIVATE generator::spartn)
endif()

set_target_properties(example_lpp2spartn PROPERTIES OUTPUT_NAME "example-lpp2spartn")
set_target_properties(example_lpp2spartn PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_lpp2spartn)


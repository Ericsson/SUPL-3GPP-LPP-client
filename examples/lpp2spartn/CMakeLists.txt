add_executable(example_lpp2spartn "main.cpp")
add_executable(examples::lpp2spartn ALIAS example_lpp2spartn)

target_link_libraries(example_lpp2spartn PRIVATE
    args
    dependency::loglet
    dependency::generator::spartn2
    asn1::generated::lpp
    asn1::helper
)

set_target_properties(example_lpp2spartn PROPERTIES OUTPUT_NAME "lpp2spartn")
set_target_properties(example_lpp2spartn PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_lpp2spartn)

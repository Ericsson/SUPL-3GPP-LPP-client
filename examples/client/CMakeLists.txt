
add_executable(example_client 
    "main.cpp" 
    "assistance_data.cpp" 
)
add_executable(examples::client ALIAS example_client)

target_include_directories(example_client PRIVATE "./")
target_link_libraries(example_client PRIVATE args)
target_link_libraries(example_client PRIVATE dependency::streamline)
target_link_libraries(example_client PRIVATE dependency::io)
target_link_libraries(example_client PRIVATE dependency::format::ubx)
target_link_libraries(example_client PRIVATE dependency::format::nmea)
target_link_libraries(example_client PRIVATE dependency::format::ctrl)
target_link_libraries(example_client PRIVATE dependency::format::lpp)
target_link_libraries(example_client PRIVATE dependency::scheduler)
target_link_libraries(example_client PRIVATE dependency::loglet)
target_link_libraries(example_client PRIVATE dependency::core)
target_link_libraries(example_client PRIVATE dependency::supl)
target_link_libraries(example_client PRIVATE dependency::lpp)
target_link_libraries(example_client PRIVATE asn1::generated::lpp asn1::helper)

set_target_properties(example_client PROPERTIES OUTPUT_NAME "example-client")
set_target_properties(example_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_client)
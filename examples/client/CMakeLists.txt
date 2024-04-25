
add_executable(example_client 
    "main.cpp" 
    "assistance_data.cpp" 
)
add_executable(examples::client ALIAS example_client)

target_include_directories(example_client PRIVATE "./")
target_link_libraries(example_client PRIVATE utility modem lpplib Threads::Threads)
target_link_libraries(example_client PRIVATE args)
target_link_libraries(example_client PRIVATE generator::rtcm)
target_link_libraries(example_client PRIVATE generator::spartn)
target_link_libraries(example_client PRIVATE generator::spartn2)
target_link_libraries(example_client PRIVATE dependency::interface)
target_link_libraries(example_client PRIVATE receiver::ublox)
target_link_libraries(example_client PRIVATE receiver::nmea)
target_link_libraries(example_client PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(example_client PRIVATE scheduler supl lpp loglet)

set_target_properties(example_client PROPERTIES OUTPUT_NAME "example-client")
set_target_properties(example_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_OPENSSL)
target_link_libraries(example_client PRIVATE OpenSSL::SSL)
target_compile_definitions(example_client PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(example_client PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_client PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

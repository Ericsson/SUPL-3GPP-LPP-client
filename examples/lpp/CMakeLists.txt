

add_executable(example_lpp 
    "main.cpp" 
    "osr_example.cpp" 
    "ssr_example.cpp" 
    "agnss_example.cpp" 
    "location_information.cpp"
    "options.cpp"
)
add_executable(examples::lpp ALIAS example_lpp)

target_include_directories(example_lpp PRIVATE "./")
target_link_libraries(example_lpp PRIVATE utility modem lpplib Threads::Threads)
target_link_libraries(example_lpp PRIVATE args)
target_link_libraries(example_lpp PRIVATE generator::rtcm)
target_link_libraries(example_lpp PRIVATE generator::spartn)
target_link_libraries(example_lpp PRIVATE generator::spartn2)
target_link_libraries(example_lpp PRIVATE dependency::interface)
target_link_libraries(example_lpp PRIVATE receiver::ublox)
target_link_libraries(example_lpp PRIVATE receiver::nmea)
target_link_libraries(example_lpp PRIVATE asn1::generated asn1::helper)

set_target_properties(example_lpp PROPERTIES OUTPUT_NAME "example-lpp")
set_target_properties(example_lpp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_OPENSSL)
target_link_libraries(example_lpp PRIVATE OpenSSL::SSL)
target_compile_definitions(example_lpp PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(example_lpp PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example_lpp PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

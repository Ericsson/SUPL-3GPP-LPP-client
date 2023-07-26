

add_executable(example "main.cpp" "osr_example.cpp" "ssr_example.cpp" "agnss_example.cpp" "location_information.cpp")
target_include_directories(example PRIVATE ../include/)
target_link_libraries(example PRIVATE utility modem rtklib asnlib lpplib osr rtcm_generator Threads::Threads)
target_link_libraries(example PRIVATE args)
target_link_libraries(example PRIVATE transmitter)

if (USE_OPENSSL)
target_link_libraries(example PRIVATE OpenSSL::SSL)
target_compile_definitions(example PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(example PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(example PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

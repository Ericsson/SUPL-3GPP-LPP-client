
add_library(supl 
    "src/session.cpp"
    "src/tcp_client.cpp"
    "src/encode.cpp"
    "src/decode.cpp"
    "src/print.cpp"
)
target_include_directories(supl PUBLIC "include/")
target_link_libraries(supl PRIVATE Threads::Threads atomic)
target_link_libraries(supl PRIVATE asn1::generated::supl asn1::helper)
target_link_libraries(supl PRIVATE loglet)

target_compile_options(supl PUBLIC "-Wno-unused-private-field")

if (USE_ASAN)
target_compile_options(supl PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(supl PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

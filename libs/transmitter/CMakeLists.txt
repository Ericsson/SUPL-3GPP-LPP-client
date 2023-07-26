list(APPEND SPARTN_SOURCES
    "src/transmitter.cpp"
    "src/i2c.cpp"
    "src/serial.cpp"
    "src/tcp.cpp"
    "src/udp.cpp"
    "src/file.cpp"
    "src/stdout.cpp"
)

add_library(transmitter ${SPARTN_SOURCES})
target_include_directories(transmitter PUBLIC "include/")
target_include_directories(transmitter PRIVATE "src/")
target_link_libraries(transmitter PRIVATE utility Threads::Threads)

if (USE_ASAN)
target_compile_options(transmitter PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(transmitter PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

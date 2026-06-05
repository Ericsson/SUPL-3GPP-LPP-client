add_library(client_io STATIC
    parse.cpp
    stream.cpp
    input.cpp
    output.cpp
    io.cpp
    tag_registry.cpp
    chunked_log.cpp
)
add_library(dependency::client-io ALIAS client_io)

target_include_directories(client_io PUBLIC include)
target_link_libraries(client_io PUBLIC
    args
    dependency::io
    dependency::scheduler
    dependency::loglet
    dependency::core
    dependency::streamline
    dependency::format::tbin
)

setup_target(client_io)

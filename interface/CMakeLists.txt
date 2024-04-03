
add_library(dependency_interface STATIC 
    "file.cpp"
    "serial.cpp"
    "i2c.cpp"
    "stdin.cpp"
    "stdout.cpp"
    "file_descriptor.cpp"
    "socket.cpp"
    "reconnectable_socket.cpp"
    "tcp.cpp"
    "udp.cpp"
    "unix_socket.cpp"
)
add_library(dependency::interface ALIAS dependency_interface)

target_include_directories(dependency_interface PRIVATE "./" "include/interface/")
target_include_directories(dependency_interface PUBLIC "include/")

if (USE_ASAN)
target_compile_options(dependency_interface PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(dependency_interface PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

if(${INTERFACE_FD_DEBUG})
    target_compile_definitions(dependency_interface PRIVATE INTERFACE_FD_DEBUG=1)
endif(${INTERFACE_FD_DEBUG})

if(${INTERFACE_SOCKET_DEBUG})
    target_compile_definitions(dependency_interface PRIVATE INTERFACE_SOCKET_DEBUG=1)
endif(${INTERFACE_SOCKET_DEBUG})

target_compile_options(dependency_interface PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Werror"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(dependency_interface PRIVATE
        "-Weverything"
        "-Wno-switch-enum"
        "-Wno-signed-enum-bitfield"
        "-Wno-deprecated-register"
        "-Wno-c++98-compat"
        "-Wno-c++98-compat-pedantic"
        "-Wno-c++98-c++11-compat-pedantic"
        "-Wno-nested-anon-types"
        "-Wno-gnu-anonymous-struct"
        "-Wno-missing-prototypes"
        "-Wno-documentation"
        "-Wno-documentation-unknown-command"
        "-Wno-weak-vtables"
        "-Wno-unused-const-variable"
        "-Wno-format-nonliteral"
        "-Wno-global-constructors"
        "-Wno-exit-time-destructors"
        "-Wno-padded"
        "-Wno-unsafe-buffer-usage"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(dependency_interface PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()


add_library(dependency_interface STATIC 
    "file.cpp"
    "serial.cpp"
    "i2c.cpp"
    "file_descriptor.cpp"
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

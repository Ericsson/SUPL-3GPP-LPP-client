
add_library(dependency_msgpack 
    "msgpack.cpp"
    "vector.cpp"
)
add_library(dependency::msgpack ALIAS dependency_msgpack)
target_include_directories(dependency_msgpack PUBLIC "include/")
target_include_directories(dependency_msgpack PRIVATE "./" "include/msgpack/")
target_link_libraries(dependency_msgpack PUBLIC dependency::core)

setup_target(dependency_msgpack)

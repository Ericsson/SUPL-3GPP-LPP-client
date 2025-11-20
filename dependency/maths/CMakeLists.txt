
add_library(dependency_maths 
    "float3.cpp"
    "mat3.cpp"
)
add_library(dependency::maths ALIAS dependency_maths)
target_include_directories(dependency_maths PUBLIC "include/")
target_include_directories(dependency_maths PRIVATE "./" "include/maths/")
target_link_libraries(dependency_maths PUBLIC dependency::core dependency::msgpack)

setup_target(dependency_maths)

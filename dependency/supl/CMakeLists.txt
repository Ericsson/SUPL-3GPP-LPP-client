
add_library(dependency_supl 
    "supl.cpp"
    "session.cpp"
    "tcp_client.cpp"
    "encode.cpp"
    "decode.cpp"
    "print.cpp"
)
add_library(dependency::supl ALIAS dependency_supl)
target_include_directories(dependency_supl PUBLIC "include/")
target_link_libraries(dependency_supl PRIVATE asn1::generated::supl asn1::helper)
target_link_libraries(dependency_supl PUBLIC dependency::core)
target_link_libraries(dependency_supl PRIVATE dependency::loglet)

setup_target(dependency_supl)

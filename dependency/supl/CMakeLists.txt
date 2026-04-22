add_library(dependency_supl 
    "supl.cpp"
    "session.cpp"
    "tcp_client.cpp"
    "encode.cpp"
    "decode.cpp"
    "print.cpp"
    "tls.cpp"
)
add_library(dependency::supl ALIAS dependency_supl)
target_include_directories(dependency_supl PUBLIC "include/")
target_link_libraries(dependency_supl PRIVATE asn1::generated::supl asn1::helper)
target_link_libraries(dependency_supl PUBLIC dependency::core)
target_link_libraries(dependency_supl PRIVATE dependency::loglet)

# TLS backends - add additional backends here (mbedTLS, wolfSSL, ...)
if(USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_sources(dependency_supl PRIVATE "tls_openssl.cpp")
    target_link_libraries(dependency_supl PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(dependency_supl PRIVATE USE_OPENSSL)
endif()

setup_target(dependency_supl)

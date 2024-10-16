
add_library(dependency_lpp 
    "session.cpp"
    "transaction.cpp"
    "message.cpp"
    "client.cpp"
    "request_assistance_data.cpp"
    "provide_capabilities.cpp"
    "provide_location_information.cpp"
    "abort.cpp"
    "periodic_session/handler.cpp"
    "periodic_session/assistance_data.cpp"
    "location_information_delivery.cpp"
)
add_library(dependency::lpp ALIAS dependency_lpp)
target_include_directories(dependency_lpp PUBLIC "include/")
target_include_directories(dependency_lpp PRIVATE "./" "include/lpp/")
target_link_libraries(dependency_lpp PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_lpp PRIVATE dependency::loglet)
target_link_libraries(dependency_lpp PRIVATE dependency::supl)
target_link_libraries(dependency_lpp PRIVATE dependency::scheduler)
target_link_libraries(dependency_lpp PUBLIC dependency::core)
target_link_libraries(dependency_lpp PUBLIC dependency::time)

setup_target(dependency_lpp)

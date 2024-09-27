
add_library(dependency_generator_tokoro STATIC 
    "generator.cpp"
    "satellite.cpp"
    "data.cpp"
    "wgs84.cpp"
    "observation.cpp"
    "helper.cpp"
    "ecef.cpp"
)
add_library(dependency::generator::tokoro ALIAS dependency_generator_tokoro)

target_include_directories(dependency_generator_tokoro PRIVATE "./" "include/generator/tokoro/")
target_include_directories(dependency_generator_tokoro PUBLIC "include/")
target_link_libraries(dependency_generator_tokoro PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::core)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::format::nav)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::generator::rtcm)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::maths)

setup_target(dependency_generator_tokoro)

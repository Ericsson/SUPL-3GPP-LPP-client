
add_library(dependency_generator_spartn2 STATIC 
    "spartn.cpp"
    "generator.cpp"
    "ocb.cpp"
    "gad.cpp"
    "hpac.cpp"
    "time.cpp"
    "builder.cpp"
    "message.cpp"
)
add_library(dependency::generator::spartn2 ALIAS dependency_generator_spartn2)

target_include_directories(dependency_generator_spartn2 PRIVATE "./" "include/generator/spartn2/")
target_include_directories(dependency_generator_spartn2 PUBLIC "include/")
target_link_libraries(dependency_generator_spartn2 PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_spartn2 PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_spartn2 PUBLIC dependency::core)

setup_target(dependency_generator_spartn2)

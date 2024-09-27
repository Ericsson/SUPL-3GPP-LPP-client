
add_library(generator_spartn2 STATIC 
    "generator.cpp"
    "ocb.cpp"
    "gad.cpp"
    "hpac.cpp"
    "time.cpp"
    "builder.cpp"
    "message.cpp"
)
add_library(generator::spartn2 ALIAS generator_spartn2)

target_include_directories(generator_spartn2 PRIVATE "./" "include/generator/spartn2/")
target_include_directories(generator_spartn2 PUBLIC "include/")
target_link_libraries(generator_spartn2 PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(generator_spartn2 PRIVATE utility)

if (SPARTN_DEBUG_PRINT)
target_compile_definitions(generator_spartn2 PRIVATE SPARTN_DEBUG_PRINT=2)
endif (SPARTN_DEBUG_PRINT)

setup_target(generator_spartn2)

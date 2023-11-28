
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
target_link_libraries(generator_spartn2 PRIVATE asn1::generated asn1::helper)
target_link_libraries(generator_spartn2 PRIVATE utility)

if (USE_ASAN)
target_compile_options(generator_spartn2 PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(generator_spartn2 PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

if (SPARTN_DEBUG_PRINT)
target_compile_definitions(generator_spartn2 PRIVATE SPARTN_DEBUG_PRINT)
endif (SPARTN_DEBUG_PRINT)

target_compile_options(generator_spartn2 PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wcast-align"
    "-Woverloaded-virtual"
    "-Wsign-conversion"
    "-Wno-conversion"
    "-Wno-old-style-cast"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(generator_spartn2 PRIVATE
        "-Wmisleading-indentation"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(generator_spartn2 PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()
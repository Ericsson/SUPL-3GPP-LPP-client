
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
    "-Werror"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(generator_spartn2 PRIVATE
        "-Weverything"
        "-Wno-switch-enum"
        "-Wno-signed-enum-bitfield"
        "-Wno-deprecated-register"
        "-Wno-c++98-compat"
        "-Wno-c++98-compat-pedantic"
        "-Wno-c++98-c++11-compat-pedantic"
        "-Wno-nested-anon-types"
        "-Wno-gnu-anonymous-struct"
        "-Wno-missing-prototypes"
        "-Wno-documentation"
        "-Wno-documentation-unknown-command"
        "-Wno-weak-vtables"
        "-Wno-unused-const-variable"
        "-Wno-format-nonliteral"
        "-Wno-global-constructors"
        "-Wno-exit-time-destructors"
        "-Wno-padded"
        "-Wno-unsafe-buffer-usage"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(generator_spartn2 PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()
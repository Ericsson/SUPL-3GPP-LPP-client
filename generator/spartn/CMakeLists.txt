
add_library(generator_spartn STATIC 
    "generator.cpp"
    "time.cpp"
    "transmitter.cpp"
)
add_library(generator::spartn ALIAS generator_spartn)

target_include_directories(generator_spartn PRIVATE "./" "include/generator/spartn/")
target_include_directories(generator_spartn PUBLIC "include/")
target_link_libraries(generator_spartn PRIVATE asn1::generated asn1::helper)
target_link_libraries(generator_spartn PRIVATE utility)

if (USE_ASAN)
target_compile_options(generator_spartn PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(generator_spartn PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

target_compile_options(generator_spartn PRIVATE
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
    target_compile_options(generator_spartn PRIVATE
        "-Wmisleading-indentation"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(generator_spartn PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()
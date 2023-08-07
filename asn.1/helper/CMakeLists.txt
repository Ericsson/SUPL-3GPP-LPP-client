
add_library(asn1_helper STATIC 
    "bit_string.cpp"
)
add_library(asn1::helper ALIAS asn1_helper)

target_include_directories(asn1_helper PRIVATE "./" "include/asn.1")
target_include_directories(asn1_helper PUBLIC "include/")
target_link_libraries(asn1_helper PRIVATE asn1::generated)

if (USE_ASAN)
target_compile_options(asn1_helper PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(asn1_helper PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

target_compile_options(asn1_helper PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wcast-align"
    "-Woverloaded-virtual"
    "-Wsign-conversion"
    "-Wno-conversion"
    "-Wmisleading-indentation"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(asn1_helper PRIVATE
        "-Wduplicated-cond"
        "-Wduplicated-branches"
    )
endif()
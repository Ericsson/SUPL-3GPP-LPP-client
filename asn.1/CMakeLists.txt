file(GLOB_RECURSE ASN1_SOURCE "generated/*.c")

add_library(asn1_generated STATIC ${ASN1_SOURCE})
add_library(asn1::generated ALIAS asn1_generated)

target_compile_definitions(asn1_generated PUBLIC "ASN_DISABLE_OER_SUPPORT=1")
target_include_directories(asn1_generated PUBLIC "generated")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(asn1_generated PRIVATE
        "-Wno-parentheses-equality" 
        "-Wno-unused-but-set-variable"
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

endif()

if (USE_ASAN)
target_compile_options(asn1_generated PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(asn1_generated PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

add_subdirectory(helper)

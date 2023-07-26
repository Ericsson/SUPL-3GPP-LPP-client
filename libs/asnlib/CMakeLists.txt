file(GLOB_RECURSE ASNLIB_SOURCE "src/*.c")
add_library (asnlib STATIC ${ASNLIB_SOURCE} "indent.c")
target_compile_definitions(asnlib PUBLIC "ASN_DISABLE_OER_SUPPORT=1")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(asnlib PRIVATE
    "-Wno-parentheses-equality" "-Wno-unused-but-set-variable")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

endif()

target_include_directories(asnlib PUBLIC "src")

if (USE_ASAN)
target_compile_options(asnlib PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(asnlib PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)
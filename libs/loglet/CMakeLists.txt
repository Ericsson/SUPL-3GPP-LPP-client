
add_library(loglet 
    "src/loglet.cpp"
)
target_include_directories(loglet PUBLIC "include/")
target_link_libraries(loglet PRIVATE Threads::Threads atomic)

target_compile_options(loglet PUBLIC "-Wno-unused-private-field")

if (USE_ASAN)
target_compile_options(loglet PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(loglet PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

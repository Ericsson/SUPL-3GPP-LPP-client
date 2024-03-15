
add_library(lpp 
    "src/session.cpp"
    "src/transaction.cpp"
    "src/message.cpp"
)
target_include_directories(lpp PUBLIC "include/")
target_link_libraries(lpp PRIVATE Threads::Threads atomic)
target_link_libraries(lpp PRIVATE asn1::generated asn1::helper)
target_link_libraries(lpp PRIVATE loglet supl scheduler)

target_compile_options(lpp PUBLIC "-Wno-unused-private-field")

if (USE_ASAN)
target_compile_options(lpp PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(lpp PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

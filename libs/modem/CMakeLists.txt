list(APPEND modem_SOURCES 
    "include/modem.h"
    "src/modem.cpp"
)

add_library(modem ${modem_SOURCES})
target_include_directories(modem PUBLIC "include/")
target_link_libraries(modem PRIVATE utility lpplib Threads::Threads atomic)

target_compile_options(modem PUBLIC "-Wno-unused-private-field")

if (USE_ASAN)
target_compile_options(modem PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(modem PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

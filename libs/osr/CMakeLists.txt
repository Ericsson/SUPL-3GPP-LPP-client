list(APPEND OSR_SOURCES 
    "include/osr/osr.h"
    "include/osr/lpp2osr.h"
    "include/osr/osr2rtklib.h"
    "src/osr.cpp"
    "src/lpp2osr.cpp"
    "src/osr2rtklib.cpp"
)

add_library(osr ${OSR_SOURCES})
target_include_directories(osr PUBLIC "include/")
target_link_libraries(osr PRIVATE lpplib asnlib rtklib utility Threads::Threads)

if (USE_ASAN)
target_compile_options(osr PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(osr PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

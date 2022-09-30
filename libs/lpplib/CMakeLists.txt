list(APPEND LPP_SOURCES 
    "include/asn_helper.h"
    "include/asnlib.h"
    "include/internal_lpp.h"
    "include/lpp.h"
    "include/supl.h"
    "include/tcp.h"
    "src/asn_helper.cpp"
    "src/assistance_data.cpp"
    "src/capabilities.cpp"
    "src/gather.cpp"
    "src/internal_lpp.cpp"
    "src/lpp.cpp"
    "src/supl.cpp"
    "src/tcp.cpp"
)

add_library(lpplib ${LPP_SOURCES})
target_include_directories(lpplib PUBLIC "include/")
target_link_libraries(lpplib PRIVATE asnlib utility Threads::Threads)

if (USE_OPENSSL)
target_link_libraries(lpplib PRIVATE OpenSSL::SSL)
target_compile_definitions(lpplib PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(lpplib PRIVATE -fsanitize=address)
target_link_libraries(lpplib PRIVATE -fsanitize=address)
endif (USE_ASAN)

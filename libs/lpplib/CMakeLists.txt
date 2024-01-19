list(APPEND LPP_SOURCES 
    "include/lpp/asn_helper.h"
    "include/lpp/asnlib.h"
    "include/lpp/internal_lpp.h"
    "include/lpp/lpp.h"
    "include/lpp/supl.h"
    "include/lpp/tcp.h"
    "include/lpp/location_information.h"
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
target_include_directories(lpplib PRIVATE "include/lpp/")
target_link_libraries(lpplib PRIVATE utility Threads::Threads)
target_link_libraries(lpplib PRIVATE asn1::generated asn1::helper)

if (USE_OPENSSL)
target_link_libraries(lpplib PRIVATE OpenSSL::SSL)
target_compile_definitions(lpplib PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

if (USE_ASAN)
target_compile_options(lpplib PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(lpplib PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

if (DEBUG_LPP_LIB)
target_compile_definitions(lpplib PRIVATE "DEBUG_LPP_LIB=1")
endif (DEBUG_LPP_LIB)

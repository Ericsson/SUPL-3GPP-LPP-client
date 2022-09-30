list(APPEND OSR_SOURCES 
    "include/rtcm_generator.h"
    "include/osr2rtcm.h"
    "src/rtcm_generator.cpp"
    "src/osr2rtcm.cpp"
)

add_library(rtcm_generator ${OSR_SOURCES})
target_include_directories(rtcm_generator PUBLIC "include/")
target_link_libraries(rtcm_generator PRIVATE osr lpplib asnlib rtklib utility Threads::Threads)

if (USE_ASAN)
target_compile_options(rtcm_generator PRIVATE -fsanitize=address)
target_link_libraries(rtcm_generator PRIVATE -fsanitize=address)
endif (USE_ASAN)

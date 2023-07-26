list(APPEND UTILITY_SOURCES 
    "include/utility/types.h"
    "include/utility/gps_time.h"
    "include/utility/mutex.h"
    "include/utility/optional.h"
    "include/utility/sem.h"
    "include/utility/signal_id.h"
    "include/utility/satellite_id.h"
    "src/signal_id.cpp"
    "src/satellite_id.cpp"
    "src/time/bdt_time.cpp"
    "src/time/glo_time.cpp"
    "src/time/gps_time.cpp"
    "src/time/gst_time.cpp"
    "src/time/tai_time.cpp"
    "src/time/utc_time.cpp"
)

add_library(utility ${UTILITY_SOURCES})
target_include_directories(utility PUBLIC "include/")
target_link_libraries(utility PRIVATE Threads::Threads)

if (USE_ASAN)
target_compile_options(utility PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(utility PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

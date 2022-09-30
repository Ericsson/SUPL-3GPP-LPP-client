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
)

add_library(utility ${UTILITY_SOURCES})
target_include_directories(utility PUBLIC "include/")
target_link_libraries(utility PRIVATE Threads::Threads)

if (USE_ASAN)
target_compile_options(utility PRIVATE -fsanitize=address)
target_link_libraries(utility PRIVATE -fsanitize=address)
endif (USE_ASAN)

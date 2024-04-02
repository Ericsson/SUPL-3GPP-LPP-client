
add_library(scheduler 
    "src/scheduler.cpp"
    "src/task.cpp"
    "src/periodic_task.cpp"
)
target_include_directories(scheduler PUBLIC "include/")
target_link_libraries(scheduler PRIVATE Threads::Threads atomic)
target_link_libraries(scheduler PRIVATE loglet)

target_compile_options(scheduler PUBLIC "-Wno-unused-private-field")

if (USE_ASAN)
target_compile_options(scheduler PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(scheduler PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

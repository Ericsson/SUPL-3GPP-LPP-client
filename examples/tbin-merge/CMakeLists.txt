add_executable(tbin_merge main.cpp)
target_link_libraries(tbin_merge PRIVATE dependency::format::tbin)
setup_target(tbin_merge)
set_target_properties(tbin_merge PROPERTIES OUTPUT_NAME "tbin-merge")

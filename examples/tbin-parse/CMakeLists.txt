add_executable(tbin_parse main.cpp)
target_link_libraries(tbin_parse PRIVATE
    dependency::format::tbin
    dependency::format::lpp
    dependency::format::helper
    dependency::core
)
setup_target(tbin_parse)
set_target_properties(tbin_parse PROPERTIES OUTPUT_NAME "tbin-parse")


add_library(dependency_format_nav STATIC 
    "nav.cpp"
    "gps/lnav.cpp"
    "gal/inav.cpp"
    "d1.cpp"
)
add_library(dependency::format::nav ALIAS dependency_format_nav)

target_include_directories(dependency_format_nav PRIVATE "./" "include/format/nav/")
target_include_directories(dependency_format_nav PUBLIC "include/")
target_link_libraries(dependency_format_nav PUBLIC dependency::format::helper)
target_link_libraries(dependency_format_nav PUBLIC dependency::loglet)
target_link_libraries(dependency_format_nav PUBLIC dependency::time)
target_link_libraries(dependency_format_nav PUBLIC dependency::maths)
target_link_libraries(dependency_format_nav PUBLIC dependency::core)
target_link_libraries(dependency_format_nav PUBLIC dependency::ephemeris)

setup_target(dependency_format_nav)

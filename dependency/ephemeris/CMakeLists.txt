
add_library(dependency_ephemeris STATIC 
    "gps.cpp"
    "gal.cpp"
    "bds.cpp"
    "qzs.cpp"
)
add_library(dependency::ephemeris ALIAS dependency_ephemeris)
target_include_directories(dependency_ephemeris PRIVATE "./" "include/ephemeris/")
target_include_directories(dependency_ephemeris PUBLIC "include/")
target_link_libraries(dependency_ephemeris PRIVATE dependency::loglet)
target_link_libraries(dependency_ephemeris PUBLIC dependency::core)
target_link_libraries(dependency_ephemeris PUBLIC dependency::maths)
target_link_libraries(dependency_ephemeris PUBLIC dependency::time)
target_link_libraries(dependency_ephemeris PUBLIC dependency::msgpack)

setup_target(dependency_ephemeris)

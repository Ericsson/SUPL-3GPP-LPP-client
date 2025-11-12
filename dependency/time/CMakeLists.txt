
add_library(dependency_time STATIC 
    "gps.cpp"
    "glo.cpp"
    "gst.cpp"
    "bdt.cpp"
    "tai.cpp"
    "utc.cpp"
    "timestamp.cpp"
)

add_library(dependency::time ALIAS dependency_time)
target_include_directories(dependency_time PRIVATE "./" "include/time/")
target_include_directories(dependency_time PUBLIC "include/")
target_link_libraries(dependency_time PUBLIC dependency::core)
target_link_libraries(dependency_time PRIVATE dependency::loglet)

setup_target(dependency_time)



add_library(dependency_streamline STATIC 
    "streamline.cpp"
)

add_library(dependency::streamline ALIAS dependency_streamline)
target_include_directories(dependency_streamline PRIVATE "./" "include/streamline/")
target_include_directories(dependency_streamline PUBLIC "include/")
target_link_libraries(dependency_streamline PUBLIC dependency::scheduler)
target_link_libraries(dependency_streamline PUBLIC dependency::loglet)
target_link_libraries(dependency_streamline PUBLIC dependency::core)

setup_target(dependency_streamline)

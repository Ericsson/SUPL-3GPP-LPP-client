

add_library(dependency_datatrace "datatrace.cpp")
add_library(dependency::datatrace ALIAS dependency_datatrace)
target_include_directories(dependency_datatrace PRIVATE "./" "include/datatrace/")
target_include_directories(dependency_datatrace PUBLIC "include/")
target_link_libraries(dependency_datatrace PUBLIC dependency::loglet)
target_link_libraries(dependency_datatrace PUBLIC dependency::core)
target_link_libraries(dependency_datatrace PUBLIC dependency::time)
target_link_libraries(dependency_datatrace PUBLIC dependency::maths)


if(CMAKE_CROSSCOMPILING)    
    target_link_libraries(dependency_datatrace PUBLIC ${LIBMOSQUITTO_LIBRARIES})
    target_include_directories(dependency_datatrace PRIVATE ${LIBMOSQUITTO_INCLUDE_DIRS})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)
    target_link_libraries(dependency_datatrace PUBLIC PkgConfig::Mosquitto)
endif()


setup_target(dependency_datatrace)

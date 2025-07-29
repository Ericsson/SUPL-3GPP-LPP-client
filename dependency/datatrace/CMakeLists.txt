

add_library(dependency_datatrace "datatrace.cpp")
add_library(dependency::datatrace ALIAS dependency_datatrace)
target_include_directories(dependency_datatrace PRIVATE "./" "include/datatrace/")
target_include_directories(dependency_datatrace PUBLIC "include/")
target_link_libraries(dependency_datatrace PUBLIC dependency::loglet)
target_link_libraries(dependency_datatrace PUBLIC dependency::core)
target_link_libraries(dependency_datatrace PUBLIC dependency::time)
target_link_libraries(dependency_datatrace PUBLIC dependency::maths)

# TODO: Switch back to original repo
# URI "gh:eclipse-mosquitto/mosquitto#v2.0.22"
CPMAddPackage(
    URI "gh:besmarsh/mosquitto#fix-cmake-builds-without-tls"
    OPTIONS 
        "WITH_CLIENTS OFF"
        "WITH_BROKER OFF"
        "WITH_APPS OFF"
        "WITH_PLUGINS OFF"
        "DOCUMENTATION OFF"
        "WITH_TLS OFF"
        "WITH_TLS_PSK OFF"
        "WITH_EC OFF"
        "WITH_UNIX_SOCKETS OFF"
        "WITH_SOCKS OFF"
        "WITH_STATIC_LIBRARIES ON"
        "WITH_PIC ON"
)

target_link_libraries(dependency_datatrace PUBLIC libmosquitto_static)
target_include_directories(dependency_datatrace PUBLIC ${mosquitto_SOURCE_DIR}/include)

setup_target(dependency_datatrace)

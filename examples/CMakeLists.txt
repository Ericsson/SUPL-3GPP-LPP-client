
add_subdirectory("ntrip")
add_subdirectory("ctrl_toggle")
add_subdirectory("modem_ctrl")
add_subdirectory("client")
if(BUILD_EXAMPLE_TRANSFORM)
    add_subdirectory("transform")
endif()

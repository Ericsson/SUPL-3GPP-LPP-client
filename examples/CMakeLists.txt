
add_subdirectory("ublox")
add_subdirectory("nmea")
add_subdirectory("lpp")
add_subdirectory("lpp2spartn")
add_subdirectory("ntrip")
add_subdirectory("ctrl_switch")

if(${EXPERIMENTAL})
add_subdirectory("client")
endif()

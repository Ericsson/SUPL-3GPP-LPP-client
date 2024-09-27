
if(${INCLUDE_GENERATOR_RTCM})
add_subdirectory("rtcm")
endif()

if(${INCLUDE_GENERATOR_SPARTN})
add_subdirectory("spartn")
endif()

if(${INCLUDE_GENERATOR_TOKORO})
add_subdirectory("tokoro")
endif()

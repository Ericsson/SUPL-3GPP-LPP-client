
if(${INCLUDE_GENERATOR_RTCM})
add_subdirectory("rtcm")
endif()

if(${INCLUDE_GENERATOR_SPARTN})
add_subdirectory("spartn2")
endif()

if(${INCLUDE_GENERATOR_SPARTN_OLD})
add_subdirectory("spartn")
endif()


add_library(dependency_generator_tokoro STATIC 
    "generator.cpp"
    "satellite.cpp"
    "data.cpp"
    "observation.cpp"

    "models/helper.cpp"
    "models/gpt.cpp"
    "models/mops.cpp"
    "models/geoid.cpp"
    "models/shapiro.cpp"
    "models/earth_solid_tides.cpp"
    "models/phase_windup.cpp"
    "models/antenna_phase_variation.cpp"
    "models/astronomical_arguments.cpp"
    "models/nutation.cpp"
    "models/sun_moon.cpp"
    
    "coordinates/coordinate.cpp"
    "coordinates/reference_ellipsoid.cpp"
    "coordinates/enu.cpp"
    "coordinates/wgs84.cpp"
    "coordinates/ecef.cpp"
    "coordinates/eci.cpp"
)
add_library(dependency::generator::tokoro ALIAS dependency_generator_tokoro)

target_include_directories(dependency_generator_tokoro PRIVATE "./" "include/generator/tokoro/")
target_include_directories(dependency_generator_tokoro PUBLIC "include/")
target_link_libraries(dependency_generator_tokoro PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::core)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::format::nav)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::generator::rtcm)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::maths)

if(${DATA_TRACING})
    target_link_libraries(dependency_generator_tokoro PRIVATE dependency::datatrace)
endif()

setup_target(dependency_generator_tokoro)

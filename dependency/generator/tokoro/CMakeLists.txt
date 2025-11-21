
add_library(dependency_generator_tokoro STATIC 
    "generator.cpp"
    "satellite.cpp"
    "data.cpp"
    "observation.cpp"
    "decode.cpp"

    "models/helper.cpp"
    "models/mops.cpp"
    "models/geoid.cpp"
    "models/shapiro.cpp"
    "models/earth_solid_tides.cpp"
    "models/phase_windup.cpp"
    "models/antenna_phase_variation.cpp"
    "models/astronomical_arguments.cpp"
    "models/nutation.cpp"
    "models/sun_moon.cpp"
    "models/gpt.cpp"
    
    "coordinates/coordinate.cpp"
    "coordinates/reference_ellipsoid.cpp"
    "coordinates/enu.cpp"
    "coordinates/wgs84.cpp"
    "coordinates/ecef.cpp"
    "coordinates/eci.cpp"

    "data/orbit.cpp"
    "data/clock.cpp"
    "data/grid.cpp"
    "data/set.cpp"
    "data/ionosphere.cpp"
    "data/troposphere.cpp"
    "data/gather.cpp"
)
add_library(dependency::generator::tokoro ALIAS dependency_generator_tokoro)

target_include_directories(dependency_generator_tokoro PRIVATE "./" "include/generator/tokoro/")
target_include_directories(dependency_generator_tokoro PUBLIC "include/")
target_link_libraries(dependency_generator_tokoro PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::core)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::format::nav)
if(INCLUDE_FORMAT_RINEX)
    target_link_libraries(dependency_generator_tokoro PUBLIC dependency::format::rinex)
    target_compile_definitions(dependency_generator_tokoro PRIVATE "INCLUDE_FORMAT_RINEX=1")
endif()
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::format::antex)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::maths)
target_link_libraries(dependency_generator_tokoro PUBLIC dependency::generator::rtcm)

if(DATA_TRACING)
    target_link_libraries(dependency_generator_tokoro PRIVATE dependency::datatrace)
endif()

setup_target(dependency_generator_tokoro)

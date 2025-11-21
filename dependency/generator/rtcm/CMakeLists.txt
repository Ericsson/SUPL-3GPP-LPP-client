
add_library(dependency_generator_rtcm STATIC 
    "rtcm.cpp"
    "generator.cpp"
    "satellite_id.cpp"
    "signal_id.cpp"
    "extract/extract_common_observation_info.cpp"
    "extract/extract_reference_station_info.cpp"
    "extract/extract_observations.cpp"
    "extract/extract_bias_information.cpp"
    "extract/residuals.cpp"
    "extract/auxiliary_information.cpp"
    "extract/extract.cpp"
    "lowest_msm.cpp"
    "encoder.cpp"
    "messages/msm.cpp"
    "messages/1230.cpp"
    "messages/residuals.cpp"
    "messages/reference_station.cpp"
)
add_library(dependency::generator::rtcm ALIAS dependency_generator_rtcm)

target_include_directories(dependency_generator_rtcm PRIVATE "./" "include/generator/rtcm/")
target_include_directories(dependency_generator_rtcm PUBLIC "include/")
target_link_libraries(dependency_generator_rtcm PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_rtcm PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_rtcm PUBLIC dependency::time)
target_link_libraries(dependency_generator_rtcm PUBLIC dependency::core)

if(DATA_TRACING)
    target_link_libraries(dependency_generator_rtcm PRIVATE dependency::datatrace)
endif()

target_compile_definitions(dependency_generator_rtcm PUBLIC "INCLUDE_GENERATOR_RTCM=1")

setup_target(dependency_generator_rtcm)


add_library(generator_rtcm STATIC 
    "generator.cpp"
    "satellite_id.cpp"
    "signal_id.cpp"
    "extract/extract_common_observation_info.cpp"
    "extract/extract_reference_station_info.cpp"
    "extract/extract_observations.cpp"
    "extract/extract_bias_information.cpp"
    "extract/residuals.cpp"
    "extract/auxiliary_information.cpp"
    "time/tai_time.cpp"
    "time/gps_time.cpp"
    "time/utc_time.cpp"
    "time/bdt_time.cpp"
    "time/gst_time.cpp"
    "time/glo_time.cpp"
    "lowest_msm.cpp"
    "encoder.cpp"
    "messages/msm.cpp"
    "messages/1230.cpp"
    "messages/residuals.cpp"
    "messages/reference_station.cpp"
)
add_library(generator::rtcm ALIAS generator_rtcm)

target_include_directories(generator_rtcm PRIVATE "./" "include/generator/rtcm/")
target_include_directories(generator_rtcm PUBLIC "include/")
target_link_libraries(generator_rtcm PRIVATE asn1::generated asn1::helper)

if (USE_ASAN)
target_compile_options(generator_rtcm PRIVATE -fsanitize=address,undefined,leak)
target_link_libraries(generator_rtcm PRIVATE -fsanitize=address,undefined,leak)
endif (USE_ASAN)

target_compile_options(generator_rtcm PRIVATE
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wcast-align"
    "-Woverloaded-virtual"
    "-Wsign-conversion"
    "-Wno-conversion"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(generator_rtcm PRIVATE
        "-Wmisleading-indentation"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(generator_rtcm PRIVATE
        "-Wno-missing-field-initializers"
    )
endif()

add_executable(example_client 
    "main.cpp" 
    "config.cpp"
    "config/assistance_data.cpp"
    "config/data_tracing.cpp"
    "config/gnss.cpp"
    "config/identity.cpp"
    "config/input.cpp"
    "config/location_information.cpp"
    "config/location_server.cpp"
    "config/logging.cpp"
    "config/output.cpp"
    "config/print.cpp"
    "config/scheduler.cpp"
    "config/stream.cpp"
    "config/ubx_config.cpp"
    "processor/lpp.cpp"
    "processor/control.cpp"
    "processor/location_information.cpp"
    "processor/ubx.cpp"
    "processor/ubx_config.cpp"
    "processor/nmea.cpp"
    "processor/rtcm.cpp"
    "processor/raw.cpp"
    "processor/test.cpp"
    "processor/possib_logger.cpp"
    "processor/tlf.cpp"
    "processor/chunked_log.cpp"
)
add_executable(examples::client ALIAS example_client)

target_include_directories(example_client PRIVATE "./")
target_link_libraries(example_client PRIVATE args)
target_link_libraries(example_client PRIVATE dependency::streamline)
target_link_libraries(example_client PRIVATE dependency::io)
target_link_libraries(example_client PRIVATE dependency::format::ubx)
target_link_libraries(example_client PRIVATE dependency::format::nmea)
target_link_libraries(example_client PRIVATE dependency::format::rtcm)
target_link_libraries(example_client PRIVATE dependency::format::ctrl)
target_link_libraries(example_client PRIVATE dependency::format::lpp)
target_link_libraries(example_client PRIVATE dependency::scheduler)
target_link_libraries(example_client PRIVATE dependency::loglet)
target_link_libraries(example_client PRIVATE dependency::core)
target_link_libraries(example_client PRIVATE dependency::supl)
target_link_libraries(example_client PRIVATE dependency::lpp)
target_link_libraries(example_client PRIVATE dependency::time)
target_link_libraries(example_client PRIVATE asn1::generated::lpp asn1::helper)

if(INCLUDE_GENERATOR_RTCM)
    target_sources(example_client PRIVATE
        "config/lpp2frame_rtcm.cpp"
        "config/lpp2rtcm.cpp"
        "config/lpp2eph.cpp"
        "config/ubx2eph.cpp"
        "config/rtcm2eph.cpp"
        "processor/lpp2rtcm.cpp"
        "processor/lpp2frame_rtcm.cpp"
        "processor/lpp2eph.cpp"
        "processor/ubx2eph.cpp"
        "processor/rtcm2eph.cpp"
    )
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_RTCM=1")
    target_link_libraries(example_client PRIVATE dependency::generator::rtcm)
    target_link_libraries(example_client PRIVATE dependency::ephemeris)
    target_link_libraries(example_client PRIVATE dependency::format::nav)
endif()

if(INCLUDE_GENERATOR_SPARTN)
    target_sources(example_client PRIVATE
        "config/lpp2spartn.cpp"
        "processor/lpp2spartn.cpp"
    )
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_SPARTN=1")
    target_link_libraries(example_client PRIVATE dependency::generator::spartn2)
endif()

if(INCLUDE_GENERATOR_TOKORO)
    target_sources(example_client PRIVATE
        "config/agnss.cpp"
        "config/tokoro.cpp"
        "processor/agnss.cpp"
        "processor/tokoro.cpp"
    )
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_TOKORO=1")
    target_link_libraries(example_client PRIVATE dependency::format::nav)
    if(INCLUDE_FORMAT_ANTEX)
        target_link_libraries(example_client PRIVATE dependency::format::antex)
    endif()
    target_link_libraries(example_client PRIVATE dependency::generator::tokoro)
    
    if(ENABLE_TOKORO_SNAPSHOT)
        target_sources(example_client PRIVATE
            "snapshot/tokoro_snapshot.cpp"
        )
        target_link_libraries(example_client PRIVATE dependency::msgpack)
    endif()
endif()

if(INCLUDE_GENERATOR_IDOKEIDO)
    target_sources(example_client PRIVATE
        "config/idokeido.cpp"
        "processor/idokeido.cpp"
    )
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_IDOKEIDO=1")
    target_link_libraries(example_client PRIVATE dependency::format::nav)
    if(INCLUDE_FORMAT_ANTEX)
        target_link_libraries(example_client PRIVATE dependency::format::antex)
    endif()
    target_link_libraries(example_client PRIVATE dependency::generator::idokeido)
endif()

if(DATA_TRACING)
    target_link_libraries(example_client PRIVATE dependency::datatrace)
endif()

set_target_properties(example_client PROPERTIES OUTPUT_NAME "example-client")
set_target_properties(example_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_client)

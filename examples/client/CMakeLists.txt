
add_executable(example_client 
    "main.cpp" 
    "config.cpp"
    "processor/lpp.cpp"
    "processor/control.cpp"
    "processor/location_information.cpp"
    "processor/ubx.cpp"
    "processor/nmea.cpp"
    "processor/lpp2rtcm.cpp"
    "processor/lpp2frame_rtcm.cpp"
    "processor/lpp2spartn.cpp"
    "processor/tokoro.cpp"
    "processor/test.cpp"
    "processor/possib_logger.cpp"
    "processor/idokeido.cpp"
    "processor/tlf.cpp"
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
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_RTCM=1")
    target_link_libraries(example_client PRIVATE dependency::generator::rtcm)
endif()

if(INCLUDE_GENERATOR_SPARTN)
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_SPARTN=1")
    target_link_libraries(example_client PRIVATE dependency::generator::spartn2)
endif()

if(INCLUDE_GENERATOR_TOKORO)
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_TOKORO=1")
    target_link_libraries(example_client PRIVATE dependency::format::nav)
    target_link_libraries(example_client PRIVATE dependency::format::antex)
    target_link_libraries(example_client PRIVATE dependency::generator::tokoro)
endif()

if(INCLUDE_GENERATOR_IDOKEIDO)
    target_compile_definitions(example_client PRIVATE "INCLUDE_GENERATOR_IDOKEIDO=1")
    target_link_libraries(example_client PRIVATE dependency::format::nav)
    target_link_libraries(example_client PRIVATE dependency::format::antex)
    target_link_libraries(example_client PRIVATE dependency::generator::idokeido)
endif()

if(DATA_TRACING)
    target_link_libraries(example_client PRIVATE dependency::datatrace)
endif()

set_target_properties(example_client PROPERTIES OUTPUT_NAME "example-client")
set_target_properties(example_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

setup_target(example_client)

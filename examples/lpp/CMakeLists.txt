

add_executable(example_lpp 
    "main.cpp" 
    "osr_example.cpp" 
    "ssr_example.cpp" 
    "agnss_example.cpp" 
    "location_information.cpp"
    "options.cpp"
    "processor/nmea.cpp"
    "processor/ubx.cpp"
    "processor/lpp.cpp"
    "processor/lpp2spartn.cpp"
    "processor/ctrl.cpp"
    "processor/lpp2rtcm.cpp"
    "processor/tokoro.cpp"
)
add_executable(examples::lpp ALIAS example_lpp)

target_include_directories(example_lpp PRIVATE "./")
target_include_directories(example_lpp PRIVATE "./include")
target_link_libraries(example_lpp PRIVATE lpplib Threads::Threads)
target_link_libraries(example_lpp PRIVATE args)
target_link_libraries(example_lpp PRIVATE dependency::streamline)
target_link_libraries(example_lpp PRIVATE dependency::io)
target_link_libraries(example_lpp PRIVATE dependency::format::ubx)
target_link_libraries(example_lpp PRIVATE dependency::format::nmea)
target_link_libraries(example_lpp PRIVATE dependency::format::ctrl)
target_link_libraries(example_lpp PRIVATE dependency::format::lpp)
target_link_libraries(example_lpp PRIVATE asn1::generated::lpp asn1::helper)

if(${INCLUDE_GENERATOR_RTCM})
    target_compile_definitions(example_lpp PRIVATE "INCLUDE_GENERATOR_RTCM=1")
    target_link_libraries(example_lpp PRIVATE dependency::generator::rtcm)
endif()

if(${INCLUDE_GENERATOR_SPARTN})
    target_compile_definitions(example_lpp PRIVATE "INCLUDE_GENERATOR_SPARTN=1")
    target_link_libraries(example_lpp PRIVATE dependency::generator::spartn2)
endif()

if(${INCLUDE_GENERATOR_TOKORO})
    target_compile_definitions(example_lpp PRIVATE "INCLUDE_GENERATOR_TOKORO=1")
    target_link_libraries(example_lpp PRIVATE dependency::format::nav)
    target_link_libraries(example_lpp PRIVATE dependency::generator::tokoro)
endif()

if(${DATA_TRACING})
    target_link_libraries(example_lpp PRIVATE dependency::datatrace)
endif()

set_target_properties(example_lpp PROPERTIES OUTPUT_NAME "example-lpp")
set_target_properties(example_lpp PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (USE_OPENSSL)
target_link_libraries(example_lpp PRIVATE OpenSSL::SSL)
target_compile_definitions(example_lpp PRIVATE "USE_OPENSSL=1")
endif (USE_OPENSSL)

setup_target(example_lpp)

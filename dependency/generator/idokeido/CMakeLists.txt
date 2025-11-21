
add_library(dependency_generator_idokeido STATIC 
    "satellite_position.cpp"
    "idokeido.cpp"
    "spp.cpp"
    "eph.cpp"
    "clock.cpp"
    "orbit.cpp"
    "ionosphere.cpp"
    "correction.cpp"
    "decode.cpp"
    "coordinates/enu.cpp"
    "coordinates/look_angles.cpp"
    "coordinates/ecef.cpp"
    "coordinates/reference_ellipsoid.cpp"
    "models/klobuchar.cpp"
)
add_library(dependency::generator::idokeido ALIAS dependency_generator_idokeido)

target_include_directories(dependency_generator_idokeido PRIVATE "./" "include/generator/idokeido/")
target_include_directories(dependency_generator_idokeido PUBLIC "include/")
target_link_libraries(dependency_generator_idokeido PRIVATE asn1::generated::lpp asn1::helper)
target_link_libraries(dependency_generator_idokeido PUBLIC dependency::loglet)
target_link_libraries(dependency_generator_idokeido PUBLIC dependency::core)
target_link_libraries(dependency_generator_idokeido PUBLIC dependency::format::nav)
if(INCLUDE_FORMAT_RINEX)
    target_link_libraries(dependency_generator_idokeido PUBLIC dependency::format::rinex)
endif()
if(INCLUDE_FORMAT_ANTEX)
    target_link_libraries(dependency_generator_idokeido PUBLIC dependency::format::antex)
endif()

CPMAddPackage(
    GITLAB_REPOSITORY libeigen/eigen
    GIT_TAG 3.4
    OPTIONS "EIGEN_BUILD_DOC OFF"
    "EIGEN_BUILD_PKGCONFIG OF"
    "EIGEN_BUILD_DOC OFF"
    SYSTEM YES
)

target_link_libraries(dependency_generator_idokeido PUBLIC Eigen3::Eigen)

if(INCLUDE_GENERATOR_TOKORO)
    target_compile_definitions(dependency_generator_idokeido PRIVATE "INCLUDE_GENERATOR_TOKORO=1")
    target_link_libraries(dependency_generator_idokeido PRIVATE dependency::generator::tokoro)
endif()

if(DATA_TRACING)
    target_link_libraries(dependency_generator_idokeido PRIVATE dependency::datatrace)
endif()

setup_target(dependency_generator_idokeido)
target_compile_options(dependency_generator_idokeido PRIVATE -Wno-c++20-designator)

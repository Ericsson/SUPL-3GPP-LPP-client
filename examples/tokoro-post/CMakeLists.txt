add_executable(tokoro-post main.cpp)
target_link_libraries(tokoro-post PRIVATE
    dependency::generator::tokoro
    dependency::generator::rtcm
    dependency::format::tbin
    dependency::format::ubx
    dependency::format::lpp
    dependency::format::rinex
    dependency::format::nav
    dependency::ephemeris
    dependency::time
    dependency::loglet
    dependency::core
    dependency::maths
    dependency::gnss
    asn1_lpp_generated
    asn1_helper
)
if(INCLUDE_FORMAT_ANTEX)
    target_link_libraries(tokoro-post PRIVATE dependency::format::antex)
    target_compile_definitions(tokoro-post PRIVATE "INCLUDE_FORMAT_ANTEX=1")
endif()
setup_target(tokoro-post)

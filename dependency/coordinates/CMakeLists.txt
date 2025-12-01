add_library(coordinates
    src/ecef_llh.cpp
    src/ecef_enu.cpp
    src/ecef_ned.cpp
    src/ecef_aer.cpp
)

target_include_directories(coordinates PUBLIC include)
target_link_libraries(coordinates PUBLIC Eigen3::Eigen)

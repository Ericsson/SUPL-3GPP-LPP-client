if(NOT TARGET Eigen3::Eigen)
    if(COMMAND CPMAddPackage)
        CPMAddPackage(
            GITLAB_REPOSITORY libeigen/eigen
            GIT_TAG 3.4
            OPTIONS "EIGEN_BUILD_DOC OFF"
            "EIGEN_BUILD_PKGCONFIG OFF"
            SYSTEM YES
        )
    else()
        find_package(Eigen3 QUIET)
        if(NOT Eigen3_FOUND)
            message(STATUS "Eigen3 not found, will enable CPM")
            set(USE_CPM ON PARENT_SCOPE)
            include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
            CPMAddPackage(
                GITLAB_REPOSITORY libeigen/eigen
                GIT_TAG 3.4
                OPTIONS "EIGEN_BUILD_DOC OFF"
                "EIGEN_BUILD_PKGCONFIG OFF"
                SYSTEM YES
            )
        endif()
    endif()
endif()

add_library(dependency_coordinates STATIC
    src/ecef_llh.cpp
    src/ecef_enu.cpp
    src/ecef_ned.cpp
    src/ecef_aer.cpp
    src/frame.cpp
    src/frame_id.cpp
    src/transform.cpp
    src/transform_graph.cpp
    src/transform/sweref99.cpp
    src/transform/itrf.cpp
    src/transform/etrf.cpp
    src/model/nkgrf17vel.cpp
)

add_library(dependency::coordinates ALIAS dependency_coordinates)
target_include_directories(dependency_coordinates PUBLIC include)
target_link_libraries(dependency_coordinates PUBLIC Eigen3::Eigen)
target_link_libraries(dependency_coordinates PRIVATE dependency::loglet)
setup_target(dependency_coordinates)

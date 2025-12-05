if(NOT TARGET Eigen3::Eigen)
    CPMAddPackage(
        GITLAB_REPOSITORY libeigen/eigen
        GIT_TAG 3.4
        OPTIONS "EIGEN_BUILD_DOC OFF"
        "EIGEN_BUILD_PKGCONFIG OFF"
        SYSTEM YES
    )
endif()

if(NOT TARGET taywee::args)
    CPMAddPackage(
        NAME args
        GITHUB_REPOSITORY Taywee/args
        GIT_TAG 6.4.7
        OPTIONS "ARGS_BUILD_EXAMPLE OFF"
        "ARGS_BUILD_UNITTESTS OFF"
    )
endif()

if(NOT TARGET args)
    add_library(args ALIAS taywee::args)
endif()

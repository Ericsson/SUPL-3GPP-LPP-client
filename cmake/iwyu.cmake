option(ENABLE_IWYU "Enable include-what-you-use analysis" OFF)

if(ENABLE_IWYU)
    find_program(IWYU_EXE NAMES include-what-you-use iwyu)
    if(IWYU_EXE)
        message(STATUS "include-what-you-use found: ${IWYU_EXE}")
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_EXE}"
            "-Xiwyu" "--mapping_file=${CMAKE_SOURCE_DIR}/cmake/iwyu.imp"
            "-Xiwyu" "--transitive_includes_only")
    else()
        message(WARNING "include-what-you-use requested but not found")
    endif()
endif()

option(ENABLE_CPPCHECK "Enable cppcheck analysis" OFF)

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES cppcheck)
    if(CPPCHECK_EXE)
        message(STATUS "cppcheck found: ${CPPCHECK_EXE}")
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE}"
            "--enable=warning,style,performance,portability"
            "--suppress=*:${CMAKE_SOURCE_DIR}/external/*"
            "--suppress=*:${CMAKE_BINARY_DIR}/*"
            "--inline-suppr"
            "--quiet")
    else()
        message(WARNING "cppcheck requested but not found")
    endif()
endif()

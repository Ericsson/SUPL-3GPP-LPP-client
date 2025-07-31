
add_library(dependency_loglet 
    "loglet.cpp"
)
add_library(dependency::loglet ALIAS dependency_loglet)
target_include_directories(dependency_loglet PUBLIC "include/")
target_link_libraries(dependency_loglet PRIVATE Threads::Threads atomic)
target_link_libraries(dependency_loglet PUBLIC dependency::core)

if(DISABLE_TRACE)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_TRACE=1")
endif()

if(DISABLE_VERBOSE)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_VERBOSE=1")
endif()

if(DISABLE_DEBUG)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_DEBUG=1")
endif()

if(DISABLE_INFO)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_INFO=1")
endif()

if(DISABLE_NOTICE)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_NOTICE=1")
endif()

if(DISABLE_WARNING)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_WARNING=1")
endif()

if(DISABLE_ERROR)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_ERROR=1")
endif()

if(DISABLE_LOGGING)
    target_compile_definitions(dependency_loglet PUBLIC "DISABLE_LOGGING=1")
endif()

if(LOG_FUNCTION_PERFORMANCE)
    target_compile_definitions(dependency_loglet PUBLIC "FUNCTION_PERFORMANCE=1")
endif()

setup_target(dependency_loglet)

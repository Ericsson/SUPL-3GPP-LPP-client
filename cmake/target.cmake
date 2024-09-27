
function(setup_target target)
    target_compile_options(${target} PRIVATE
        "-Wall"
        "-Wextra"
        "-Wpedantic"
    )

    if(${WARNINGS_AS_ERRORS})
        target_compile_options(${target} PRIVATE
            "-Werror"
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(${BUILD_WITH_ALL_WARNINGS})
            target_compile_options(${target} PRIVATE
                "-Weverything"
                "-Wno-c++98-compat"
                "-Wno-c++98-compat-pedantic"
                "-Wno-c++98-c++11-compat-pedantic"
                "-Wno-nested-anon-types"
                "-Wno-gnu-anonymous-struct"
                "-Wno-missing-prototypes"
                "-Wno-documentation"
                "-Wno-documentation-unknown-command"
                "-Wno-weak-vtables"
                "-Wno-unused-const-variable"
                "-Wno-format-nonliteral"
                "-Wno-global-constructors"
                "-Wno-exit-time-destructors"
                "-Wno-padded"
            )
        endif()

        target_compile_options(${target} PRIVATE
            "-Wno-c++17-extensions"
            "-Wno-gnu-zero-variadic-macro-arguments"
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE
            "-Wno-missing-field-initializers"
            "-Wno-error=pragmas"
            "-Wno-pragmas"
        )
    endif()

    if (USE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined,leak)
        target_link_libraries(${target} PRIVATE -fsanitize=address,undefined,leak)
    endif (USE_ASAN)
endfunction()

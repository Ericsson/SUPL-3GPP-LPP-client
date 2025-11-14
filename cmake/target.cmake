
# Function to shuffle a list (Fisher-Yates shuffle)
function(shuffle_list out_var)
    set(list_copy ${ARGN})
    list(LENGTH list_copy len)
    if(len LESS 2)
        set(${out_var} ${list_copy} PARENT_SCOPE)
        return()
    endif()
    math(EXPR last_index "${len} - 1")
    foreach(i RANGE ${last_index} 1)
        string(RANDOM LENGTH 8 ALPHABET "0123456789" rand_str)
        math(EXPR j "${rand_str} % (${i} + 1)")
        list(GET list_copy ${i} item_i)
        list(GET list_copy ${j} item_j)
        list(REMOVE_AT list_copy ${i})
        if(j LESS i)
            list(REMOVE_AT list_copy ${j})
            list(INSERT list_copy ${j} ${item_i})
            list(INSERT list_copy ${i} ${item_j})
        else()
            list(INSERT list_copy ${i} ${item_j})
            list(REMOVE_AT list_copy ${j})
            list(INSERT list_copy ${j} ${item_i})
        endif()
    endforeach()
    set(${out_var} ${list_copy} PARENT_SCOPE)
endfunction()

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

    if(${USE_DWARF4})
        target_compile_options(${target} PRIVATE
            "-gdwarf-4"
        )
    endif()

    if(${DISABLE_LOGGING})
        target_compile_definitions(${target} PRIVATE
            "-DDISABLE_LOGGING"
        )
    endif()

    if(${DATA_TRACING})
        target_compile_definitions(${target} PRIVATE
            "-DDATA_TRACING"
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
                "-Wno-switch-default"
                "-Wno-unsafe-buffer-usage"
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

    if(DEFINED ${target}_DISABLE_ANALYZERS)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
            set_target_properties(${target} PROPERTIES
                CXX_CLANG_TIDY ""
                CXX_CPPCHECK ""
                CXX_INCLUDE_WHAT_YOU_USE ""
            )
        endif()
    endif()

    if(UNITY_BUILD)
        get_target_property(target_type ${target} TYPE)
        if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
            set_target_properties(${target} PROPERTIES UNITY_BUILD ON)
            
            if(SHUFFLE_UNITY_SOURCES)
                get_target_property(sources ${target} SOURCES)
                if(sources)
                    shuffle_list(shuffled_sources ${sources})
                    set_target_properties(${target} PROPERTIES SOURCES "${shuffled_sources}")
                endif()
            endif()
        endif()
    endif()
endfunction()

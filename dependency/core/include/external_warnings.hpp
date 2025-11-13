#pragma once

// clang-format off

// Macros to suppress external library warnings across different compilers
#if defined(__clang__)
#define EXTERNAL_WARNINGS_PUSH \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wreserved-macro-identifier\"") \
    _Pragma("clang diagnostic ignored \"-Wreserved-identifier\"") \
    _Pragma("clang diagnostic ignored \"-Wundef\"") \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-function\"") \
    _Pragma("clang diagnostic ignored \"-Wsuggest-destructor-override\"") \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-copy-with-user-provided-dtor\"") \
    _Pragma("clang diagnostic ignored \"-Wnewline-eof\"") \
    _Pragma("clang diagnostic ignored \"-Wmissing-variable-declarations\"") \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-destructor-override\"") \
    _Pragma("clang diagnostic ignored \"-Wsuggest-override\"") \
    _Pragma("clang diagnostic ignored \"-Wshadow-field\"")

#define EXTERNAL_WARNINGS_POP _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
#define EXTERNAL_WARNINGS_PUSH \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wundef\"") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
    _Pragma("GCC diagnostic ignored \"-Wsuggest-destructor-override\"") \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-copy-with-user-provided-dtor\"") \
    _Pragma("GCC diagnostic ignored \"-Wnewline-eof\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-variable-declarations\"") \
    _Pragma("GCC diagnostic ignored \"-Winconsistent-missing-destructor-override\"") \
    _Pragma("GCC diagnostic ignored \"-Wsuggest-override\"") \
    _Pragma("GCC diagnostic ignored \"-Wshadow-field\"")

#define EXTERNAL_WARNINGS_POP _Pragma("GCC diagnostic pop")

#else
#define EXTERNAL_WARNINGS_PUSH
#define EXTERNAL_WARNINGS_POP
#endif

// clang-format on

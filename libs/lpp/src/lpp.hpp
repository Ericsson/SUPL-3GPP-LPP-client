#pragma once
#include <loglet/loglet.hpp>
#include <lpp/types.hpp>

#define LOGLET_CURRENT_MODULE "lpp"

#define SCOPE_FUNCTION()                                                                           \
    DEBUGF("%s()", LOGLET_CURRENT_FUNCTION);                                                       \
    LOGLET_INDENT_SCOPE()

#define SCOPE_FUNCTIONF(fmt, ...)                                                                  \
    DEBUGF("%s(" fmt ")", LOGLET_CURRENT_FUNCTION, ##__VA_ARGS__);                                 \
    LOGLET_INDENT_SCOPE()

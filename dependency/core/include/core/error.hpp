#pragma once
#include <core/core.hpp>
#include <cstdint>

enum class ErrorDomain : uint8_t {
    Generic   = 0,
    Scheduler = 1,
    IO        = 2,
    LPP       = 3,
    SUPL      = 4,
    Format    = 5,
    Modem     = 6,
};

using ErrorCodeBase = uint16_t;

// Function pointer type for domain-specific error string conversion
using ErrorToStringFunc = char const* (*)(ErrorCodeBase);

struct Error {
    ErrorDomain   domain;
    ErrorCodeBase code;
    int           system_errno;

    NODISCARD bool     ok() const NOEXCEPT { return code == 0; }
    NODISCARD explicit operator bool() const NOEXCEPT { return ok(); }

    NODISCARD Error value() const NOEXCEPT { return Error::none(); }
    NODISCARD Error take_value() NOEXCEPT { return Error::none(); }

    NODISCARD Error const& error() const NOEXCEPT { return *this; }
    NODISCARD Error        take_error() NOEXCEPT { return *this; }

    NODISCARD char const* domain_name() const NOEXCEPT;
    NODISCARD char const* code_name() const NOEXCEPT;

    NODISCARD bool operator==(Error const& other) const NOEXCEPT {
        return domain == other.domain && code == other.code;
    }

    NODISCARD bool operator!=(Error const& other) const NOEXCEPT { return !(*this == other); }

    NODISCARD static Error none() NOEXCEPT { return Error{ErrorDomain::Generic, 0, 0}; }
};

NODISCARD constexpr Error ok() NOEXCEPT {
    return Error{ErrorDomain::Generic, 0, 0};
}

NODISCARD constexpr Error make_error(ErrorDomain domain, ErrorCodeBase code,
                                     int sys_errno = 0) NOEXCEPT {
    return Error{domain, code, sys_errno};
}

// Register domain-specific error string converter
void register_error_to_string(ErrorDomain domain, ErrorToStringFunc func) NOEXCEPT;

// Macro to register error-to-string function (unity build safe via __COUNTER__)
#define REGISTER_ERROR_DOMAIN(domain_value, func)                                                  \
    namespace {                                                                                    \
    struct ErrorRegistration##__COUNTER__ {                                                        \
        ErrorRegistration##__COUNTER__() {                                                         \
            ::register_error_to_string(domain_value, func);                                        \
        }                                                                                          \
    };                                                                                             \
    static ErrorRegistration##__COUNTER__ error_registration##__COUNTER__;                         \
    }

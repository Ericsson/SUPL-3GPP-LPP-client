#pragma once
#include <core/error.hpp>
#include <utility>

template <typename T>
class Result {
public:
    Result(T value, Error error) NOEXCEPT : mValue(std::move(value)), mError(error) {}

    // Allow implicit conversion from Error for TRY macro
    Result(Error error) NOEXCEPT : mValue(), mError(error) {}

    NODISCARD bool     ok() const NOEXCEPT { return mError.ok(); }
    NODISCARD explicit operator bool() const NOEXCEPT { return ok(); }

    NODISCARD T&       value() NOEXCEPT { return mValue; }
    NODISCARD T const& value() const NOEXCEPT { return mValue; }
    NODISCARD T        take_value() NOEXCEPT { return std::move(mValue); }

    NODISCARD Error const& error() const NOEXCEPT { return mError; }
    NODISCARD Error        take_error() NOEXCEPT { return std::move(mError); }

    NODISCARD T&       unwrap() NOEXCEPT { return mValue; }
    NODISCARD T const& unwrap() const NOEXCEPT { return mValue; }

    NODISCARD T&       operator*() NOEXCEPT { return mValue; }
    NODISCARD T const& operator*() const NOEXCEPT { return mValue; }

    NODISCARD T*       operator->() NOEXCEPT { return &mValue; }
    NODISCARD T const* operator->() const NOEXCEPT { return &mValue; }

private:
    T     mValue;
    Error mError;
};

// Helper to create Result with value
template <typename T>
NODISCARD Result<T> ok(T value) NOEXCEPT {
    return Result<T>{std::move(value), ::ok()};
}

// Helper to create Result with error
template <typename T>
NODISCARD Result<T> err(Error error) NOEXCEPT {
    return Result<T>{T{}, error};
}

#define TRY(expr)                                                                                  \
    ({                                                                                             \
        auto _result = (expr);                                                                     \
        if (!_result) {                                                                            \
            return _result.error();                                                                \
        }                                                                                          \
        _result.value();                                                                           \
    })

#define TRY_UNWRAP(expr)                                                                           \
    ({                                                                                             \
        auto _result = (expr);                                                                     \
        if (!_result) {                                                                            \
            return _result.error();                                                                \
        }                                                                                          \
        _result.value();                                                                           \
    })

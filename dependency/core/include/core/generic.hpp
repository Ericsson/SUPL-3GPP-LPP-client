#pragma once
#include <core/error.hpp>

namespace core {

enum class ErrorCode : ErrorCodeBase {
    None             = 0,
    InvalidArgument  = 1,
    OutOfMemory      = 2,
    NotImplemented   = 3,
    InternalError    = 4,
    PermissionDenied = 5,
};

inline Error make_error(ErrorCode code, int system_errno = 0) {
    return ::make_error(ErrorDomain::Generic, static_cast<ErrorCodeBase>(code), system_errno);
}

inline char const* error_to_string(ErrorCode code) {
    switch (code) {
    case ErrorCode::None: return "None";
    case ErrorCode::InvalidArgument: return "InvalidArgument";
    case ErrorCode::OutOfMemory: return "OutOfMemory";
    case ErrorCode::NotImplemented: return "NotImplemented";
    case ErrorCode::InternalError: return "InternalError";
    case ErrorCode::PermissionDenied: return "PermissionDenied";
    }
    return "Unknown";
}

}  // namespace core

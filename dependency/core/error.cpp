#include <array>
#include <core/error.hpp>

static std::array<ErrorToStringFunc, 128> error_to_string_funcs = {};

char const* Error::domain_name() const NOEXCEPT {
    switch (domain) {
    case ErrorDomain::Generic: return "Generic";
    case ErrorDomain::Scheduler: return "Scheduler";
    case ErrorDomain::IO: return "IO";
    case ErrorDomain::LPP: return "LPP";
    case ErrorDomain::SUPL: return "SUPL";
    case ErrorDomain::Format: return "Format";
    case ErrorDomain::Modem: return "Modem";
    }
    return "Unknown";
}

char const* Error::code_name() const NOEXCEPT {
    auto domain_idx = static_cast<size_t>(domain);
    if (domain_idx < error_to_string_funcs.size() && error_to_string_funcs[domain_idx]) {
        return error_to_string_funcs[domain_idx](code);
    }
    return nullptr;
}

void register_error_to_string(ErrorDomain domain, ErrorToStringFunc func) NOEXCEPT {
    auto idx = static_cast<size_t>(domain);
    if (idx < error_to_string_funcs.size()) {
        error_to_string_funcs[idx] = func;
    }
}

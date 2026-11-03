#pragma once
#include <cstdio>
#include <string>

namespace generator {
namespace tokoro {

// Why a lookup of assistance data failed. `reason` is a static string, nullptr means it did not.
struct DataStatus {
    char const* reason{nullptr};
    bool        has_age{false};
    double      age{0.0};
    double      limit{0.0};

    bool ok() const { return reason == nullptr; }

    void fail(char const* r) {
        reason  = r;
        has_age = false;
    }

    void stale(char const* r, double a, double l) {
        reason  = r;
        has_age = true;
        age     = a;
        limit   = l;
    }

    // `reason` or `reason(age=23.1s,max=15.0s)`, without spaces so that it fits a diag column
    std::string text() const {
        if (!reason) return "ok";
        if (!has_age) return reason;
        char buffer[96];
        snprintf(buffer, sizeof(buffer), "%s(age=%.1fs,max=%.1fs)", reason, age, limit);
        return buffer;
    }
};

}  // namespace tokoro
}  // namespace generator

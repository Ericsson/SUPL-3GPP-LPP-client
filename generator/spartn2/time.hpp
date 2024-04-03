#pragma once
#include <generator/spartn2/types.hpp>

#include <cmath>

struct SpartnTime {
    double   seconds;
    uint32_t rounded_seconds;
};

inline bool operator==(SpartnTime const& lhs, SpartnTime const& rhs) {
    constexpr double tolerance = 1e-6;
    return std::abs(lhs.seconds - rhs.seconds) < tolerance &&
           lhs.rounded_seconds == rhs.rounded_seconds;
}

inline bool operator>(SpartnTime const& lhs, SpartnTime const& rhs) {
    return lhs.seconds > rhs.seconds;
}

struct GNSS_SystemTime;
SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time);

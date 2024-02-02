#pragma once
#include <generator/spartn2/types.hpp>

struct SpartnTime {
    double   seconds;
    uint32_t rounded_seconds;
};

inline bool operator==(const SpartnTime& lhs, const SpartnTime& rhs) {
    return lhs.seconds == rhs.seconds && lhs.rounded_seconds == rhs.rounded_seconds;
}

inline bool operator>(const SpartnTime& lhs, const SpartnTime& rhs) {
    return lhs.seconds > rhs.seconds;
}

struct GNSS_SystemTime;
SpartnTime spartn_time_from(const GNSS_SystemTime& epoch_time);

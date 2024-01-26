#pragma once
#include <generator/spartn2/types.hpp>

struct SpartnTime {
    double   seconds;
    uint32_t rounded_seconds;
};

struct GNSS_SystemTime;
SpartnTime spartn_time_from(const GNSS_SystemTime& epoch_time);

#pragma once
#include <core/core.hpp>

CONSTEXPR static uint8_t INVALID_MAPPING = 255;

struct SystemMapping {
    long               gnss_id;
    long               signal_count;
    uint8_t const*     to_spartn;
    uint8_t const*     mapping;
    double const*      freq;
    char const* const* signal_names;

    char const* signal_name(long signal_id) const;
};

extern SystemMapping const GPS_SM;
extern SystemMapping const GLO_SM;
extern SystemMapping const GAL_SM;
extern SystemMapping const BDS_SM;

char const* bias_type_name(long gnss_id, bool is_phase, uint8_t type);

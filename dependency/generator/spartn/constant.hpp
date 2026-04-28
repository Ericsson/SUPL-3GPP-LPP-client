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
    char const* const* rinex_suffixes;
    char const* const* rinex_names;

    NODISCARD char const* signal_name(long signal_id) const;
    NODISCARD char const* signal_rinex_suffix(long signal_id) const;
    NODISCARD char const* signal_rinex_name(long signal_id) const;
    NODISCARD char const* gnss_name() const;

    // Returns the LPP signal index for the given RINEX suffix (e.g. "5X"), or -1 if not found.
    NODISCARD int rinex_suffix_to_index(char const* suffix) const;
};

extern SystemMapping const gGpsSm;
extern SystemMapping const gGloSm;
extern SystemMapping const gGalSm;
extern SystemMapping const gBdsSm;

char const* bias_type_name(long gnss_id, bool is_phase, uint8_t type);
char const* bias_type_rinex_name(long gnss_id, bool is_phase, uint8_t type);

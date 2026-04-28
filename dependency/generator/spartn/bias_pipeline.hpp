#pragma once
#include <generator/spartn2/bias_map.hpp>
#include "constant.hpp"

#include <array>
#include <core/core.hpp>

namespace generator {
namespace spartn {

// Maximum SPARTN bias slots across all constellations (BDS/GAL = 5, GPS = 4, GLO = 2).
static CONSTEXPR uint8_t MAX_BIAS_SLOTS = 8;

// Maximum LPP signal indices per constellation.
static CONSTEXPR uint8_t MAX_RINEX_SIGNALS = 24;

struct BiasEntry {
    double  correction;
    double  continuity_indicator;
    bool    fix_flag;
    uint8_t spartn_slot;
};

// Fixed-size working buffer for one satellite's biases (code or phase).
// No heap allocation.
struct BiasSlots {
    std::array<BiasEntry, MAX_BIAS_SLOTS> slots;
    uint8_t                               mask{0};  // bitmask of occupied slots

    bool has(uint8_t slot) const { return (mask & (1u << slot)) != 0; }
    void set(uint8_t slot, BiasEntry const& e) {
        slots[slot] = e;
        mask |= static_cast<uint8_t>(1u << slot);
    }
};

// Intermediate per-signal entry used between stage 1 and stage 3.
struct RinexBias {
    uint8_t rinex_idx;
    double  correction;
    double  continuity_indicator;
    bool    fix_flag;
    int     mapped_from{-1};  // rinex_idx of source signal if mapped, -1 if original
};

// Working set of RINEX biases for one satellite (code or phase).
// Populated by stage 1, expanded by stage 2, consumed by stage 3.
struct RinexBiasSet {
    std::array<RinexBias, MAX_RINEX_SIGNALS> entries;
    uint32_t                                 present_mask{0};  // bitmask by rinex_idx
    uint8_t                                  count{0};

    bool has(uint8_t rinex_idx) const { return (present_mask & (1u << rinex_idx)) != 0; }

    void add(RinexBias const& b) {
        if (has(b.rinex_idx)) return;
        entries[count++] = b;
        present_mask |= (1u << b.rinex_idx);
    }
};

// Stage 1: populate RinexBiasSet from LPP signal list.
// Calls visitor(long lpp_signal_id, double correction, double continuity_indicator, bool fix_flag)
// for each LPP signal element; caller iterates the ASN.1 list and calls stage1_add.
void stage1_add(SystemMapping const& sm, RinexBiasSet& set, long lpp_signal_id, double correction,
                double continuity_indicator, bool fix_flag);

// Stage 2: expand RinexBiasSet using BiasMap.
// For each entry in the map (in order), if from_rinex_idx is present and to_rinex_idx is not,
// add a scaled copy. Frequency scaling: correction *= freq[from] / freq[to].
void stage2_expand(SystemMapping const& sm, BiasMap const& map, RinexBiasSet& set, bool is_code);

// Stage 3: assign SPARTN slots from RinexBiasSet.
// Signals with no SPARTN slot are discarded.
BiasSlots stage3_assign(SystemMapping const& sm, RinexBiasSet const& set);

}  // namespace spartn
}  // namespace generator

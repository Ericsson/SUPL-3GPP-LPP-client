#include "bias_pipeline.hpp"

namespace generator {
namespace spartn {

void stage1_add(SystemMapping const& sm, RinexBiasSet& set, long lpp_signal_id, double correction,
                double continuity_indicator, bool fix_flag) {
    if (lpp_signal_id < 0 || lpp_signal_id >= sm.signal_count) return;
    if (!sm.rinex_suffixes[lpp_signal_id]) return;
    RinexBias b{static_cast<uint8_t>(lpp_signal_id), correction, continuity_indicator, fix_flag};
    set.add(b);
}

void stage2_expand(SystemMapping const& sm, BiasMap const& map, RinexBiasSet& set, bool is_code) {
    for (uint8_t i = 0; i < map.count; ++i) {
        auto const& e = map.entries[i];
        if (is_code && !e.apply_code) continue;
        if (!is_code && !e.apply_phase) continue;
        if (!set.has(e.from_rinex_idx)) continue;
        if (set.has(e.to_rinex_idx)) continue;

        // Find the source entry.
        RinexBias const* src = nullptr;
        for (uint8_t j = 0; j < set.count; ++j) {
            if (set.entries[j].rinex_idx == e.from_rinex_idx) {
                src = &set.entries[j];
                break;
            }
        }
        if (!src) continue;

        double scale  = 1.0;
        double f_from = sm.freq[e.from_rinex_idx];
        double f_to   = sm.freq[e.to_rinex_idx];
        if (!is_code && f_from > 0.0 && f_to > 0.0 && f_from != f_to) {
            scale = f_from / f_to;
        }

        RinexBias mapped{e.to_rinex_idx, src->correction * scale, src->continuity_indicator,
                         src->fix_flag, static_cast<int>(e.from_rinex_idx)};
        set.add(mapped);
    }
}

BiasSlots stage3_assign(SystemMapping const& sm, RinexBiasSet const& set) {
    BiasSlots result{};
    for (uint8_t i = 0; i < set.count; ++i) {
        auto const& rb   = set.entries[i];
        uint8_t     slot = sm.to_spartn[rb.rinex_idx];
        if (slot == INVALID_MAPPING) continue;
        if (result.has(slot)) continue;  // first-come-first-served
        result.set(slot, {rb.correction, rb.continuity_indicator, rb.fix_flag, slot});
    }
    return result;
}

}  // namespace spartn
}  // namespace generator

#pragma once
#include <array>
#include <cassert>
#include <core/core.hpp>

namespace generator {
namespace spartn {

// Maximum number of user-configurable bias map entries per GNSS.
static CONSTEXPR uint8_t BIAS_MAP_MAX_ENTRIES = 32;

struct BiasMapEntry {
    uint8_t from_rinex_idx;
    uint8_t to_rinex_idx;
    bool    apply_code;
    bool    apply_phase;
};

struct BiasMap {
    std::array<BiasMapEntry, BIAS_MAP_MAX_ENTRIES> entries;
    uint8_t                                        count{0};

    void add(BiasMapEntry e) {
        assert(count < BIAS_MAP_MAX_ENTRIES);
        entries[count++] = e;
    }

    void clear() { count = 0; }
};

}  // namespace spartn
}  // namespace generator

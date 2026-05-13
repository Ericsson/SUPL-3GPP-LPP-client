#pragma once
#include "bias_map.hpp"

namespace generator {
namespace spartn {

// Apply default bias mappings for GPS, GAL, and BDS.
// maps[0]=GPS, maps[1]=GLO, maps[2]=GAL, maps[3]=BDS
inline void apply_default_bias_maps(BiasMap maps[4]) {
    // GPS: 2X→2L, 5X→5Q
    maps[0].add({2, 10, true, true});   // GPS 2X (L2C M+L) → 2L (L2C(L))
    maps[0].add({14, 13, true, true});  // GPS 5X (L5 I+Q)  → 5Q (L5 Q)
    // GAL: 8X→5Q, 8X→7Q, 1X→1C, 6X→6C
    maps[2].add({20, 22, true, true});  // GAL 8X (E5(A+B) I+Q) → 5Q (E5A Q)
    maps[2].add({20, 16, true, true});  // GAL 8X (E5(A+B) I+Q) → 7Q (E5B Q)
    maps[2].add({8, 5, true, true});    // GAL 1X (E1 B+C)      → 1C (E1 C)
    maps[2].add({13, 10, true, true});  // GAL 6X (E6 B+C)      → 6C (E6 C)
    // BDS: 5X→5P, 1X→1P
    maps[3].add({14, 13, true, true});  // BDS 5X (B2a D+P) → 5P (B2a P)
    maps[3].add({11, 10, true, true});  // BDS 1X (B1C D+P) → 1P (B1C P)
}

}  // namespace spartn
}  // namespace generator

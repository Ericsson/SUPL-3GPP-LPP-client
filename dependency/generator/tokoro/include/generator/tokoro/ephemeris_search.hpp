#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace generator {
namespace tokoro {

// An ephemeris that was considered for a satellite
struct EphemerisCandidate {
    uint16_t iod;      // IOD that is compared to the IOD of the SSR orbit correction
    uint16_t iode;     // IODE (GPS, QZSS, BeiDou)
    uint16_t iodc;     // IODC (GPS, QZSS, BeiDou)
    uint16_t iod_nav;  // IODnav (Galileo)
    double   toe;
    uint16_t week;
};

// Result of the search for the ephemeris that belongs to an SSR orbit correction
struct EphemerisSearch {
    bool   unsupported{false};  // the GNSS has no ephemeris support
    size_t total{0};            // ephemeris stored for the satellite
    // ephemeris that are valid at the time of the search, with or without matching IOD
    std::vector<EphemerisCandidate> valid;
    bool                            found{false};
    EphemerisCandidate              selected{};
};

}  // namespace tokoro
}  // namespace generator

#pragma once
#include <core/core.hpp>

#include <cstdint>
#include <unordered_map>

#include <time/tai.hpp>

namespace ephemeris {
struct GloEphemeris;
}

namespace format {
namespace nav {

// One GLONASS L1OF/L2OF navigation string (ICD 5.0, 4.4). `bits` holds ICD bits 85..1,
// MSB first (bit 85, the idle chip, is the MSB of bits[0]).
struct GloString {
    uint32_t bits[3];
    uint32_t frame;  // receiver frame counter, identifies strings of the same frame

    // From UBX-RXM-SFRBX (4 words: 3 string words and the frame counter)
    NODISCARD static GloString from_sfrbx(uint32_t const words[4]) NOEXCEPT;

    NODISCARD uint32_t bit_range(int high, int low) const NOEXCEPT;
    NODISCARD uint8_t  number() const NOEXCEPT { return static_cast<uint8_t>(bit_range(84, 81)); }
    // Hamming code check (ICD 5.0, 4.7)
    NODISCARD bool check() const NOEXCEPT;
};

// Collects the immediate strings (1-4) of a frame into an ephemeris
class GloEphemerisCollector {
public:
    GloEphemerisCollector() = default;

    // `now` resolves the four-year interval of the day number when string 5 (N4) is missing
    bool process(uint8_t slot, int8_t frequency_channel, GloString const& string,
                 ts::Tai const& now, ephemeris::GloEphemeris& ephemeris) NOEXCEPT;

private:
    struct Frame {
        GloString strings[4];
        bool      received[4];
    };

    std::unordered_map<uint8_t, Frame> mFrames;
    uint8_t                            mN4{0};
};

}  // namespace nav
}  // namespace format

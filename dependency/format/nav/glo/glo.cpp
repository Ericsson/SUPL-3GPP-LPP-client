#include "glo.hpp"

#include <cmath>

#include <ephemeris/glo.hpp>
#include <loglet/loglet.hpp>
#include <time/glo.hpp>

LOGLET_MODULE3(format, nav, glo);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nav, glo)

namespace format {
namespace nav {

GloString GloString::from_sfrbx(uint32_t const words[4]) NOEXCEPT {
    GloString string{};
    string.bits[0] = words[0];
    string.bits[1] = words[1];
    string.bits[2] = words[2];
    string.frame   = words[3];
    return string;
}

// ICD bit `b` (85..1) is at offset 85 - b from the MSB of bits[0]
static uint32_t icd_bit(uint32_t const bits[3], int b) {
    auto offset = 85 - b;
    return (bits[offset / 32] >> (31 - (offset % 32))) & 1u;
}

uint32_t GloString::bit_range(int high, int low) const NOEXCEPT {
    uint32_t value = 0;
    for (auto b = high; b >= low; b--)
        value = (value << 1) | icd_bit(bits, b);
    return value;
}

bool GloString::check() const NOEXCEPT {
    // Data bits 9..85 take the Hamming positions that are not powers of two; check bit k
    // (1..7) covers the positions with bit k - 1 set and check bit 8 covers the whole string.
    uint32_t syndrome = 0;
    auto     position = 3;
    for (auto b = 9; b <= 85; b++, position++) {
        while ((position & (position - 1)) == 0)
            position++;
        if (!icd_bit(bits, b)) continue;
        for (auto k = 0; k < 7; k++)
            if (position & (1 << k)) syndrome ^= 1u << k;
    }
    for (auto k = 0; k < 7; k++)
        syndrome ^= icd_bit(bits, k + 1) << k;

    uint32_t parity = 0;
    for (auto b = 1; b <= 85; b++)
        parity ^= icd_bit(bits, b);
    return syndrome == 0 && parity == 0;
}

static double sign_magnitude(uint32_t value, int length, int exponent) {
    auto magnitude = static_cast<double>(value & ((1u << (length - 1)) - 1u));
    auto result    = std::ldexp(magnitude, exponent);
    return (value & (1u << (length - 1))) ? -result : result;
}

bool GloEphemerisCollector::process(uint8_t slot, int8_t frequency_channel, GloString const& string,
                                    ts::Tai const&           now,
                                    ephemeris::GloEphemeris& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTIONF("R%02u", slot);
    if (!string.check()) {
        VERBOSEF("hamming code check failed");
        return false;
    }

    auto number = string.number();
    if (number == 5) {
        auto n4 = static_cast<uint8_t>(string.bit_range(36, 32));
        if (n4 > 0) mN4 = n4;
        return false;
    }
    if (number < 1 || number > 4) return false;

    auto& frame = mFrames[slot];
    if (frame.received[0] && frame.strings[0].frame != string.frame) frame = Frame{};
    frame.strings[number - 1]  = string;
    frame.received[number - 1] = true;
    if (number != 4) return false;
    for (auto i = 0; i < 4; i++) {
        if (!frame.received[i] || frame.strings[i].frame != string.frame) return false;
    }

    auto const& s1 = frame.strings[0];
    auto const& s2 = frame.strings[1];
    auto const& s3 = frame.strings[2];
    auto const& s4 = frame.strings[3];

    auto bn  = s2.bit_range(80, 78);
    auto tb  = s2.bit_range(76, 70);
    auto ln  = s3.bit_range(65, 65);
    auto nt  = s4.bit_range(26, 16);
    auto tod = static_cast<double>(tb) * 900.0;

    ts::Glo reference_time{};
    if (nt > 0 && mN4 > 0) {
        auto day       = static_cast<int64_t>(mN4 - 1) * 1461 + (static_cast<int64_t>(nt) - 1);
        reference_time = ts::Glo::from_absolute_day_tod(day, tod);
    } else if (nt > 0) {
        reference_time =
            ts::Glo::from_period_day_tod(static_cast<int64_t>(nt) - 1, tod, ts::Glo(now));
    } else {
        VERBOSEF("missing day number");
        return false;
    }

    ephemeris                  = {};
    ephemeris.slot_number      = slot;
    ephemeris.frequency_number = frequency_channel;
    ephemeris.reference_time   = reference_time;
    ephemeris.velocity.x       = sign_magnitude(s1.bit_range(64, 41), 24, -20);
    ephemeris.acceleration.x   = sign_magnitude(s1.bit_range(40, 36), 5, -30);
    ephemeris.position.x       = sign_magnitude(s1.bit_range(35, 9), 27, -11);
    ephemeris.velocity.y       = sign_magnitude(s2.bit_range(64, 41), 24, -20);
    ephemeris.acceleration.y   = sign_magnitude(s2.bit_range(40, 36), 5, -30);
    ephemeris.position.y       = sign_magnitude(s2.bit_range(35, 9), 27, -11);
    ephemeris.gamma_n          = sign_magnitude(s3.bit_range(79, 69), 11, -40);
    ephemeris.velocity.z       = sign_magnitude(s3.bit_range(64, 41), 24, -20);
    ephemeris.acceleration.z   = sign_magnitude(s3.bit_range(40, 36), 5, -30);
    ephemeris.position.z       = sign_magnitude(s3.bit_range(35, 9), 27, -11);
    ephemeris.tau_n            = sign_magnitude(s4.bit_range(80, 59), 22, -30);
    ephemeris.age              = static_cast<uint8_t>(s4.bit_range(53, 49));
    ephemeris.m                = static_cast<uint8_t>(s4.bit_range(10, 9));
    ephemeris.health           = static_cast<uint8_t>(((bn >> 2) & 1u) | ln);
    ephemeris.lpp_iod          = static_cast<uint16_t>(tb);

    frame = Frame{};
    DEBUGF("ephemeris R%02u tb=%u", slot, tb);
    return true;
}

}  // namespace nav
}  // namespace format

#include "gal/inav.hpp"

#include <cmath>

#include <ephemeris/gal.hpp>
#include <loglet/loglet.hpp>

#define SEMI_CIRCLE_TO_RAD 3.1415926535898

LOGLET_MODULE3(format, nav, gal);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nav, gal)

namespace format {
namespace nav {
namespace gal {

static int64_t signed_transform(uint32_t bit_length, uint64_t value) {
    auto sign_mask = static_cast<uint64_t>(1) << (bit_length - 1);
    auto sign      = value & sign_mask;
    if (!sign) {
        return static_cast<int64_t>(value);
    } else {
        auto signed_extended = (~static_cast<uint64_t>(0)) << bit_length;
        return static_cast<int64_t>(value | signed_extended);
    }
}

static double signed_scale(uint32_t bit_length, uint32_t value, double power) {
    return static_cast<double>(signed_transform(bit_length, value)) * std::pow(2.0, power);
}

static double unsigned_scale(uint32_t value, double power) {
    return static_cast<double>(value) * std::pow(2.0, power);
}

static bool decode_word_1(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    // clang-format off
    uint32_t i = 6;
    auto iod_nav = words.u16(i, 10); i += 10;
    auto toe     = words.u32(i, 14); i += 14;
    auto m0      = words.u32(i, 32); i += 32;
    auto e       = words.u32(i, 32); i += 32;
    auto sqrt_a  = words.u32(i, 32); i += 32;
    // clang-format on

    auto& data   = word.type1;
    data.iod_nav = iod_nav;
    data.toe     = static_cast<double>(toe) * 60.0;
    data.m0      = signed_scale(32, m0, -31.0) * SEMI_CIRCLE_TO_RAD;
    data.e       = unsigned_scale(e, -33.0);
    data.sqrt_a  = unsigned_scale(sqrt_a, -19.0);

    VERBOSEF("word1: iod_nav=%u, toe=%g, m0=%g, e=%g, sqrt_a=%g", data.iod_nav, data.toe, data.m0,
             data.e, data.sqrt_a);
    return true;
}

static bool decode_word_2(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    // clang-format off
    uint32_t i = 6;
    auto iod_nav = words.u16(i, 10); i += 10;
    auto omega0  = words.u32(i, 32); i += 32;
    auto i0      = words.u32(i, 32); i += 32;
    auto omega   = words.u32(i, 32); i += 32;
    auto idot    = words.u32(i, 14); i += 14;
    // clang-format on

    auto& data   = word.type2;
    data.iod_nav = iod_nav;
    data.omega0  = signed_scale(32, omega0, -31.0) * SEMI_CIRCLE_TO_RAD;
    data.i0      = signed_scale(32, i0, -31.0) * SEMI_CIRCLE_TO_RAD;
    data.omega   = signed_scale(32, omega, -31.0) * SEMI_CIRCLE_TO_RAD;
    data.idot    = signed_scale(14, idot, -43.0) * SEMI_CIRCLE_TO_RAD;

    VERBOSEF("word2: iod_nav=%u, omega0=%g, i0=%g, omega=%g, idot=%g", data.iod_nav, data.omega0,
             data.i0, data.omega, data.idot);
    return true;
}

static bool decode_word_3(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    // clang-format off
    uint32_t i = 6;
    auto iod_nav   = words.u16(i, 10); i += 10;
    auto omega_dot = words.u32(i, 24); i += 24;
    auto delta_n   = words.u32(i, 16); i += 16;
    auto cuc       = words.u32(i, 16); i += 16;
    auto cus       = words.u32(i, 16); i += 16;
    auto crc       = words.u32(i, 16); i += 16;
    auto crs       = words.u32(i, 16); i += 16;
    // clang-format on

    auto& data     = word.type3;
    data.iod_nav   = iod_nav;
    data.omega_dot = signed_scale(24, omega_dot, -43.0) * SEMI_CIRCLE_TO_RAD;
    data.delta_n   = signed_scale(16, delta_n, -43.0) * SEMI_CIRCLE_TO_RAD;
    data.cuc       = signed_scale(16, cuc, -29.0);
    data.cus       = signed_scale(16, cus, -29.0);
    data.crc       = signed_scale(16, crc, -5.0);
    data.crs       = signed_scale(16, crs, -5.0);

    VERBOSEF("word3: iod_nav=%u, omega_dot=%g, delta_n=%g, cuc=%g, cus=%g, crc=%g, crs=%g",
             data.iod_nav, data.omega_dot, data.delta_n, data.cuc, data.cus, data.crc, data.crs);
    return true;
}

static bool decode_word_4(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    // clang-format off
    uint32_t i = 6;
    auto iod_nav = words.u16(i, 10); i += 10;
    auto sv_id   = words.u8(i, 6); i += 6;
    auto cic     = words.u32(i, 16); i += 16;
    auto cis     = words.u32(i, 16); i += 16;
    auto toc     = words.u32(i, 14); i += 14;
    auto af0     = words.u32(i, 31); i += 31;
    auto af1     = words.u32(i, 21); i += 21;
    auto af2     = words.u32(i, 6); i += 6;
    // clang-format on

    auto& data   = word.type4;
    data.iod_nav = iod_nav;
    data.sv_id   = sv_id;
    data.cic     = signed_scale(16, cic, -29.0);
    data.cis     = signed_scale(16, cis, -29.0);
    data.toc     = static_cast<double>(toc) * 60.0;
    data.af0     = signed_scale(31, af0, -34.0);
    data.af1     = signed_scale(21, af1, -46.0);
    data.af2     = signed_scale(6, af2, -59.0);

    VERBOSEF("word4: iod_nav=%u, sv_id=%u, cic=%g, cis=%g, toc=%g, af0=%g, af1=%g, af2=%g",
             data.iod_nav, data.sv_id, data.cic, data.cis, data.toc, data.af0, data.af1, data.af2);
    return true;
}

static bool decode_word_5(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    // clang-format off
    uint32_t i = 6;
    auto a_i0       = words.u32(i, 11); i += 11;
    auto a_i1       = words.u32(i, 11); i += 11;
    auto a_i2       = words.u32(i, 14); i += 14;
    auto region1    = words.b1(i); i += 1;
    auto region2    = words.b1(i); i += 1;
    auto region3    = words.b1(i); i += 1;
    auto region4    = words.b1(i); i += 1;
    auto region5    = words.b1(i); i += 1;
    auto bgd_e1_e5a = words.u16(i, 10); i += 10;
    auto bgd_e1_e5b = words.u16(i, 10); i += 10;
    auto e5a_hs     = words.u8(i, 2); i += 2;
    auto e1b_hs     = words.u8(i, 2); i += 2;
    auto e5b_dvs    = words.u8(i, 1); i += 1;
    auto e1b_dvs    = words.u8(i, 1); i += 1;
    auto wn         = words.u16(i, 12); i += 12;
    auto tow        = words.u32(i, 20); i += 20;
    // clang-format on

    auto& data      = word.type5;
    data.a_i0       = signed_scale(11, a_i0, -2.0);
    data.a_i1       = signed_scale(11, a_i1, -8.0);
    data.a_i2       = signed_scale(14, a_i2, -15.0);
    data.region1    = region1;
    data.region2    = region2;
    data.region3    = region3;
    data.region4    = region4;
    data.region5    = region5;
    data.bgd_e1_e5a = signed_scale(10, bgd_e1_e5a, -32.0);
    data.bgd_e1_e5b = signed_scale(10, bgd_e1_e5b, -32.0);
    data.e5a_hs     = e5a_hs;
    data.e1b_hs     = e1b_hs;
    data.e5b_dvs    = e5b_dvs;
    data.e1b_dvs    = e1b_dvs;
    data.wn         = wn;
    data.tow        = tow;

    VERBOSEF("word5: a_i0=%g, a_i1=%g, a_i2=%g, region1=%u, region2=%u, region3=%u, region4=%u, "
             "region5=%u, bgd_e1_e5a=%g, bgd_e1_e5b=%g, e5a_hs=%u, e1b_hs=%u, e5b_dvs=%u, "
             "e1b_dvs=%u, wn=%u, tow=%u",
             data.a_i0, data.a_i1, data.a_i2, data.region1, data.region2, data.region3,
             data.region4, data.region5, data.bgd_e1_e5a, data.bgd_e1_e5b, data.e5a_hs, data.e1b_hs,
             data.e5b_dvs, data.e1b_dvs, data.wn, data.tow);
    return true;
}

bool InavWord::decode(Words const& words, InavWord& word) NOEXCEPT {
    FUNCTION_SCOPE();

    if (words.size() != 256) {
        VERBOSEF("invalid number of words: %zu, expected: 256 bits", words.size());
        return false;
    }

    auto page1_odd   = words.b1(0);
    auto page1_alert = words.b1(1);
    auto page2_odd   = words.b1(128);
    auto page2_alert = words.b1(129);

    if (page1_odd || !page2_odd) {
        VERBOSEF("invalid page parity");
        return false;
    } else if (page1_alert || page2_alert) {
        VERBOSEF("invalid page alert");
        return false;
    }

    Words data{128};
    data.set(0, 32, words.u32(2, 32));
    data.set(32, 32, words.u32(34, 32));
    data.set(64, 32, words.u32(66, 32));
    data.set(96, 16, words.u32(98, 16));
    data.set(112, 16, words.u32(130, 16));

    auto word_type = words.u8(2, 6);
    word.word_type = word_type;
    switch (word_type) {
    case 1: return decode_word_1(data, word);
    case 2: return decode_word_2(data, word);
    case 3: return decode_word_3(data, word);
    case 4: return decode_word_4(data, word);
    case 5: return decode_word_5(data, word);
    default: VERBOSEF("unsupported word: %u", word_type); return true;
    }
}

bool InavEphemerisCollector::process(uint8_t prn, InavWord const& word,
                                     ephemeris::GalEphemeris& ephemeris) NOEXCEPT {
    auto& internal_ephemeris = mBuffer[prn];

    // Process subframe based on its type
    switch (word.word_type) {
    case 1:
        internal_ephemeris.word1      = true;
        internal_ephemeris.word1_data = word.type1;
        break;
    case 2:
        internal_ephemeris.word2      = true;
        internal_ephemeris.word2_data = word.type2;
        break;
    case 3:
        internal_ephemeris.word3      = true;
        internal_ephemeris.word3_data = word.type3;
        break;
    case 4:
        internal_ephemeris.word4      = true;
        internal_ephemeris.word4_data = word.type4;
        break;
    case 5:
        internal_ephemeris.word5      = true;
        internal_ephemeris.word5_data = word.type5;
        break;
    default:
        // Unknown subframe type
        VERBOSEF("unsupported word type: %u", word.word_type);
        return false;
    }

    if (internal_ephemeris.word1 && internal_ephemeris.word2 && internal_ephemeris.word3 &&
        internal_ephemeris.word4 && internal_ephemeris.word5) {
        // Check that the IODEs match
        if (internal_ephemeris.word1_data.iod_nav != internal_ephemeris.word2_data.iod_nav ||
            internal_ephemeris.word1_data.iod_nav != internal_ephemeris.word3_data.iod_nav ||
            internal_ephemeris.word1_data.iod_nav != internal_ephemeris.word4_data.iod_nav) {
            VERBOSEF("IODE mismatch for PRN %u", prn);
            return false;
        }

        VERBOSEF("processing ephemeris for PRN %u (week: %u, IOD: %u)", prn,
                 internal_ephemeris.word1_data.iod_nav, internal_ephemeris.word1_data.iod_nav);

        auto& w1 = internal_ephemeris.word1_data;
        auto& w2 = internal_ephemeris.word2_data;
        auto& w3 = internal_ephemeris.word3_data;
        auto& w4 = internal_ephemeris.word4_data;
        auto& w5 = internal_ephemeris.word5_data;

        ephemeris.prn         = prn;
        ephemeris.week_number = w5.wn;
        ephemeris.iod_nav     = w1.iod_nav;

        ephemeris.toc = w4.toc;
        ephemeris.af0 = w4.af0;
        ephemeris.af1 = w4.af1;
        ephemeris.af2 = w4.af2;

        ephemeris.toe       = w1.toe;
        ephemeris.m0        = w1.m0;
        ephemeris.e         = w1.e;
        ephemeris.a         = w1.sqrt_a * w1.sqrt_a;
        ephemeris.delta_n   = w3.delta_n;
        ephemeris.i0        = w2.i0;
        ephemeris.idot      = w2.idot;
        ephemeris.omega0    = w2.omega0;
        ephemeris.omega     = w2.omega;
        ephemeris.omega_dot = w3.omega_dot;
        ephemeris.cuc       = w3.cuc;
        ephemeris.cus       = w3.cus;
        ephemeris.crc       = w3.crc;
        ephemeris.crs       = w3.crs;
        ephemeris.cic       = w4.cic;
        ephemeris.cis       = w4.cis;

        // [3GPP TS 37.355]: In the case of broadcasted Galileo ephemeris, the iod contains the IOD
        // index as described in [8].
        ephemeris.lpp_iod = ephemeris.iod_nav;

        internal_ephemeris.word1 = false;
        internal_ephemeris.word2 = false;
        internal_ephemeris.word3 = false;
        internal_ephemeris.word4 = false;
        internal_ephemeris.word5 = false;
        return true;
    } else {
        return false;
    }
}

}  // namespace gal
}  // namespace nav
}  // namespace format

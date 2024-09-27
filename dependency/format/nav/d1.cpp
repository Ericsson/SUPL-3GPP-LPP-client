#include "d1.hpp"

#include <cmath>

#include <ephemeris/bds.hpp>
#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "nav/d1"
#define SEMI_CIRCLE_TO_RAD 3.1415926535898

namespace format {
namespace nav {

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

static bool decode_subframe1(Words const& words, D1Subframe1& subframe) NOEXCEPT {
    VERBOSEF("decoding subframe1");
    LOGLET_VINDENT_SCOPE();

    auto sow_msb        = words.u32(18, 8);
    auto sow_lsb        = words.u32(30, 12);
    auto sow            = (sow_msb << 12) | sow_lsb;
    auto sat_h1         = words.u8(42, 1);
    auto aodc           = words.u8(43, 5);
    auto urai           = words.u8(48, 4);
    auto wn             = words.u16(60, 13);
    auto toc_msb        = words.u32(73, 9);
    auto toc_lsb        = words.u32(90, 8);
    auto toc            = (toc_msb << 8) | toc_lsb;
    auto tgd1           = words.u32(98, 10);
    auto tgd2_msb       = words.u32(108, 4);
    auto tgd2_lsb       = words.u32(120, 6);
    auto tgd2           = (tgd2_msb << 6) | tgd2_lsb;
    auto iono_alpha0    = words.u32(126, 8);
    auto iono_alpha1    = words.u32(134, 8);
    auto iono_alpha2    = words.u32(150, 8);
    auto iono_alpha3    = words.u32(158, 8);
    auto iono_beta0_msb = words.u32(166, 6);
    auto iono_beta0_lsb = words.u32(180, 2);
    auto iono_beta0     = (iono_beta0_msb << 2) | iono_beta0_lsb;
    auto iono_beta1     = words.u32(182, 8);
    auto iono_beta2     = words.u32(190, 8);
    auto iono_beta3_msb = words.u32(198, 4);
    auto iono_beta3_lsb = words.u32(210, 4);
    auto iono_beta3     = (iono_beta3_msb << 4) | iono_beta3_lsb;
    auto a2             = words.u32(214, 11);
    auto a0_msb         = words.u32(225, 7);
    auto a0_lsb         = words.u32(240, 17);
    auto a0             = (a0_msb << 17) | a0_lsb;
    auto a1_msb         = words.u32(257, 5);
    auto a1_lsb         = words.u32(270, 17);
    auto a1             = (a1_msb << 17) | a1_lsb;
    auto aode           = words.u8(287, 5);

    subframe.sow           = sow;
    subframe.sat_h1        = sat_h1;
    subframe.aodc          = aodc;
    subframe.aode          = aode;
    subframe.urai          = urai;
    subframe.wn            = wn;
    subframe.iono_alpha[0] = signed_scale(8, iono_alpha0, -30.0);
    subframe.iono_alpha[1] = signed_scale(8, iono_alpha1, -27.0);
    subframe.iono_alpha[2] = signed_scale(8, iono_alpha2, -24.0);
    subframe.iono_alpha[3] = signed_scale(8, iono_alpha3, -24.0);
    subframe.iono_beta[0]  = signed_scale(8, iono_beta0, 11.0);
    subframe.iono_beta[1]  = signed_scale(8, iono_beta1, 14.0);
    subframe.iono_beta[2]  = signed_scale(8, iono_beta2, 16.0);
    subframe.iono_beta[3]  = signed_scale(8, iono_beta3, 16.0);
    subframe.toc           = unsigned_scale(toc, 3.0);
    subframe.a0            = signed_scale(24, a0, -33.0);
    subframe.a1            = signed_scale(22, a1, -50.0);
    subframe.a2            = signed_scale(11, a2, -66.0);
    subframe.tgd1          = signed_scale(10, tgd1, 1.0) * 1e-10;
    subframe.tgd2          = signed_scale(10, tgd2, 1.0) * 1e-10;

    VERBOSEF(
        "subframe1: sow=%g, sat_h1=%u, aodc=%u, urai=%u, wn=%u, toc=%g, tgd1=%g, tgd2=%g, "
        "iono_alpha=[%g, %g, %g, %g], iono_beta=[%g, %g, %g, %g], a0=%g, a1=%g, a2=%g, aode=%u",
        subframe.sow, subframe.sat_h1, subframe.aodc, subframe.urai, subframe.wn, subframe.toc,
        subframe.tgd1, subframe.tgd2, subframe.iono_alpha[0], subframe.iono_alpha[1],
        subframe.iono_alpha[2], subframe.iono_alpha[3], subframe.iono_beta[0],
        subframe.iono_beta[1], subframe.iono_beta[2], subframe.iono_beta[3], subframe.a0,
        subframe.a1, subframe.a2, subframe.aode);

    return true;
}

static bool decode_subframe2(Words const& words, D1Subframe2& subframe) NOEXCEPT {
    VERBOSEF("decoding subframe2");
    LOGLET_VINDENT_SCOPE();

    auto sow_msb     = words.u32(18, 8);
    auto sow_lsb     = words.u32(30, 12);
    auto sow         = (sow_msb << 12) | sow_lsb;
    auto delta_n_msb = words.u32(42, 10);
    auto delta_n_lsb = words.u32(60, 6);
    auto delta_n     = (delta_n_msb << 6) | delta_n_lsb;
    auto cuc_msb     = words.u32(66, 16);
    auto cuc_lsb     = words.u32(90, 2);
    auto cuc         = (cuc_msb << 2) | cuc_lsb;
    auto m0_msb      = words.u32(92, 20);
    auto m0_lsb      = words.u32(120, 12);
    auto m0          = (m0_msb << 12) | m0_lsb;
    auto e_msb       = words.u32(132, 10);
    auto e_lsb       = words.u32(150, 22);
    auto e           = (e_msb << 22) | e_lsb;
    auto cus         = words.u32(180, 18);
    auto crc_msb     = words.u32(198, 4);
    auto crc_lsb     = words.u32(210, 14);
    auto crc         = (crc_msb << 14) | crc_lsb;
    auto crs_msb     = words.u32(224, 8);
    auto crs_lsb     = words.u32(240, 10);
    auto crs         = (crs_msb << 10) | crs_lsb;
    auto sqrt_a_msb  = words.u32(250, 12);
    auto sqrt_a_lsb  = words.u32(270, 20);
    auto sqrt_a      = (sqrt_a_msb << 20) | sqrt_a_lsb;
    auto toe_msb     = words.u32(290, 2);

    subframe.sow     = sow;
    subframe.sqrt_a  = unsigned_scale(sqrt_a, -19.0);
    subframe.e       = unsigned_scale(e, -33.0);
    subframe.delta_n = signed_scale(16, delta_n, -43.0) * SEMI_CIRCLE_TO_RAD;
    subframe.m0      = signed_scale(32, m0, -31.0) * SEMI_CIRCLE_TO_RAD;
    subframe.cuc     = signed_scale(18, cuc, -31.0);
    subframe.cus     = signed_scale(18, cus, -31.0);
    subframe.crc     = signed_scale(18, crc, -6.0);
    subframe.crs     = signed_scale(18, crs, -6.0);
    subframe.toe_msb = toe_msb;

    VERBOSEF("subframe2: sow=%g, delta_n=%g, cuc=%g, m0=%g, e=%g, cus=%g, crc=%g, crs=%g, "
             "sqrt_a=%g, toe_msb=%u",
             subframe.sow, subframe.delta_n, subframe.cuc, subframe.m0, subframe.e, subframe.cus,
             subframe.crc, subframe.crs, subframe.sqrt_a, subframe.toe_msb);

    return true;
}

static bool decode_subframe3(Words const& words, D1Subframe3& subframe) NOEXCEPT {
    VERBOSEF("decoding subframe3");
    LOGLET_VINDENT_SCOPE();

    auto sow_msb       = words.u32(18, 8);
    auto sow_lsb       = words.u32(30, 12);
    auto sow           = (sow_msb << 12) | sow_lsb;
    auto toe_xsb       = words.u32(42, 10);
    auto toe_lsb       = words.u32(60, 5);
    auto toe           = (toe_xsb << 5) | toe_lsb;
    auto i0_msb        = words.u32(65, 17);
    auto i0_lsb        = words.u32(90, 15);
    auto i0            = (i0_msb << 15) | i0_lsb;
    auto cic_msb       = words.u32(105, 7);
    auto cic_lsb       = words.u32(120, 11);
    auto cic           = (cic_msb << 11) | cic_lsb;
    auto omega_dot_msb = words.u32(131, 11);
    auto omega_dot_lsb = words.u32(150, 13);
    auto omega_dot     = (omega_dot_msb << 13) | omega_dot_lsb;
    auto cis_msb       = words.u32(163, 9);
    auto cis_lsb       = words.u32(180, 9);
    auto cis           = (cis_msb << 9) | cis_lsb;
    auto idot_msb      = words.u32(189, 13);
    auto idot_lsb      = words.u32(210, 1);
    auto idot          = (idot_msb << 1) | idot_lsb;
    auto omega0_msb    = words.u32(211, 21);
    auto omega0_lsb    = words.u32(240, 11);
    auto omega0        = (omega0_msb << 11) | omega0_lsb;
    auto omega_msb     = words.u32(251, 13);
    auto omega_lsb     = words.u32(270, 11);
    auto omega         = (omega_msb << 11) | omega_lsb;

    subframe.sow       = sow;
    subframe.toe_lsb   = toe;
    subframe.omega     = signed_scale(32, omega, -31.0) * SEMI_CIRCLE_TO_RAD;
    subframe.omega0    = signed_scale(32, omega0, -31.0) * SEMI_CIRCLE_TO_RAD;
    subframe.omega_dot = signed_scale(24, omega_dot, -43.0) * SEMI_CIRCLE_TO_RAD;
    subframe.i0        = signed_scale(32, i0, -31.0) * SEMI_CIRCLE_TO_RAD;
    subframe.idot      = signed_scale(14, idot, -43.0) * SEMI_CIRCLE_TO_RAD;
    subframe.cic       = signed_scale(18, cic, -31.0);
    subframe.cis       = signed_scale(18, cis, -31.0);

    VERBOSEF("subframe3: sow=%g, toe=%u, i0=%g, cic=%g, omega_dot=%g, cis=%g, idot=%g, omega0=%g, "
             "omega=%g",
             subframe.sow, subframe.toe_lsb, subframe.i0, subframe.cic, subframe.omega_dot,
             subframe.cis, subframe.idot, subframe.omega0, subframe.omega);

    return true;
}

bool D1Subframe::decode(Words const& words, uint8_t sv_id, D1Subframe& subframe) NOEXCEPT {
    VERBOSEF("decoding subframe");
    LOGLET_VINDENT_SCOPE();

    if (words.size() != 300) {
        VERBOSEF("invalid number of words: %zu, expected: 300 bits", words.size());
        return false;
    }

    auto id = words.u8(15, 3);
    if (sv_id < 6 || sv_id > 58) {
        VERBOSEF("invalid sv_id: %u", sv_id);
        return false;
    }

    subframe.subframe_id = id;
    subframe.sv_id       = sv_id;

    switch (id) {
    case 1: return decode_subframe1(words, subframe.subframe1);
    case 2: return decode_subframe2(words, subframe.subframe2);
    case 3: return decode_subframe3(words, subframe.subframe3);
    default: VERBOSEF("unknown subframe id: %u", id); return false;
    }
}

bool D1Collector::process(uint8_t prn, D1Subframe const& subframe,
                          ephemeris::BdsEphemeris& ephemeris) NOEXCEPT {
    auto& internal_ephemeris = mBuffer[prn];

    // Process subframe based on its type
    switch (subframe.subframe_id) {
    case 1:
        if (internal_ephemeris.subframe1) {
            VERBOSEF("discarding previous subframe 1 for PRN %u", prn);
            internal_ephemeris.subframe1 = false;
        }
        if (internal_ephemeris.subframe2) {
            VERBOSEF("discarding previous subframe 2 for PRN %u", prn);
            internal_ephemeris.subframe2 = false;
        }
        if (internal_ephemeris.subframe3) {
            VERBOSEF("discarding previous subframe 3 for PRN %u", prn);
            internal_ephemeris.subframe3 = false;
        }

        internal_ephemeris.subframe1      = true;
        internal_ephemeris.subframe1_data = subframe.subframe1;
        break;
    case 2:
        if (internal_ephemeris.subframe2) {
            VERBOSEF("discarding previous subframe 2 for PRN %u", prn);
            internal_ephemeris.subframe2 = false;
        }
        if (internal_ephemeris.subframe3) {
            VERBOSEF("discarding previous subframe 3 for PRN %u", prn);
            internal_ephemeris.subframe3 = false;
        }

        if (!internal_ephemeris.subframe1) {
            VERBOSEF("subframe 2 received before subframe 1 for PRN %u", prn);
        } else {
            internal_ephemeris.subframe2      = true;
            internal_ephemeris.subframe2_data = subframe.subframe2;
        }
        break;
    case 3:
        if (internal_ephemeris.subframe3) {
            VERBOSEF("discarding previous subframe 3 for PRN %u", prn);
            internal_ephemeris.subframe3 = false;
        }

        if (!internal_ephemeris.subframe1) {
            VERBOSEF("subframe 3 received before subframe 1 for PRN %u", prn);
        } else if (!internal_ephemeris.subframe2) {
            VERBOSEF("subframe 3 received before subframe 2 for PRN %u", prn);
        } else {
            internal_ephemeris.subframe3      = true;
            internal_ephemeris.subframe3_data = subframe.subframe3;
        }
        break;
    default:
        // Unknown subframe type
        return false;
    }

    if (internal_ephemeris.subframe1 && internal_ephemeris.subframe2 &&
        internal_ephemeris.subframe3) {
        // Check that SOW is consistent
        if (internal_ephemeris.subframe1_data.sow == internal_ephemeris.subframe2_data.sow + 6 &&
            internal_ephemeris.subframe1_data.sow == internal_ephemeris.subframe3_data.sow + 12) {
            VERBOSEF("SOW mismatch for PRN %u: %u, %u, %u", prn,
                     internal_ephemeris.subframe1_data.sow, internal_ephemeris.subframe2_data.sow,
                     internal_ephemeris.subframe3_data.sow);
            internal_ephemeris.subframe1 = false;
            internal_ephemeris.subframe2 = false;
            internal_ephemeris.subframe3 = false;
            return false;
        }

        VERBOSEF("processing ephemeris for PRN %u (week: %u)", prn,
                 internal_ephemeris.subframe1_data.wn);

        auto const& sf1 = internal_ephemeris.subframe1_data;
        auto const& sf2 = internal_ephemeris.subframe2_data;
        auto const& sf3 = internal_ephemeris.subframe3_data;

        ephemeris.prn               = prn;
        ephemeris.week_number       = sf1.wn;
        ephemeris.sv_health         = sf1.sat_h1;
        ephemeris.iodc              = sf1.aodc;
        ephemeris.iode              = sf1.aode;
        ephemeris.toc               = sf1.toc;
        auto toe      = sf2.toe_msb << 15 | sf3.toe_lsb;
        ephemeris.toe = static_cast<double>(toe) * 8.0;

        ephemeris.af0               = sf1.a0;
        ephemeris.af1               = sf1.a1;
        ephemeris.af2               = sf1.a2;

        ephemeris.delta_n           = sf2.delta_n;
        ephemeris.m0                = sf2.m0;
        ephemeris.e                 = sf2.e;
        ephemeris.cuc               = sf2.cuc;
        ephemeris.cus               = sf2.cus;
        ephemeris.crc               = sf2.crc;
        ephemeris.crs               = sf2.crs;
        ephemeris.a                 = sf2.sqrt_a * sf2.sqrt_a;
        
        ephemeris.i0                = sf3.i0;
        ephemeris.idot              = sf3.idot;
        ephemeris.omega0            = sf3.omega0;
        ephemeris.omega             = sf3.omega;
        ephemeris.omega_dot         = sf3.omega_dot;
        ephemeris.cic               = sf3.cic;
        ephemeris.cis               = sf3.cis;

        internal_ephemeris.subframe1 = false;
        internal_ephemeris.subframe2 = false;
        internal_ephemeris.subframe3 = false;
        return true;
    } else {
        return false;
    }
}

}  // namespace nav
}  // namespace format

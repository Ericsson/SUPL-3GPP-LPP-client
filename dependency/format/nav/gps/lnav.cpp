#include "gps/lnav.hpp"

#include <cmath>

#include <ephemeris/gps.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE(nav);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(nav)

namespace format {
namespace nav {

namespace gps {
namespace lnav {

static Tlm decode_tlm(Words const& words) {
    Tlm tlm{};
    tlm.preamble              = words.u8(0, 8);
    tlm.tlm                   = words.u16(8, 14);
    tlm.integrity_status_flag = words.b1(22);
    tlm.reserved              = words.b1(23);
    return tlm;
}

static How decode_how(Words const& words) {
    How how{};
    how.tow                = words.u32(30, 17);
    how.alert_flag         = words.b1(47);
    how.anti_spoofing_flag = words.b1(48);
    how.subframe_id        = words.u8(49, 3);
    return how;
}

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

static bool decode_subframe1(Words const& words, Subframe1& subframe) {
    FUNCTION_SCOPE();
    auto week_number    = words.u16(60, 10);
    auto ca_or_p_on_l2  = words.u8(70, 2);
    auto ura_index      = words.u8(72, 4);
    auto sv_health      = words.u8(76, 6);
    auto iodc_msb       = words.u8(82, 2);
    auto l2_p_data_flag = words.b1(90);
    auto tgd            = words.u8(196, 8);
    auto iodc_lsb       = words.u8(210, 8);
    auto toc            = words.u16(218, 16);
    auto af2            = words.u8(240, 8);
    auto af1            = words.u16(248, 16);
    auto af0            = words.u32(270, 22);

    subframe.week_number    = week_number;
    subframe.ca_or_p_on_l2  = ca_or_p_on_l2;
    subframe.ura_index      = ura_index;
    subframe.sv_health      = sv_health;
    subframe.iodc           = static_cast<uint16_t>((static_cast<uint32_t>(iodc_msb) << 8) |
                                                    static_cast<uint32_t>(iodc_lsb));
    subframe.l2_p_data_flag = l2_p_data_flag;
    subframe.tgd            = signed_scale(8, tgd, -31);
    subframe.toc            = unsigned_scale(toc, 4);
    subframe.af2            = signed_scale(8, af2, -55);
    subframe.af1            = signed_scale(16, af1, -43);
    subframe.af0            = signed_scale(22, af0, -31);

    VERBOSEF(
        "subframe1: week_number: %u, ca_or_p_on_l2: %u, ura_index: %u, sv_health: %u, iodc: %u, "
        "l2_p_data_flag: %d, tgd: %g (%u), toc: %g (%u), af2: %g (%u), af1: %g (%u), af0: %g (%u)",
        subframe.week_number, subframe.ca_or_p_on_l2, subframe.ura_index, subframe.sv_health,
        subframe.iodc, subframe.l2_p_data_flag, subframe.tgd, tgd, subframe.toc, toc, subframe.af2,
        af2, subframe.af1, af1, subframe.af0, af0);
    return true;
}

static bool decode_subframe2(Words const& words, Subframe2& subframe) {
    FUNCTION_SCOPE();
    auto iode              = words.u8(60, 8);
    auto crs               = words.u16(68, 16);
    auto delta_n           = words.u16(90, 16);
    auto m0_msb            = words.u8(106, 8);
    auto m0_lsb            = words.u32(120, 24);
    auto cuc               = words.u16(150, 16);
    auto e_msb             = words.u8(166, 8);
    auto e_lsb             = words.u32(180, 24);
    auto cus               = words.u16(210, 16);
    auto sqrt_a_msb        = words.u8(226, 8);
    auto sqrt_a_lsb        = words.u32(240, 24);
    auto toe               = words.u16(270, 16);
    auto fit_interval_flag = words.b1(286);
    auto aodo              = words.u8(287, 5);

    auto m0     = (static_cast<uint32_t>(m0_msb) << 24) | m0_lsb;
    auto e      = (static_cast<uint32_t>(e_msb) << 24) | e_lsb;
    auto sqrt_a = (static_cast<uint32_t>(sqrt_a_msb) << 24) | sqrt_a_lsb;

#define SEMI_CIRCLE_TO_RAD 3.1415926535898
    subframe.iode              = iode;
    subframe.crs               = signed_scale(16, crs, -5);
    subframe.delta_n           = signed_scale(16, delta_n, -43) * SEMI_CIRCLE_TO_RAD;
    subframe.m0                = signed_scale(32, m0, -31) * SEMI_CIRCLE_TO_RAD;
    subframe.cuc               = signed_scale(16, cuc, -29);
    subframe.e                 = unsigned_scale(e, -33);
    subframe.cus               = signed_scale(16, cus, -29);
    subframe.sqrt_a            = unsigned_scale(sqrt_a, -19);
    subframe.toe               = unsigned_scale(toe, 4);
    subframe.fit_interval_flag = fit_interval_flag;
    subframe.aodo              = aodo;

    VERBOSEF("subframe2: iode: %u, crs: %g (%" PRIi64 "), delta_n: %g (%" PRIi64
             "), m0: %g (%" PRIi64 "), cuc: %g (%" PRIi64 "), e: "
             "%g (%u), "
             "cus: %g (%" PRIi64
             "), sqrt_a: %g (%u), toe: %g (%u), fit_interval_flag: %d, aodo: %u",
             subframe.iode, subframe.crs, signed_transform(16, crs), subframe.delta_n,
             signed_transform(16, delta_n), subframe.m0, signed_transform(32, m0), subframe.cuc,
             signed_transform(16, cuc), subframe.e, e, subframe.cus, signed_transform(16, cus),
             subframe.sqrt_a, sqrt_a, subframe.toe, toe, subframe.fit_interval_flag, subframe.aodo);
    return true;
}

static bool decode_subframe3(Words const& words, Subframe3& subframe) {
    FUNCTION_SCOPE();
    auto cic        = words.u16(60, 16);
    auto omega0_msb = words.u8(76, 8);
    auto omega0_lsb = words.u32(90, 24);
    auto cis        = words.u16(120, 16);
    auto i0_msb     = words.u8(136, 8);
    auto i0_lsb     = words.u32(150, 24);
    auto crc        = words.u16(180, 16);
    auto omega_msb  = words.u8(196, 8);
    auto omega_lsb  = words.u32(210, 24);
    auto omega_dot  = words.u32(240, 24);
    auto iode       = words.u8(270, 8);
    auto idot       = words.u32(278, 14);

    auto omega0 = (static_cast<uint32_t>(omega0_msb) << 24) | omega0_lsb;
    auto i0     = (static_cast<uint32_t>(i0_msb) << 24) | i0_lsb;
    auto omega  = (static_cast<uint32_t>(omega_msb) << 24) | omega_lsb;

    subframe.cic       = signed_scale(16, cic, -29);
    subframe.omega0    = signed_scale(32, omega0, -31) * SEMI_CIRCLE_TO_RAD;
    subframe.cis       = signed_scale(16, cis, -29);
    subframe.i0        = signed_scale(32, i0, -31) * SEMI_CIRCLE_TO_RAD;
    subframe.crc       = signed_scale(16, crc, -5);
    subframe.omega     = signed_scale(32, omega, -31) * SEMI_CIRCLE_TO_RAD;
    subframe.omega_dot = signed_scale(24, omega_dot, -43) * SEMI_CIRCLE_TO_RAD;
    subframe.iode      = iode;
    subframe.idot      = signed_scale(14, idot, -43) * SEMI_CIRCLE_TO_RAD;

    VERBOSEF("subframe3: cic: %g (%u), omega0: %g (%u), cis: %g (%u), i0: %g (%u), crc: %g (%u), "
             "omega: %g (%u), omega_dot: %g (%u), iode: %u, idot: %g (%u)",
             subframe.cic, cic, subframe.omega0, omega0, subframe.cis, cis, subframe.i0, i0,
             subframe.crc, crc, subframe.omega, omega, subframe.omega_dot, omega_dot, subframe.iode,
             subframe.idot);
    return true;
}

static bool decode_subframe4_page18(Words const& words, Subframe4::Page18& page) {
    FUNCTION_SCOPE();

    auto a0 = words.u8(68, 8);
    auto a1 = words.u8(76, 8);
    auto a2 = words.u8(90, 8);
    auto a3 = words.u8(98, 8);

    auto b0 = words.u8(106, 8);
    auto b1 = words.u8(120, 8);
    auto b2 = words.u8(128, 8);
    auto b3 = words.u8(136, 8);

    auto A1     = words.u32(150, 24);
    auto A0_msb = words.u32(180, 24);
    auto A0_lsb = words.u32(210, 8);
    auto A0     = (static_cast<uint32_t>(A0_msb) << 24) | A0_lsb;

    auto t_ot   = words.u32(218, 8);
    auto wn_t   = words.u8(226, 8);
    auto dt_ls  = words.u8(240, 8);
    auto wn_lsf = words.u8(248, 8);
    auto dn     = words.u8(256, 8);
    auto dt_lsf = words.u8(270, 8);

    page.a[0] = signed_scale(8, a0, -30);
    page.a[1] = signed_scale(8, a1, -27);
    page.a[2] = signed_scale(8, a2, -24);
    page.a[3] = signed_scale(8, a3, -24);

    page.b[0] = signed_scale(8, b0, 11);
    page.b[1] = signed_scale(8, b1, 14);
    page.b[2] = signed_scale(8, b2, 16);
    page.b[3] = signed_scale(8, b3, 16);

    VERBOSEF("subframe4_18: iono: a0: %g (%u), a1: %g (%u), a2: %g (%u), a3: %g (%u), b0: %g (%u), "
             "b1: %g (%u), b2: %g (%u), b3: %g (%u)",
             page.a[0], a0, page.a[1], a1, page.a[2], a2, page.a[3], a3, page.b[0], b0, page.b[1],
             b1, page.b[2], b2, page.b[3], b3);

    page.A0          = signed_scale(32, A0, -30);
    page.A1          = signed_scale(24, A1, -50);
    page.delta_t_ls  = signed_scale(8, dt_ls, 0);
    page.t_ot        = unsigned_scale(t_ot, 12);
    page.wn_t        = unsigned_scale(wn_t, 0);
    page.wn_lsf      = unsigned_scale(wn_lsf, 0);
    page.dn          = unsigned_scale(dn, 0);
    page.delta_t_lsf = signed_scale(8, dt_lsf, 0);

    VERBOSEF("subframe4_18:  utc: A0: %g (%u), A1: %g (%u), delta_t_ls: %g (%u), t_ot: %g (%u), "
             "wn_t: %g (%u), wn_lsf: %g (%u), dn: %g (%u), delta_t_lsf: %g (%u)",
             page.A0, A0, page.A1, A1, page.delta_t_ls, dt_ls, page.t_ot, t_ot, page.wn_t, wn_t,
             page.wn_lsf, wn_lsf, page.dn, dn, page.delta_t_lsf, dt_lsf);
    return true;
}

static bool decode_subframe4(Words const& words, Subframe4& subframe) {
    FUNCTION_SCOPE();

    auto data_id = words.u8(60, 2);
    auto sv_id   = words.u8(62, 6);

    subframe.data_id = data_id;
    subframe.sv_id   = sv_id;

    VERBOSEF("subframe4: data_id: %u, sv_id: %u (page_id: %i)", subframe.data_id, subframe.sv_id,
             subframe.sv_id - 38);

    switch (subframe.sv_id) {
    case 56: return decode_subframe4_page18(words, subframe.page18);
    default: return true;  // NOTE(ewasjon): Don't fail on unknown pages
    }
}

bool Subframe::decode(Words const& words, Subframe& subframe) NOEXCEPT {
    FUNCTION_SCOPE();

    if (words.size() != 300) {
        VERBOSEF("invalid number of words: %zu, expected: 300 bits", words.size());
        return false;
    }

    subframe.tlm = decode_tlm(words);
    VERBOSEF("tlm: preamble: 0x%02X, tlm: %u, integrity_status_flag: %d, reserved: %d",
             subframe.tlm.preamble, subframe.tlm.tlm, subframe.tlm.integrity_status_flag,
             subframe.tlm.reserved);
    if (subframe.tlm.preamble != 0x8B) {
        VERBOSEF("invalid preamble: 0x%02X, expected: 0x8B", subframe.tlm.preamble);
        return false;
    }

    subframe.how = decode_how(words);
    VERBOSEF("how: tow: %u, alert_flag: %d, anti_spoofing_flag: %d, subframe_id: %u",
             subframe.how.tow, subframe.how.alert_flag, subframe.how.anti_spoofing_flag,
             subframe.how.subframe_id);

    switch (subframe.how.subframe_id) {
    case 1: return decode_subframe1(words, subframe.subframe1);
    case 2: return decode_subframe2(words, subframe.subframe2);
    case 3: return decode_subframe3(words, subframe.subframe3);
    case 4: return decode_subframe4(words, subframe.subframe4);
    default: VERBOSEF("unsupported subframe id: %u", subframe.how.subframe_id); return true;
    }
}

bool EphemerisCollector::process(uint8_t prn, lnav::Subframe const& subframe,
                                 ephemeris::GpsEphemeris& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto& internal_ephemeris = mBuffer[prn];

    // Process subframe based on its type
    switch (subframe.how.subframe_id) {
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
        VERBOSEF("unsupported subframe id: %u", subframe.how.subframe_id);
        return false;
    }

    if (internal_ephemeris.subframe1 && internal_ephemeris.subframe2 &&
        internal_ephemeris.subframe3) {
        // Check that the IODEs match
        if (internal_ephemeris.subframe2_data.iode != internal_ephemeris.subframe3_data.iode) {
            DEBUGF("IODE mismatch for PRN %u (subframe 2: %u, subframe 3: %u)", prn,
                   internal_ephemeris.subframe2_data.iode, internal_ephemeris.subframe3_data.iode);
            internal_ephemeris.subframe1 = false;
            internal_ephemeris.subframe2 = false;
            internal_ephemeris.subframe3 = false;
            return false;
        }

        VERBOSEF("processing ephemeris for PRN %u (week: %u, IODE: %u)", prn,
                 internal_ephemeris.subframe1_data.week_number,
                 internal_ephemeris.subframe2_data.iode);

        auto const& sf1 = internal_ephemeris.subframe1_data;
        auto const& sf2 = internal_ephemeris.subframe2_data;
        auto const& sf3 = internal_ephemeris.subframe3_data;

        ephemeris.prn               = prn;
        ephemeris.week_number       = sf1.week_number;
        ephemeris.ca_or_p_on_l2     = sf1.ca_or_p_on_l2;
        ephemeris.ura_index         = sf1.ura_index;
        ephemeris.sv_health         = sf1.sv_health;
        ephemeris.iodc              = sf1.iodc;
        ephemeris.l2_p_data_flag    = sf1.l2_p_data_flag;
        ephemeris.tgd               = sf1.tgd;
        ephemeris.toc               = sf1.toc;
        ephemeris.af2               = sf1.af2;
        ephemeris.af1               = sf1.af1;
        ephemeris.af0               = sf1.af0;
        ephemeris.iode              = sf2.iode;
        ephemeris.crs               = sf2.crs;
        ephemeris.delta_n           = sf2.delta_n;
        ephemeris.m0                = sf2.m0;
        ephemeris.cuc               = sf2.cuc;
        ephemeris.e                 = sf2.e;
        ephemeris.cus               = sf2.cus;
        ephemeris.a                 = sf2.sqrt_a * sf2.sqrt_a;
        ephemeris.toe               = sf2.toe;
        ephemeris.fit_interval_flag = sf2.fit_interval_flag;
        ephemeris.aodo              = sf2.aodo;  // TODO(ewasjon): AODO is not a ephemeris field
        ephemeris.cic               = sf3.cic;
        ephemeris.omega0            = sf3.omega0;
        ephemeris.cis               = sf3.cis;
        ephemeris.i0                = sf3.i0;
        ephemeris.crc               = sf3.crc;
        ephemeris.omega             = sf3.omega;
        ephemeris.omega_dot         = sf3.omega_dot;
        ephemeris.idot              = sf3.idot;

        // [3GPP TS 37.355]: In the case of broadcasted GPS NAV ephemeris, the iod contains the IODC
        // as described in [4].
        ephemeris.lpp_iod = ephemeris.iode;

        internal_ephemeris.subframe1 = false;
        internal_ephemeris.subframe2 = false;
        internal_ephemeris.subframe3 = false;
        return true;
    } else {
        return false;
    }
}

}  // namespace lnav
}  // namespace gps
}  // namespace nav
}  // namespace format

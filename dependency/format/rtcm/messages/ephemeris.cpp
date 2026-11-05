#include "ephemeris.hpp"
#include "1019.hpp"
#include "1042.hpp"
#include "1046.hpp"

#include <time/bdt.hpp>

namespace format {
namespace rtcm {

ephemeris::GpsEphemeris to_ephemeris(Rtcm1019 const& rtcm) NOEXCEPT {
    ephemeris::GpsEphemeris ephemeris{};
    ephemeris.prn           = rtcm.prn;
    ephemeris.week_number   = rtcm.week;
    ephemeris.ca_or_p_on_l2 = rtcm.code_on_l2;
    ephemeris.ura_index     = rtcm.sv_accuracy;
    ephemeris.sv_health     = rtcm.sv_health;
    // [3GPP TS 37.355]: In the case of broadcasted GPS NAV ephemeris, the iod contains the IODC
    // as described in [4].
    ephemeris.lpp_iod           = rtcm.iodc;
    ephemeris.iodc              = rtcm.iodc;
    ephemeris.iode              = rtcm.iode;
    ephemeris.aodo              = 0;
    ephemeris.toc               = rtcm.t_oc;
    ephemeris.toe               = rtcm.t_oe;
    ephemeris.tgd               = rtcm.t_gd;
    ephemeris.af2               = rtcm.a_f2;
    ephemeris.af1               = rtcm.a_f1;
    ephemeris.af0               = rtcm.a_f0;
    ephemeris.crc               = rtcm.c_rc;
    ephemeris.crs               = rtcm.c_rs;
    ephemeris.cuc               = rtcm.c_uc;
    ephemeris.cus               = rtcm.c_us;
    ephemeris.cic               = rtcm.c_ic;
    ephemeris.cis               = rtcm.c_is;
    ephemeris.e                 = rtcm.e;
    ephemeris.m0                = rtcm.m_0;
    ephemeris.delta_n           = rtcm.delta_n;
    ephemeris.a                 = rtcm.sqrt_a * rtcm.sqrt_a;
    ephemeris.i0                = rtcm.i_0;
    ephemeris.omega0            = rtcm.omega_0;
    ephemeris.omega             = rtcm.omega;
    ephemeris.omega_dot         = rtcm.omegadot;
    ephemeris.idot              = rtcm.idot;
    ephemeris.fit_interval_flag = rtcm.fit;
    ephemeris.l2_p_data_flag    = rtcm.l2_p_data_flag;

    return ephemeris;
}

ephemeris::BdsEphemeris to_ephemeris(Rtcm1042 const& rtcm) NOEXCEPT {
    ephemeris::BdsEphemeris ephemeris{};
    ephemeris.prn         = rtcm.prn;
    ephemeris.week_number = rtcm.week_number;
    ephemeris.sv_health   = rtcm.sv_health;
    ephemeris.lpp_iod     = static_cast<uint16_t>(static_cast<uint32_t>(rtcm.toe) >> 9);
    ephemeris.iodc        = (static_cast<uint32_t>(rtcm.toc) / 720) % 240;
    ephemeris.iode        = (static_cast<uint32_t>(rtcm.toe) / 720) % 240;
    ephemeris.aode        = rtcm.aode;
    ephemeris.aodc        = rtcm.aodc;
    ephemeris.toc_time =
        ts::Bdt::from_week_tow(rtcm.week_number, static_cast<int64_t>(rtcm.toc), 0.0);
    ephemeris.toe_time =
        ts::Bdt::from_week_tow(rtcm.week_number, static_cast<int64_t>(rtcm.toe), 0.0);
    ephemeris.toc       = rtcm.toc;
    ephemeris.toe       = rtcm.toe;
    ephemeris.af2       = rtcm.af2;
    ephemeris.af1       = rtcm.af1;
    ephemeris.af0       = rtcm.af0;
    ephemeris.crc       = rtcm.crc;
    ephemeris.crs       = rtcm.crs;
    ephemeris.cuc       = rtcm.cuc;
    ephemeris.cus       = rtcm.cus;
    ephemeris.cic       = rtcm.cic;
    ephemeris.cis       = rtcm.cis;
    ephemeris.e         = rtcm.e;
    ephemeris.m0        = rtcm.m0;
    ephemeris.delta_n   = rtcm.delta_n;
    ephemeris.a         = rtcm.sqrt_a * rtcm.sqrt_a;
    ephemeris.i0        = rtcm.i0;
    ephemeris.omega0    = rtcm.omega0;
    ephemeris.omega     = rtcm.omega;
    ephemeris.omega_dot = rtcm.omega_dot;
    ephemeris.idot      = rtcm.idot;

    return ephemeris;
}

ephemeris::GalEphemeris to_ephemeris(Rtcm1046 const& rtcm) NOEXCEPT {
    ephemeris::GalEphemeris ephemeris{};
    ephemeris.prn         = rtcm.prn;
    ephemeris.week_number = rtcm.week_number;
    ephemeris.lpp_iod     = rtcm.iod_nav;
    ephemeris.iod_nav     = rtcm.iod_nav;
    ephemeris.toc         = rtcm.toc;
    ephemeris.toe         = rtcm.toe;
    ephemeris.af2         = rtcm.af2;
    ephemeris.af1         = rtcm.af1;
    ephemeris.af0         = rtcm.af0;
    ephemeris.crc         = rtcm.crc;
    ephemeris.crs         = rtcm.crs;
    ephemeris.cuc         = rtcm.cuc;
    ephemeris.cus         = rtcm.cus;
    ephemeris.cic         = rtcm.cic;
    ephemeris.cis         = rtcm.cis;
    ephemeris.e           = rtcm.e;
    ephemeris.m0          = rtcm.m0;
    ephemeris.delta_n     = rtcm.delta_n;
    ephemeris.a           = rtcm.sqrt_a * rtcm.sqrt_a;
    ephemeris.i0          = rtcm.i0;
    ephemeris.omega0      = rtcm.omega0;
    ephemeris.omega       = rtcm.omega;
    ephemeris.omega_dot   = rtcm.omega_dot;
    ephemeris.idot        = rtcm.idot;

    return ephemeris;
}

}  // namespace rtcm
}  // namespace format

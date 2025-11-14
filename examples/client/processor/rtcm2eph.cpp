#include "rtcm2eph.hpp"
#include <loglet/loglet.hpp>
#include <time/bdt.hpp>

LOGLET_MODULE2(p, rtcm2eph);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, rtcm2eph)

void Rtcm2Eph::handle_gps_lnav(streamline::System& system, format::rtcm::Rtcm1019* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();

    ephemeris::GpsEphemeris ephemeris{};
    ephemeris.prn           = rtcm->prn;
    ephemeris.week_number   = rtcm->week;
    ephemeris.ca_or_p_on_l2 = rtcm->code_on_l2;
    ephemeris.ura_index     = rtcm->sv_accuracy;
    ephemeris.sv_health     = rtcm->sv_health;
    // [3GPP TS 37.355]: In the case of broadcasted GPS NAV ephemeris, the iod contains the IODC
    // as described in [4].
    ephemeris.lpp_iod           = rtcm->iodc;
    ephemeris.iodc              = rtcm->iodc;
    ephemeris.iode              = rtcm->iode;
    ephemeris.aodo              = 0;
    ephemeris.toc               = rtcm->t_oc;
    ephemeris.toe               = rtcm->t_oe;
    ephemeris.tgd               = rtcm->t_gd;
    ephemeris.af2               = rtcm->a_f2;
    ephemeris.af1               = rtcm->a_f1;
    ephemeris.af0               = rtcm->a_f0;
    ephemeris.crc               = rtcm->c_rc;
    ephemeris.crs               = rtcm->c_rs;
    ephemeris.cuc               = rtcm->c_uc;
    ephemeris.cus               = rtcm->c_us;
    ephemeris.cic               = rtcm->c_ic;
    ephemeris.cis               = rtcm->c_is;
    ephemeris.e                 = rtcm->e;
    ephemeris.m0                = rtcm->m_0;
    ephemeris.delta_n           = rtcm->delta_n;
    ephemeris.a                 = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0                = rtcm->i_0;
    ephemeris.omega0            = rtcm->omega_0;
    ephemeris.omega             = rtcm->omega;
    ephemeris.omega_dot         = rtcm->omegadot;
    ephemeris.idot              = rtcm->idot;
    ephemeris.fit_interval_flag = rtcm->fit;
    ephemeris.l2_p_data_flag    = rtcm->l2_p_data_flag;

    system.push(std::move(ephemeris));
}

void Rtcm2Eph::handle_gps(streamline::System& system, format::rtcm::Rtcm1019* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.gps) return;
    if (rtcm->type() == 1019) {
        handle_gps_lnav(system, rtcm);
    } else {
        VERBOSEF("not rtcm 1019 but rtcm %d", rtcm->type());
    }
}

void Rtcm2Eph::handle_bds_d1(streamline::System& system, format::rtcm::Rtcm1042* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();

    ephemeris::BdsEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;
    ephemeris.sv_health   = rtcm->sv_health;
    ephemeris.lpp_iod     = static_cast<uint16_t>(static_cast<uint32_t>(rtcm->toe) >> 9);
    ephemeris.iodc        = (static_cast<uint32_t>(rtcm->toc) / 720) % 240;
    ephemeris.iode        = (static_cast<uint32_t>(rtcm->toe) / 720) % 240;
    ephemeris.aode        = rtcm->aode;
    ephemeris.aodc        = rtcm->aodc;
    ephemeris.toc_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toc), 0.0);
    ephemeris.toe_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toe), 0.0);
    ephemeris.toc       = rtcm->toc;
    ephemeris.toe       = rtcm->toe;
    ephemeris.af2       = rtcm->af2;
    ephemeris.af1       = rtcm->af1;
    ephemeris.af0       = rtcm->af0;
    ephemeris.crc       = rtcm->crc;
    ephemeris.crs       = rtcm->crs;
    ephemeris.cuc       = rtcm->cuc;
    ephemeris.cus       = rtcm->cus;
    ephemeris.cic       = rtcm->cic;
    ephemeris.cis       = rtcm->cis;
    ephemeris.e         = rtcm->e;
    ephemeris.m0        = rtcm->m0;
    ephemeris.delta_n   = rtcm->delta_n;
    ephemeris.a         = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0        = rtcm->i0;
    ephemeris.omega0    = rtcm->omega0;
    ephemeris.omega     = rtcm->omega;
    ephemeris.omega_dot = rtcm->omega_dot;
    ephemeris.idot      = rtcm->idot;

    system.push(std::move(ephemeris));
}

void Rtcm2Eph::handle_bds(streamline::System& system, format::rtcm::Rtcm1042* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.beidou) return;
    if (rtcm->type() == 1042) {
        handle_bds_d1(system, rtcm);
    } else {
        VERBOSEF("not rtcm 1042 but rtcm %d", rtcm->type());
    }
}

void Rtcm2Eph::handle_gal_inav(streamline::System& system, format::rtcm::Rtcm1046* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();

    ephemeris::GalEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;
    ephemeris.lpp_iod     = rtcm->iod_nav;
    ephemeris.iod_nav     = rtcm->iod_nav;
    ephemeris.toc         = rtcm->toc;
    ephemeris.toe         = rtcm->toe;
    ephemeris.af2         = rtcm->af2;
    ephemeris.af1         = rtcm->af1;
    ephemeris.af0         = rtcm->af0;
    ephemeris.crc         = rtcm->crc;
    ephemeris.crs         = rtcm->crs;
    ephemeris.cuc         = rtcm->cuc;
    ephemeris.cus         = rtcm->cus;
    ephemeris.cic         = rtcm->cic;
    ephemeris.cis         = rtcm->cis;
    ephemeris.e           = rtcm->e;
    ephemeris.m0          = rtcm->m0;
    ephemeris.delta_n     = rtcm->delta_n;
    ephemeris.a           = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0          = rtcm->i0;
    ephemeris.omega0      = rtcm->omega0;
    ephemeris.omega       = rtcm->omega;
    ephemeris.omega_dot   = rtcm->omega_dot;
    ephemeris.idot        = rtcm->idot;

    system.push(std::move(ephemeris));
}

void Rtcm2Eph::handle_gal(streamline::System& system, format::rtcm::Rtcm1046* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.galileo) return;
    if (rtcm->type() == 1046) {
        handle_gal_inav(system, rtcm);
    } else {
        VERBOSEF("not rtcm 1046 but rtcm %d", rtcm->type());
    }
}

void Rtcm2Eph::inspect(streamline::System& system, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto rtcm1019 = dynamic_cast<format::rtcm::Rtcm1019*>(ptr);
    if (rtcm1019) return handle_gps(system, rtcm1019);
    auto rtcm1042 = dynamic_cast<format::rtcm::Rtcm1042*>(ptr);
    if (rtcm1042) return handle_bds(system, rtcm1042);
    auto rtcm1046 = dynamic_cast<format::rtcm::Rtcm1046*>(ptr);
    if (rtcm1046) return handle_gal(system, rtcm1046);
}

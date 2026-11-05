#include "rtcm2eph.hpp"

#include <format/rtcm/ephemeris.hpp>
#include <loglet/loglet.hpp>
#include <time/bdt.hpp>

LOGLET_MODULE2(p, rtcm2eph);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, rtcm2eph)

void Rtcm2Eph::handle_gps_lnav(streamline::System& system, format::rtcm::Rtcm1019* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    system.push(format::rtcm::to_ephemeris(*rtcm), mTag);
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
    system.push(format::rtcm::to_ephemeris(*rtcm), mTag);
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
    system.push(format::rtcm::to_ephemeris(*rtcm), mTag);
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

void Rtcm2Eph::handle_glo(streamline::System& system, format::rtcm::Rtcm1020* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.glonass) return;
    system.push(format::rtcm::to_ephemeris(*rtcm, ts::Tai::now()), mTag);
}

void Rtcm2Eph::handle_qzs(streamline::System& system, format::rtcm::Rtcm1044* rtcm) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mConfig.qzss) return;
    system.push(format::rtcm::to_ephemeris(*rtcm), mTag);
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
    auto rtcm1020 = dynamic_cast<format::rtcm::Rtcm1020*>(ptr);
    if (rtcm1020) return handle_glo(system, rtcm1020);
    auto rtcm1044 = dynamic_cast<format::rtcm::Rtcm1044*>(ptr);
    if (rtcm1044) return handle_qzs(system, rtcm1044);
}

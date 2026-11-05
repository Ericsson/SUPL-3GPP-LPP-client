#include "lpp2eph.hpp"

#include <cmath>

#include <asn.1/bit_string.hpp>
#include <external_warnings.hpp>
#include <gnss/satellite_id.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/tai.hpp>
#include <time/utc.hpp>

EXTERNAL_WARNINGS_PUSH
#include <A-GNSS-ProvideAssistanceData.h>
#include <GLONASS-ClockModel.h>
#include <GNSS-AuxiliaryInformation.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-ID-GLONASS-SatElement.h>
#include <GNSS-ID-GLONASS.h>
#include <GNSS-ID.h>
#include <GNSS-NavModelSatelliteElement.h>
#include <GNSS-NavigationModel.h>
#include <GNSS-ReferenceTime.h>
#include <GNSS-SystemTime.h>
#include <NavModel-GLONASS-ECEF.h>
#include <NavModelKeplerianSet.h>
#include <NavModelNAV-KeplerianSet.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <StandardClockModelElement.h>
EXTERNAL_WARNINGS_POP

LOGLET_MODULE2(p, lpp2eph);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, lpp2eph)

static bool system_time(GNSS_SystemTime const& time, ts::Tai& result) {
    auto tod = static_cast<double>(time.gnss_TimeOfDay);
    if (time.gnss_TimeOfDayFrac_msec)
        tod += static_cast<double>(*time.gnss_TimeOfDayFrac_msec) * 1e-3;
    auto day = time.gnss_DayNumber;
    switch (time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps:
    case GNSS_ID__gnss_id_qzss: result = ts::Tai(ts::Gps::from_day_tod(day, tod)); return true;
    case GNSS_ID__gnss_id_glonass: result = ts::Tai(ts::Glo::from_day_tod(day, tod)); return true;
    case GNSS_ID__gnss_id_galileo: result = ts::Tai(ts::Gst::from_day_tod(day, tod)); return true;
    case GNSS_ID__gnss_id_bds: result = ts::Tai(ts::Bdt::from_day_tod(day, tod)); return true;
    default: return false;
    }
}

// The week of `toe` closest to the reference time
template <typename T>
static uint16_t nearest_week(T const& reference, double toe) {
    auto week = reference.week();
    auto dt   = toe - reference.time_of_week().full_seconds();
    if (dt > 302400.0) week--;
    if (dt < -302400.0) week++;
    return static_cast<uint16_t>(week);
}

void Lpp2Eph::inspect(streamline::System& system, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!message) return;

    auto pad = lpp::get_provide_assistance_data(message);
    if (!pad) return;
    if (!pad->a_gnss_ProvideAssistanceData) return;

    auto& agnss = *pad->a_gnss_ProvideAssistanceData;

    // The navigation models have no week (day for GLONASS); resolve it from the assistance data
    // reference time, which also works when replaying recorded data. Without one, use the clock.
    ts::Tai reference_time;
    if (agnss.gnss_CommonAssistData && agnss.gnss_CommonAssistData->gnss_ReferenceTime &&
        system_time(agnss.gnss_CommonAssistData->gnss_ReferenceTime->gnss_SystemTime,
                    reference_time)) {
        mReferenceTime    = reference_time;
        mHasReferenceTime = true;
    } else if (!mHasReferenceTime) {
        mReferenceTime = ts::Tai::now();
    }

    if (!agnss.gnss_GenericAssistData) return;

    // the GLONASS frequency channels are needed by the navigation models
    for (int i = 0; i < agnss.gnss_GenericAssistData->list.count; i++) {
        auto& generic = *agnss.gnss_GenericAssistData->list.array[i];
        if (generic.gnss_ID.gnss_id != GNSS_ID__gnss_id_glonass) continue;
        if (!generic.gnss_AuxiliaryInformation) continue;
        process_glo_auxiliary_information(*generic.gnss_AuxiliaryInformation);
    }

    for (int i = 0; i < agnss.gnss_GenericAssistData->list.count; i++) {
        auto& generic = *agnss.gnss_GenericAssistData->list.array[i];
        if (!generic.gnss_NavigationModel) continue;

        auto& nav_model = *generic.gnss_NavigationModel;
        auto  gnss_id   = generic.gnss_ID.gnss_id;

        if (gnss_id == GNSS_ID__gnss_id_gps && mConfig.gps) {
            process_gps_navigation_model(system, nav_model);
        } else if (gnss_id == GNSS_ID__gnss_id_galileo && mConfig.galileo) {
            process_gal_navigation_model(system, nav_model);
        } else if (gnss_id == GNSS_ID__gnss_id_bds && mConfig.beidou) {
            process_bds_navigation_model(system, nav_model);
        } else if (gnss_id == GNSS_ID__gnss_id_qzss && mConfig.qzss) {
            process_qzs_navigation_model(system, nav_model);
        } else if (gnss_id == GNSS_ID__gnss_id_glonass && mConfig.glonass) {
            process_glo_navigation_model(system, nav_model);
        }
    }
}

void Lpp2Eph::process_gps_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto reference = ts::Gps(mReferenceTime);

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_nav_ClockModel) continue;

        bool is_nav_keplerian = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_nav_KeplerianSet;
        bool is_keplerian     = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_keplerianSet;
        if (!is_nav_keplerian && !is_keplerian) continue;

        auto satellite_id = SatelliteId::from_lpp(SatelliteId::Gnss::GPS, sat.svID.satellite_id);
        if (!satellite_id.is_valid()) continue;

        auto prn = satellite_id.prn();
        if (!prn.valid) continue;

        auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;

        helper::BitStringReader iod_reader(&sat.iod);
        uint8_t                 iode = static_cast<uint8_t>(iod_reader.integer<uint16_t>(3, 8));
        uint16_t                iodc = static_cast<uint16_t>(iod_reader.integer<uint16_t>(1, 10));

        ephemeris::GpsEphemeris eph{};
        eph.prn  = prn.value;
        eph.iode = iode;
        eph.iodc = iodc;
        // [3GPP TS 37.355]: In the case of broadcasted GPS NAV ephemeris, the iod contains the IODC
        // as described in [4].
        eph.lpp_iod = iodc;

        if (is_nav_keplerian) {
            auto& kep     = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe       = static_cast<double>(kep.navToe) * 16;
            eph.a         = std::pow(std::ldexp(static_cast<double>(kep.navAPowerHalf), -19), 2);
            eph.delta_n   = std::ldexp(static_cast<double>(kep.navDeltaN), -43) * M_PI;
            eph.m0        = std::ldexp(static_cast<double>(kep.navM0), -31) * M_PI;
            eph.e         = std::ldexp(static_cast<double>(kep.navE), -33);
            eph.omega     = std::ldexp(static_cast<double>(kep.navOmega), -31) * M_PI;
            eph.cuc       = std::ldexp(static_cast<double>(kep.navCuc), -29);
            eph.cus       = std::ldexp(static_cast<double>(kep.navCus), -29);
            eph.crc       = static_cast<double>(kep.navCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.navCrs) * 0.03125;
            eph.cic       = std::ldexp(static_cast<double>(kep.navCic), -29);
            eph.cis       = std::ldexp(static_cast<double>(kep.navCis), -29);
            eph.i0        = std::ldexp(static_cast<double>(kep.navI0), -31) * M_PI;
            eph.idot      = std::ldexp(static_cast<double>(kep.navIDot), -43) * M_PI;
            eph.omega0    = std::ldexp(static_cast<double>(kep.navOmegaA0), -31) * M_PI;
            eph.omega_dot = std::ldexp(static_cast<double>(kep.navOmegaADot), -43) * M_PI;
        } else {
            auto& kep     = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe       = static_cast<double>(kep.keplerToe) * 60;
            eph.a         = std::pow(std::ldexp(static_cast<double>(kep.keplerAPowerHalf), -19), 2);
            eph.delta_n   = std::ldexp(static_cast<double>(kep.keplerDeltaN), -43) * M_PI;
            eph.m0        = std::ldexp(static_cast<double>(kep.keplerM0), -31) * M_PI;
            eph.e         = std::ldexp(static_cast<double>(kep.keplerE), -33);
            eph.omega     = std::ldexp(static_cast<double>(kep.keplerW), -31) * M_PI;
            eph.cuc       = std::ldexp(static_cast<double>(kep.keplerCuc), -29);
            eph.cus       = std::ldexp(static_cast<double>(kep.keplerCus), -29);
            eph.crc       = static_cast<double>(kep.keplerCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.keplerCrs) * 0.03125;
            eph.cic       = std::ldexp(static_cast<double>(kep.keplerCic), -29);
            eph.cis       = std::ldexp(static_cast<double>(kep.keplerCis), -29);
            eph.i0        = std::ldexp(static_cast<double>(kep.keplerI0), -31) * M_PI;
            eph.idot      = std::ldexp(static_cast<double>(kep.keplerIDot), -43) * M_PI;
            eph.omega0    = std::ldexp(static_cast<double>(kep.keplerOmega0), -31) * M_PI;
            eph.omega_dot = std::ldexp(static_cast<double>(kep.keplerOmegaDot), -43) * M_PI;
        }

        eph.af0         = std::ldexp(static_cast<double>(clock.navaf0), -31);
        eph.af1         = std::ldexp(static_cast<double>(clock.navaf1), -43);
        eph.af2         = std::ldexp(static_cast<double>(clock.navaf2), -55);
        eph.toc         = static_cast<double>(clock.navToc) * 16;
        eph.tgd         = std::ldexp(static_cast<double>(clock.navTgd), -31);
        eph.week_number = nearest_week(reference, eph.toe);

        auto gps_time = ts::Gps::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("GPS ephemeris %s: PRN=%u lpp_iod=%u toe=%s now=%s", satellite_id.name(), eph.prn,
               eph.lpp_iod, ts::Tai(gps_time).rtklib_time_string().c_str(),
               ts::Tai::now().rtklib_time_string().c_str());
        system.push(std::move(eph), mTag);
    }
}

void Lpp2Eph::process_gal_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto reference = ts::Gst(mReferenceTime);

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];

        // Only support I/NAV (E1/E5a) for now - stanModelID=0
        if (sat.gnss_ClockModel.present == GNSS_ClockModel_PR_standardClockModelList) {
            auto& list       = sat.gnss_ClockModel.choice.standardClockModelList;
            bool  found_inav = false;
            for (int j = 0; j < list.list.count; j++) {
                if (list.list.array[j]->stanModelID && *list.list.array[j]->stanModelID == 0) {
                    found_inav = true;
                    break;
                }
            }
            if (!found_inav) {
                VERBOSEF("Galileo satellite without I/NAV (stanModelID=0), skipping");
                continue;
            }
        } else if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_nav_ClockModel) {
            continue;
        }

        bool is_nav_keplerian = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_nav_KeplerianSet;
        bool is_keplerian     = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_keplerianSet;
        if (!is_nav_keplerian && !is_keplerian) continue;

        auto satellite_id =
            SatelliteId::from_lpp(SatelliteId::Gnss::GALILEO, sat.svID.satellite_id);
        if (!satellite_id.is_valid()) continue;

        auto prn = satellite_id.prn();
        if (!prn.valid) continue;

        uint16_t lpp_iod = static_cast<uint16_t>(helper::BitString::from(&sat.iod)->as_int64());
        helper::BitStringReader iod_reader(&sat.iod);
        uint16_t                iod = static_cast<uint16_t>(iod_reader.integer<uint16_t>(1, 10));

        ephemeris::GalEphemeris eph{};
        eph.prn     = prn.value;
        eph.iod_nav = iod;
        eph.lpp_iod = lpp_iod;

        if (is_nav_keplerian) {
            auto& kep     = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe       = static_cast<double>(kep.navToe) * 16;
            eph.a         = std::pow(std::ldexp(static_cast<double>(kep.navAPowerHalf), -19), 2);
            eph.delta_n   = std::ldexp(static_cast<double>(kep.navDeltaN), -43) * M_PI;
            eph.m0        = std::ldexp(static_cast<double>(kep.navM0), -31) * M_PI;
            eph.e         = std::ldexp(static_cast<double>(kep.navE), -33);
            eph.omega     = std::ldexp(static_cast<double>(kep.navOmega), -31) * M_PI;
            eph.cuc       = std::ldexp(static_cast<double>(kep.navCuc), -29);
            eph.cus       = std::ldexp(static_cast<double>(kep.navCus), -29);
            eph.crc       = static_cast<double>(kep.navCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.navCrs) * 0.03125;
            eph.cic       = std::ldexp(static_cast<double>(kep.navCic), -29);
            eph.cis       = std::ldexp(static_cast<double>(kep.navCis), -29);
            eph.i0        = std::ldexp(static_cast<double>(kep.navI0), -31) * M_PI;
            eph.idot      = std::ldexp(static_cast<double>(kep.navIDot), -43) * M_PI;
            eph.omega0    = std::ldexp(static_cast<double>(kep.navOmegaA0), -31) * M_PI;
            eph.omega_dot = std::ldexp(static_cast<double>(kep.navOmegaADot), -43) * M_PI;
        } else {
            auto& kep     = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe       = static_cast<double>(kep.keplerToe) * 60;
            eph.a         = std::pow(std::ldexp(static_cast<double>(kep.keplerAPowerHalf), -19), 2);
            eph.delta_n   = std::ldexp(static_cast<double>(kep.keplerDeltaN), -43) * M_PI;
            eph.m0        = std::ldexp(static_cast<double>(kep.keplerM0), -31) * M_PI;
            eph.e         = std::ldexp(static_cast<double>(kep.keplerE), -33);
            eph.omega     = std::ldexp(static_cast<double>(kep.keplerW), -31) * M_PI;
            eph.cuc       = std::ldexp(static_cast<double>(kep.keplerCuc), -29);
            eph.cus       = std::ldexp(static_cast<double>(kep.keplerCus), -29);
            eph.crc       = static_cast<double>(kep.keplerCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.keplerCrs) * 0.03125;
            eph.cic       = std::ldexp(static_cast<double>(kep.keplerCic), -29);
            eph.cis       = std::ldexp(static_cast<double>(kep.keplerCis), -29);
            eph.i0        = std::ldexp(static_cast<double>(kep.keplerI0), -31) * M_PI;
            eph.idot      = std::ldexp(static_cast<double>(kep.keplerIDot), -43) * M_PI;
            eph.omega0    = std::ldexp(static_cast<double>(kep.keplerOmega0), -31) * M_PI;
            eph.omega_dot = std::ldexp(static_cast<double>(kep.keplerOmegaDot), -43) * M_PI;
        }

        // Extract clock parameters based on model type
        if (sat.gnss_ClockModel.present == GNSS_ClockModel_PR_standardClockModelList) {
            auto&                        list = sat.gnss_ClockModel.choice.standardClockModelList;
            StandardClockModelElement_t* inav_clock = nullptr;
            for (int j = 0; j < list.list.count; j++) {
                if (list.list.array[j]->stanModelID && *list.list.array[j]->stanModelID == 0) {
                    inav_clock = list.list.array[j];
                    break;
                }
            }
            if (!inav_clock) continue;  // Should not happen due to earlier check

            eph.af0 = std::ldexp(static_cast<double>(inav_clock->stanClockAF0), -34);
            eph.af1 = std::ldexp(static_cast<double>(inav_clock->stanClockAF1), -46);
            eph.af2 = std::ldexp(static_cast<double>(inav_clock->stanClockAF2), -59);
            eph.toc = static_cast<double>(inav_clock->stanClockToc) * 60;
        } else {
            auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;
            eph.af0     = std::ldexp(static_cast<double>(clock.navaf0), -31);
            eph.af1     = std::ldexp(static_cast<double>(clock.navaf1), -43);
            eph.af2     = std::ldexp(static_cast<double>(clock.navaf2), -55);
            eph.toc     = static_cast<double>(clock.navToc) * 16;
        }
        eph.week_number = nearest_week(reference, eph.toe);

        auto gal_time = ts::Gst::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("Galileo ephemeris: PRN=%u lpp_iod=%u toe=%s", eph.prn, eph.lpp_iod,
               ts::Tai(gal_time).rtklib_time_string().c_str());
        system.push(std::move(eph), mTag);
    }
}

void Lpp2Eph::process_bds_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto reference = ts::Bdt(mReferenceTime);

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_bds_ClockModel_r12) continue;
        if (sat.gnss_OrbitModel.present != GNSS_OrbitModel_PR_bds_KeplerianSet_r12) continue;

        auto satellite_id = SatelliteId::from_lpp(SatelliteId::Gnss::BEIDOU, sat.svID.satellite_id);
        if (!satellite_id.is_valid()) continue;

        auto prn = satellite_id.prn();
        if (!prn.valid) continue;

        auto& kep   = sat.gnss_OrbitModel.choice.bds_KeplerianSet_r12;
        auto& clock = sat.gnss_ClockModel.choice.bds_ClockModel_r12;

        uint16_t lpp_iod = static_cast<uint16_t>(helper::BitString::from(&sat.iod)->as_int64());
        helper::BitStringReader iod_reader(&sat.iod);
        uint16_t iod_11bit = static_cast<uint16_t>(iod_reader.integer<uint16_t>(0, 11));
        uint8_t  iod       = static_cast<uint8_t>(iod_11bit >> 6);

        ephemeris::BdsEphemeris eph{};
        eph.prn         = prn.value;
        eph.iode        = iod;
        eph.iodc        = iod;
        eph.lpp_iod     = lpp_iod;
        eph.toe         = static_cast<double>(kep.bdsToe_r12) * 8;
        eph.a           = std::pow(std::ldexp(static_cast<double>(kep.bdsAPowerHalf_r12), -19), 2);
        eph.delta_n     = std::ldexp(static_cast<double>(kep.bdsDeltaN_r12), -43) * M_PI;
        eph.m0          = std::ldexp(static_cast<double>(kep.bdsM0_r12), -31) * M_PI;
        eph.e           = std::ldexp(static_cast<double>(kep.bdsE_r12), -33);
        eph.omega       = std::ldexp(static_cast<double>(kep.bdsW_r12), -31) * M_PI;
        eph.cuc         = std::ldexp(static_cast<double>(kep.bdsCuc_r12), -31);
        eph.cus         = std::ldexp(static_cast<double>(kep.bdsCus_r12), -31);
        eph.crc         = static_cast<double>(kep.bdsCrc_r12) * 0.015625;
        eph.crs         = static_cast<double>(kep.bdsCrs_r12) * 0.015625;
        eph.cic         = std::ldexp(static_cast<double>(kep.bdsCic_r12), -31);
        eph.cis         = std::ldexp(static_cast<double>(kep.bdsCis_r12), -31);
        eph.i0          = std::ldexp(static_cast<double>(kep.bdsI0_r12), -31) * M_PI;
        eph.idot        = std::ldexp(static_cast<double>(kep.bdsIDot_r12), -43) * M_PI;
        eph.omega0      = std::ldexp(static_cast<double>(kep.bdsOmega0_r12), -31) * M_PI;
        eph.omega_dot   = std::ldexp(static_cast<double>(kep.bdsOmegaDot_r12), -43) * M_PI;
        eph.af0         = std::ldexp(static_cast<double>(clock.bdsA0_r12), -33);
        eph.af1         = std::ldexp(static_cast<double>(clock.bdsA1_r12), -50);
        eph.af2         = std::ldexp(static_cast<double>(clock.bdsA2_r12), -66);
        eph.toc         = static_cast<double>(clock.bdsToc_r12) * 8;
        eph.week_number = nearest_week(reference, eph.toe);

        auto bds_time = ts::Bdt::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("BeiDou ephemeris: PRN=%u lpp_iod=%u toe=%s", eph.prn, eph.lpp_iod,
               ts::Tai(bds_time).rtklib_time_string().c_str());
        system.push(std::move(eph), mTag);
    }
}

void Lpp2Eph::process_qzs_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto reference = ts::Gps(mReferenceTime);

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_nav_ClockModel) continue;
        if (sat.gnss_OrbitModel.present != GNSS_OrbitModel_PR_nav_KeplerianSet) continue;

        auto satellite_id = SatelliteId::from_lpp(SatelliteId::Gnss::QZSS, sat.svID.satellite_id);
        if (!satellite_id.is_valid()) continue;

        auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;
        auto& kep   = sat.gnss_OrbitModel.choice.nav_KeplerianSet;

        helper::BitStringReader iod_reader(&sat.iod);
        helper::BitStringReader health_reader(&sat.svHealth);

        ephemeris::QzsEphemeris eph{};
        eph.prn  = static_cast<uint8_t>(193 + sat.svID.satellite_id);
        eph.iodc = static_cast<uint16_t>(iod_reader.integer<uint16_t>(1, 10));
        eph.iode = static_cast<uint8_t>(eph.iodc & 0xFF);
        // [3GPP TS 37.355]: for QZS-L1 the iod contains the IODC
        eph.lpp_iod           = eph.iodc;
        eph.sv_health         = static_cast<uint8_t>(health_reader.integer<uint8_t>(0, 6));
        eph.ura_index         = static_cast<uint8_t>(kep.navURA);
        eph.fit_interval_flag = kep.navFitFlag != 0;

        eph.toe       = static_cast<double>(kep.navToe) * 16;
        eph.a         = std::pow(std::ldexp(static_cast<double>(kep.navAPowerHalf), -19), 2);
        eph.delta_n   = std::ldexp(static_cast<double>(kep.navDeltaN), -43) * M_PI;
        eph.m0        = std::ldexp(static_cast<double>(kep.navM0), -31) * M_PI;
        eph.e         = std::ldexp(static_cast<double>(kep.navE), -33);
        eph.omega     = std::ldexp(static_cast<double>(kep.navOmega), -31) * M_PI;
        eph.cuc       = std::ldexp(static_cast<double>(kep.navCuc), -29);
        eph.cus       = std::ldexp(static_cast<double>(kep.navCus), -29);
        eph.crc       = std::ldexp(static_cast<double>(kep.navCrc), -5);
        eph.crs       = std::ldexp(static_cast<double>(kep.navCrs), -5);
        eph.cic       = std::ldexp(static_cast<double>(kep.navCic), -29);
        eph.cis       = std::ldexp(static_cast<double>(kep.navCis), -29);
        eph.i0        = std::ldexp(static_cast<double>(kep.navI0), -31) * M_PI;
        eph.idot      = std::ldexp(static_cast<double>(kep.navIDot), -43) * M_PI;
        eph.omega0    = std::ldexp(static_cast<double>(kep.navOmegaA0), -31) * M_PI;
        eph.omega_dot = std::ldexp(static_cast<double>(kep.navOmegaADot), -43) * M_PI;

        eph.af0 = std::ldexp(static_cast<double>(clock.navaf0), -31);
        eph.af1 = std::ldexp(static_cast<double>(clock.navaf1), -43);
        eph.af2 = std::ldexp(static_cast<double>(clock.navaf2), -55);
        eph.toc = static_cast<double>(clock.navToc) * 16;
        eph.tgd = std::ldexp(static_cast<double>(clock.navTgd), -31);

        eph.week_number = nearest_week(reference, eph.toe);
        DEBUGF("QZSS ephemeris %s: PRN=%u lpp_iod=%u", satellite_id.name(), eph.prn, eph.lpp_iod);
        system.push(std::move(eph), mTag);
    }
}

void Lpp2Eph::process_glo_auxiliary_information(GNSS_AuxiliaryInformation const& aux) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (aux.present != GNSS_AuxiliaryInformation_PR_gnss_ID_GLONASS) return;

    auto& list = aux.choice.gnss_ID_GLONASS.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element || !element->channelNumber) continue;
        mGloChannels[element->svID.satellite_id] = static_cast<int8_t>(*element->channelNumber);
    }
}

void Lpp2Eph::process_glo_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto now = ts::Glo(mReferenceTime);

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_glonass_ClockModel) continue;
        if (sat.gnss_OrbitModel.present != GNSS_OrbitModel_PR_glonass_ECEF) continue;

        auto satellite_id =
            SatelliteId::from_lpp(SatelliteId::Gnss::GLONASS, sat.svID.satellite_id);
        if (!satellite_id.is_valid()) continue;

        auto channel = mGloChannels.find(sat.svID.satellite_id);
        if (channel == mGloChannels.end()) {
            WARNF("GLONASS ephemeris %s without frequency channel (GNSS-AuxiliaryInformation)",
                  satellite_id.name());
            continue;
        }

        auto& clock = sat.gnss_ClockModel.choice.glonass_ClockModel;
        auto& orbit = sat.gnss_OrbitModel.choice.glonass_ECEF;

        // [3GPP TS 37.355]: for GLONASS the iod contains tb (15 minutes)
        helper::BitStringReader iod_reader(&sat.iod);
        helper::BitStringReader health_reader(&sat.svHealth);
        auto                    tb  = static_cast<uint16_t>(iod_reader.integer<uint16_t>(4, 7));
        auto                    tod = static_cast<double>(tb) * 900.0;

        // the day of tb is not included, use the day closest to the reference time
        auto reference_time = ts::Glo::from_absolute_day_tod(now.days(), tod);
        auto dt             = now - reference_time;
        if (dt > 43200.0) reference_time = reference_time + 86400.0;
        if (dt < -43200.0) reference_time = reference_time - 86400.0;

        ephemeris::GloEphemeris eph{};
        eph.slot_number      = static_cast<uint8_t>(sat.svID.satellite_id + 1);
        eph.frequency_number = channel->second;
        eph.reference_time   = reference_time;
        eph.position         = Float3{std::ldexp(static_cast<double>(orbit.gloX), -11),
                              std::ldexp(static_cast<double>(orbit.gloY), -11),
                              std::ldexp(static_cast<double>(orbit.gloZ), -11)};
        eph.velocity         = Float3{std::ldexp(static_cast<double>(orbit.gloXdot), -20),
                              std::ldexp(static_cast<double>(orbit.gloYdot), -20),
                              std::ldexp(static_cast<double>(orbit.gloZdot), -20)};
        eph.acceleration     = Float3{std::ldexp(static_cast<double>(orbit.gloXdotdot), -30),
                                  std::ldexp(static_cast<double>(orbit.gloYdotdot), -30),
                                  std::ldexp(static_cast<double>(orbit.gloZdotdot), -30)};
        eph.tau_n            = std::ldexp(static_cast<double>(clock.gloTau), -30);
        eph.gamma_n          = std::ldexp(static_cast<double>(clock.gloGamma), -40);
        eph.health           = static_cast<uint8_t>(health_reader.integer<uint8_t>(0, 1));
        eph.age              = static_cast<uint8_t>(orbit.gloEn);
        eph.m                = static_cast<uint8_t>(orbit.gloM);
        eph.lpp_iod          = tb;

        DEBUGF("GLONASS ephemeris %s: tb=%u channel=%d", satellite_id.name(), tb, channel->second);
        system.push(std::move(eph), mTag);
    }
}

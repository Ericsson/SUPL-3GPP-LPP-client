#include "lpp2eph.hpp"

#include <asn.1/bit_string.hpp>
#include <external_warnings.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <time/bdt.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/tai.hpp>

EXTERNAL_WARNINGS_PUSH
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-ID.h>
#include <GNSS-NavModelSatelliteElement.h>
#include <GNSS-NavigationModel.h>
#include <NavModelKeplerianSet.h>
#include <NavModelNAV-KeplerianSet.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <StandardClockModelElement.h>
EXTERNAL_WARNINGS_POP

LOGLET_MODULE2(p, lpp2eph);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, lpp2eph)

void Lpp2Eph::inspect(streamline::System& system, DataType const& message, uint64_t) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!message) return;

    auto pad = lpp::get_provide_assistance_data(message);
    if (!pad) return;
    if (!pad->a_gnss_ProvideAssistanceData) return;

    auto& agnss = *pad->a_gnss_ProvideAssistanceData;
    if (!agnss.gnss_GenericAssistData) return;

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
        }
    }
}

void Lpp2Eph::process_gps_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Gps::now().week();

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
        eph.lpp_iod     = iodc;
        eph.week_number = static_cast<uint16_t>(current_week);

        if (is_nav_keplerian) {
            auto& kep = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe   = static_cast<double>(kep.navToe) * 16;
            eph.a     = static_cast<double>(kep.navAPowerHalf) *
                    static_cast<double>(kep.navAPowerHalf) * (1.0 / 524288.0);
            eph.delta_n   = static_cast<double>(kep.navDeltaN) * 1e-43 * M_PI;
            eph.m0        = static_cast<double>(kep.navM0) * 1e-31 * M_PI;
            eph.e         = static_cast<double>(kep.navE) * 1e-33;
            eph.omega     = static_cast<double>(kep.navOmega) * 1e-31 * M_PI;
            eph.cuc       = static_cast<double>(kep.navCuc) * 1e-29;
            eph.cus       = static_cast<double>(kep.navCus) * 1e-29;
            eph.crc       = static_cast<double>(kep.navCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.navCrs) * 0.03125;
            eph.cic       = static_cast<double>(kep.navCic) * 1e-29;
            eph.cis       = static_cast<double>(kep.navCis) * 1e-29;
            eph.i0        = static_cast<double>(kep.navI0) * 1e-31 * M_PI;
            eph.idot      = static_cast<double>(kep.navIDot) * 1e-43 * M_PI;
            eph.omega0    = static_cast<double>(kep.navOmegaA0) * 1e-31 * M_PI;
            eph.omega_dot = static_cast<double>(kep.navOmegaADot) * 1e-43 * M_PI;
        } else {
            auto& kep = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe   = static_cast<double>(kep.keplerToe) * 60;
            eph.a     = static_cast<double>(kep.keplerAPowerHalf) *
                    static_cast<double>(kep.keplerAPowerHalf) * (1.0 / 524288.0);
            eph.delta_n   = static_cast<double>(kep.keplerDeltaN) * 1e-43 * M_PI;
            eph.m0        = static_cast<double>(kep.keplerM0) * 1e-31 * M_PI;
            eph.e         = static_cast<double>(kep.keplerE) * 1e-33;
            eph.omega     = static_cast<double>(kep.keplerW) * 1e-31 * M_PI;
            eph.cuc       = static_cast<double>(kep.keplerCuc) * 1e-29;
            eph.cus       = static_cast<double>(kep.keplerCus) * 1e-29;
            eph.crc       = static_cast<double>(kep.keplerCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.keplerCrs) * 0.03125;
            eph.cic       = static_cast<double>(kep.keplerCic) * 1e-29;
            eph.cis       = static_cast<double>(kep.keplerCis) * 1e-29;
            eph.i0        = static_cast<double>(kep.keplerI0) * 1e-31 * M_PI;
            eph.idot      = static_cast<double>(kep.keplerIDot) * 1e-43 * M_PI;
            eph.omega0    = static_cast<double>(kep.keplerOmega0) * 1e-31 * M_PI;
            eph.omega_dot = static_cast<double>(kep.keplerOmegaDot) * 1e-43 * M_PI;
        }

        eph.af0 = static_cast<double>(clock.navaf0) * 1e-31;
        eph.af1 = static_cast<double>(clock.navaf1) * 1e-43;
        eph.af2 = static_cast<double>(clock.navaf2) * 1e-55;
        eph.toc = static_cast<double>(clock.navToc) * 16;
        eph.tgd = static_cast<double>(clock.navTgd) * 1e-31;

        auto gps_time = ts::Gps::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("GPS ephemeris %s: PRN=%u lpp_iod=%u toe=%s now=%s", satellite_id.name(), eph.prn,
               eph.lpp_iod, ts::Tai(gps_time).rtklib_time_string().c_str(),
               ts::Tai::now().rtklib_time_string().c_str());
        system.push(std::move(eph));
    }
}

void Lpp2Eph::process_gal_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Gst::now().week();

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
        eph.prn         = prn.value;
        eph.iod_nav     = iod;
        eph.lpp_iod     = lpp_iod;
        eph.week_number = static_cast<uint16_t>(current_week);

        if (is_nav_keplerian) {
            auto& kep = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe   = static_cast<double>(kep.navToe) * 60;
            eph.a     = static_cast<double>(kep.navAPowerHalf) *
                    static_cast<double>(kep.navAPowerHalf) * (1.0 / 524288.0);
            eph.delta_n   = static_cast<double>(kep.navDeltaN) * 1e-43 * M_PI;
            eph.m0        = static_cast<double>(kep.navM0) * 1e-31 * M_PI;
            eph.e         = static_cast<double>(kep.navE) * 1e-33;
            eph.omega     = static_cast<double>(kep.navOmega) * 1e-31 * M_PI;
            eph.cuc       = static_cast<double>(kep.navCuc) * 1e-29;
            eph.cus       = static_cast<double>(kep.navCus) * 1e-29;
            eph.crc       = static_cast<double>(kep.navCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.navCrs) * 0.03125;
            eph.cic       = static_cast<double>(kep.navCic) * 1e-29;
            eph.cis       = static_cast<double>(kep.navCis) * 1e-29;
            eph.i0        = static_cast<double>(kep.navI0) * 1e-31 * M_PI;
            eph.idot      = static_cast<double>(kep.navIDot) * 1e-43 * M_PI;
            eph.omega0    = static_cast<double>(kep.navOmegaA0) * 1e-31 * M_PI;
            eph.omega_dot = static_cast<double>(kep.navOmegaADot) * 1e-43 * M_PI;
        } else {
            auto& kep = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe   = static_cast<double>(kep.keplerToe) * 60;
            eph.a     = static_cast<double>(kep.keplerAPowerHalf) *
                    static_cast<double>(kep.keplerAPowerHalf) * (1.0 / 524288.0);
            eph.delta_n   = static_cast<double>(kep.keplerDeltaN) * 1e-43 * M_PI;
            eph.m0        = static_cast<double>(kep.keplerM0) * 1e-31 * M_PI;
            eph.e         = static_cast<double>(kep.keplerE) * 1e-33;
            eph.omega     = static_cast<double>(kep.keplerW) * 1e-31 * M_PI;
            eph.cuc       = static_cast<double>(kep.keplerCuc) * 1e-29;
            eph.cus       = static_cast<double>(kep.keplerCus) * 1e-29;
            eph.crc       = static_cast<double>(kep.keplerCrc) * 0.03125;
            eph.crs       = static_cast<double>(kep.keplerCrs) * 0.03125;
            eph.cic       = static_cast<double>(kep.keplerCic) * 1e-29;
            eph.cis       = static_cast<double>(kep.keplerCis) * 1e-29;
            eph.i0        = static_cast<double>(kep.keplerI0) * 1e-31 * M_PI;
            eph.idot      = static_cast<double>(kep.keplerIDot) * 1e-43 * M_PI;
            eph.omega0    = static_cast<double>(kep.keplerOmega0) * 1e-31 * M_PI;
            eph.omega_dot = static_cast<double>(kep.keplerOmegaDot) * 1e-43 * M_PI;
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

            eph.af0 = static_cast<double>(inav_clock->stanClockAF0) * 1e-31;
            eph.af1 = static_cast<double>(inav_clock->stanClockAF1) * 1e-43;
            eph.af2 = static_cast<double>(inav_clock->stanClockAF2) * 1e-55;
            eph.toc = static_cast<double>(inav_clock->stanClockToc) * 60;
        } else {
            auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;
            eph.af0     = static_cast<double>(clock.navaf0) * 1e-31;
            eph.af1     = static_cast<double>(clock.navaf1) * 1e-43;
            eph.af2     = static_cast<double>(clock.navaf2) * 1e-55;
            eph.toc     = static_cast<double>(clock.navToc) * 60;
        }

        auto gal_time = ts::Gst::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("Galileo ephemeris: PRN=%u lpp_iod=%u toe=%s", eph.prn, eph.lpp_iod,
               ts::Tai(gal_time).rtklib_time_string().c_str());
        system.push(std::move(eph));
    }
}

void Lpp2Eph::process_bds_navigation_model(streamline::System&         system,
                                           GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Bdt::now().week();

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
        eph.prn     = prn.value;
        eph.iode    = iod;
        eph.iodc    = iod;
        eph.lpp_iod = lpp_iod;
        eph.toe     = static_cast<double>(kep.bdsToe_r12) * 8;
        eph.a       = static_cast<double>(kep.bdsAPowerHalf_r12) *
                static_cast<double>(kep.bdsAPowerHalf_r12) * (1.0 / 524288.0);
        eph.delta_n     = static_cast<double>(kep.bdsDeltaN_r12) * 1e-43 * M_PI;
        eph.m0          = static_cast<double>(kep.bdsM0_r12) * 1e-31 * M_PI;
        eph.e           = static_cast<double>(kep.bdsE_r12) * 1e-33;
        eph.omega       = static_cast<double>(kep.bdsW_r12) * 1e-31 * M_PI;
        eph.cuc         = static_cast<double>(kep.bdsCuc_r12) * 1e-31;
        eph.cus         = static_cast<double>(kep.bdsCus_r12) * 1e-31;
        eph.crc         = static_cast<double>(kep.bdsCrc_r12) * 0.015625;
        eph.crs         = static_cast<double>(kep.bdsCrs_r12) * 0.015625;
        eph.cic         = static_cast<double>(kep.bdsCic_r12) * 1e-31;
        eph.cis         = static_cast<double>(kep.bdsCis_r12) * 1e-31;
        eph.i0          = static_cast<double>(kep.bdsI0_r12) * 1e-31 * M_PI;
        eph.idot        = static_cast<double>(kep.bdsIDot_r12) * 1e-43 * M_PI;
        eph.omega0      = static_cast<double>(kep.bdsOmega0_r12) * 1e-31 * M_PI;
        eph.omega_dot   = static_cast<double>(kep.bdsOmegaDot_r12) * 1e-43 * M_PI;
        eph.af0         = static_cast<double>(clock.bdsA0_r12) * 1e-33;
        eph.af1         = static_cast<double>(clock.bdsA1_r12) * 1e-50;
        eph.af2         = static_cast<double>(clock.bdsA2_r12) * 1e-66;
        eph.toc         = static_cast<double>(clock.bdsToc_r12) * 8;
        eph.week_number = static_cast<uint16_t>(current_week);

        auto bds_time = ts::Bdt::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
        DEBUGF("BeiDou ephemeris: PRN=%u lpp_iod=%u toe=%s", eph.prn, eph.lpp_iod,
               ts::Tai(bds_time).rtklib_time_string().c_str());
        system.push(std::move(eph));
    }
}

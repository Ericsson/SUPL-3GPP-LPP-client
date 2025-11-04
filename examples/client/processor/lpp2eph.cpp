#include "lpp2eph.hpp"

#include <asn.1/bit_string.hpp>
#include <loglet/loglet.hpp>
#include <lpp/assistance_data.hpp>
#include <time/gps.hpp>

#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-ID.h>
#include <GNSS-NavigationModel.h>
#include <GNSS-NavModelSatelliteElement.h>
#include <NavModelKeplerianSet.h>
#include <NavModelNAV-KeplerianSet.h>
#include <ProvideAssistanceData-r9-IEs.h>

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

void Lpp2Eph::process_gps_navigation_model(streamline::System&           system,
                                            GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Gps::now().week();

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_nav_ClockModel) continue;

        bool is_nav_keplerian = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_nav_KeplerianSet;
        bool is_keplerian     = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_keplerianSet;
        if (!is_nav_keplerian && !is_keplerian) continue;

        auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;

        // GPS IOD: For L1/CA, bit 1 is '0' followed by 10-bit IODC
        // Extract 8 LSBs as IODE (bits 3-10 of the 11-bit field)
        helper::BitStringReader iod_reader(&sat.iod);
        uint8_t  iod  = static_cast<uint8_t>(iod_reader.integer<uint16_t>(3, 8));
        uint16_t iodc = static_cast<uint16_t>(iod_reader.integer<uint16_t>(1, 10));

        ephemeris::GpsEphemeris eph{};
        eph.prn         = sat.svID.satellite_id;
        eph.iode        = iod;
        eph.iodc        = iodc;
        eph.lpp_iod     = iod;
        eph.week_number = current_week;

        if (is_nav_keplerian) {
            auto& kep   = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe     = kep.navToe * 16;
            eph.a       = kep.navAPowerHalf * kep.navAPowerHalf * 0.0625;
            eph.delta_n = kep.navDeltaN * 1e-43 * M_PI;
            eph.m0      = kep.navM0 * 1e-31 * M_PI;
            eph.e       = kep.navE * 1e-33;
            eph.omega   = kep.navOmega * 1e-31 * M_PI;
            eph.cuc     = kep.navCuc * 1e-29;
            eph.cus     = kep.navCus * 1e-29;
            eph.crc     = kep.navCrc * 0.0625;
            eph.crs     = kep.navCrs * 0.0625;
            eph.cic     = kep.navCic * 1e-29;
            eph.cis     = kep.navCis * 1e-29;
            eph.i0      = kep.navI0 * 1e-31 * M_PI;
            eph.idot    = kep.navIDot * 1e-43 * M_PI;
            eph.omega0  = kep.navOmegaA0 * 1e-31 * M_PI;
            eph.omega_dot = kep.navOmegaADot * 1e-43 * M_PI;
        } else {
            auto& kep   = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe     = kep.keplerToe * 16;
            eph.a       = kep.keplerAPowerHalf * kep.keplerAPowerHalf * 0.0625;
            eph.delta_n = kep.keplerDeltaN * 1e-43 * M_PI;
            eph.m0      = kep.keplerM0 * 1e-31 * M_PI;
            eph.e       = kep.keplerE * 1e-33;
            eph.omega   = kep.keplerW * 1e-31 * M_PI;
            eph.cuc     = kep.keplerCuc * 1e-29;
            eph.cus     = kep.keplerCus * 1e-29;
            eph.crc     = kep.keplerCrc * 0.0625;
            eph.crs     = kep.keplerCrs * 0.0625;
            eph.cic     = kep.keplerCic * 1e-29;
            eph.cis     = kep.keplerCis * 1e-29;
            eph.i0      = kep.keplerI0 * 1e-31 * M_PI;
            eph.idot    = kep.keplerIDot * 1e-43 * M_PI;
            eph.omega0  = kep.keplerOmega0 * 1e-31 * M_PI;
            eph.omega_dot = kep.keplerOmegaDot * 1e-43 * M_PI;
        }

        eph.af0 = clock.navaf0 * 1e-31;
        eph.af1 = clock.navaf1 * 1e-43;
        eph.af2 = clock.navaf2 * 1e-55;
        eph.toc = clock.navToc * 16;
        eph.tgd = clock.navTgd * 1e-31;

        system.push(std::move(eph));
        DEBUGF("GPS ephemeris: PRN=%u IOD=%u IODC=%u", eph.prn, eph.iode, eph.iodc);
    }
}

void Lpp2Eph::process_gal_navigation_model(streamline::System&           system,
                                            GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Gst::now().week();

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_nav_ClockModel) continue;

        bool is_nav_keplerian = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_nav_KeplerianSet;
        bool is_keplerian     = sat.gnss_OrbitModel.present == GNSS_OrbitModel_PR_keplerianSet;
        if (!is_nav_keplerian && !is_keplerian) continue;

        auto& clock = sat.gnss_ClockModel.choice.nav_ClockModel;

        // Galileo IOD: Bit 1 is '0' followed by 10-bit IODnav
        helper::BitStringReader iod_reader(&sat.iod);
        uint16_t iod = static_cast<uint16_t>(iod_reader.integer<uint16_t>(1, 10));

        ephemeris::GalEphemeris eph{};
        eph.prn         = sat.svID.satellite_id;
        eph.iod_nav     = iod;
        eph.lpp_iod     = iod;
        eph.week_number = current_week;

        if (is_nav_keplerian) {
            auto& kep   = sat.gnss_OrbitModel.choice.nav_KeplerianSet;
            eph.toe     = kep.navToe * 60;
            eph.a       = kep.navAPowerHalf * kep.navAPowerHalf * 0.0625;
            eph.delta_n = kep.navDeltaN * 1e-43 * M_PI;
            eph.m0      = kep.navM0 * 1e-31 * M_PI;
            eph.e       = kep.navE * 1e-33;
            eph.omega   = kep.navOmega * 1e-31 * M_PI;
            eph.cuc     = kep.navCuc * 1e-29;
            eph.cus     = kep.navCus * 1e-29;
            eph.crc     = kep.navCrc * 0.0625;
            eph.crs     = kep.navCrs * 0.0625;
            eph.cic     = kep.navCic * 1e-29;
            eph.cis     = kep.navCis * 1e-29;
            eph.i0      = kep.navI0 * 1e-31 * M_PI;
            eph.idot    = kep.navIDot * 1e-43 * M_PI;
            eph.omega0  = kep.navOmegaA0 * 1e-31 * M_PI;
            eph.omega_dot = kep.navOmegaADot * 1e-43 * M_PI;
        } else {
            auto& kep   = sat.gnss_OrbitModel.choice.keplerianSet;
            eph.toe     = kep.keplerToe * 60;
            eph.a       = kep.keplerAPowerHalf * kep.keplerAPowerHalf * 0.0625;
            eph.delta_n = kep.keplerDeltaN * 1e-43 * M_PI;
            eph.m0      = kep.keplerM0 * 1e-31 * M_PI;
            eph.e       = kep.keplerE * 1e-33;
            eph.omega   = kep.keplerW * 1e-31 * M_PI;
            eph.cuc     = kep.keplerCuc * 1e-29;
            eph.cus     = kep.keplerCus * 1e-29;
            eph.crc     = kep.keplerCrc * 0.0625;
            eph.crs     = kep.keplerCrs * 0.0625;
            eph.cic     = kep.keplerCic * 1e-29;
            eph.cis     = kep.keplerCis * 1e-29;
            eph.i0      = kep.keplerI0 * 1e-31 * M_PI;
            eph.idot    = kep.keplerIDot * 1e-43 * M_PI;
            eph.omega0  = kep.keplerOmega0 * 1e-31 * M_PI;
            eph.omega_dot = kep.keplerOmegaDot * 1e-43 * M_PI;
        }

        eph.af0 = clock.navaf0 * 1e-31;
        eph.af1 = clock.navaf1 * 1e-43;
        eph.af2 = clock.navaf2 * 1e-55;
        eph.toc = clock.navToc * 60;

        system.push(std::move(eph));
        DEBUGF("Galileo ephemeris: PRN=%u IOD=%u", eph.prn, eph.iod_nav);
    }
}

void Lpp2Eph::process_bds_navigation_model(streamline::System&           system,
                                            GNSS_NavigationModel const& nav_model) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto current_week = ts::Bdt::now().week();

    for (int i = 0; i < nav_model.gnss_SatelliteList.list.count; i++) {
        auto& sat = *nav_model.gnss_SatelliteList.list.array[i];
        if (sat.gnss_ClockModel.present != GNSS_ClockModel_PR_bds_ClockModel_r12) continue;
        if (sat.gnss_OrbitModel.present != GNSS_OrbitModel_PR_bds_KeplerianSet_r12) continue;

        auto& kep   = sat.gnss_OrbitModel.choice.bds_KeplerianSet_r12;
        auto& clock = sat.gnss_ClockModel.choice.bds_ClockModel_r12;

        // BDS B1I/B3I IOD: 11 MSB bits of toe (scale factor 512)
        helper::BitStringReader iod_reader(&sat.iod);
        uint16_t iod_11bit = static_cast<uint16_t>(iod_reader.integer<uint16_t>(0, 11));
        uint8_t  iod       = static_cast<uint8_t>(iod_11bit >> 6);

        ephemeris::BdsEphemeris eph{};
        eph.prn         = sat.svID.satellite_id;
        eph.iode        = iod;
        eph.iodc        = iod;
        eph.lpp_iod     = iod;
        eph.toe         = kep.bdsToe_r12 * 8;
        eph.a           = kep.bdsAPowerHalf_r12 * kep.bdsAPowerHalf_r12 * 0.0625;
        eph.delta_n     = kep.bdsDeltaN_r12 * 1e-43 * M_PI;
        eph.m0          = kep.bdsM0_r12 * 1e-31 * M_PI;
        eph.e           = kep.bdsE_r12 * 1e-33;
        eph.omega       = kep.bdsW_r12 * 1e-31 * M_PI;
        eph.cuc         = kep.bdsCuc_r12 * 1e-31;
        eph.cus         = kep.bdsCus_r12 * 1e-31;
        eph.crc         = kep.bdsCrc_r12 * 0.0625;
        eph.crs         = kep.bdsCrs_r12 * 0.0625;
        eph.cic         = kep.bdsCic_r12 * 1e-31;
        eph.cis         = kep.bdsCis_r12 * 1e-31;
        eph.i0          = kep.bdsI0_r12 * 1e-31 * M_PI;
        eph.idot        = kep.bdsIDot_r12 * 1e-43 * M_PI;
        eph.omega0      = kep.bdsOmega0_r12 * 1e-31 * M_PI;
        eph.omega_dot   = kep.bdsOmegaDot_r12 * 1e-43 * M_PI;
        eph.af0         = clock.bdsA0_r12 * 1e-33;
        eph.af1         = clock.bdsA1_r12 * 1e-50;
        eph.af2         = clock.bdsA2_r12 * 1e-66;
        eph.toc         = clock.bdsToc_r12 * 8;
        eph.week_number = current_week;

        system.push(std::move(eph));
        DEBUGF("BeiDou ephemeris: PRN=%u IOD=%u", eph.prn, eph.iode);
    }
}

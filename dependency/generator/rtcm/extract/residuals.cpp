#include "extract.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <GNSS-FrequencyID-r15.h>
#include <GNSS-RTK-Residuals-r15.h>
#include <RTK-Residuals-Element-r15.h>
EXTERNAL_WARNINGS_POP

namespace generator {
namespace rtcm {

static uint32_t reference_station_id(GNSS_RTK_Residuals_r15 const& src_residuals) {
    return static_cast<uint32_t>(src_residuals.referenceStationID_r15.referenceStationID_r15);
}

static uint32_t n_refs(GNSS_RTK_Residuals_r15 const& src_residuals) {
    return static_cast<uint32_t>(src_residuals.n_Refs_r15);
}

static SatelliteId satellite_id(GenericGnssId                    gnss_id,
                                RTK_Residuals_Element_r15 const& src_element) {
    auto gnss = SatelliteId::Gnss::UNKNOWN;
    switch (gnss_id) {
    case GenericGnssId::GPS: gnss = SatelliteId::Gnss::GPS; break;
    case GenericGnssId::GLONASS: gnss = SatelliteId::Gnss::GLONASS; break;
    case GenericGnssId::GALILEO: gnss = SatelliteId::Gnss::GALILEO; break;
    case GenericGnssId::BEIDOU: gnss = SatelliteId::Gnss::BEIDOU; break;
    }

    auto id = src_element.svID_r15.satellite_id;
    return SatelliteId::from_lpp(gnss, id);
}

static double s_oc(RTK_Residuals_Element_r15 const& src_element) {
    return static_cast<double>(src_element.s_oc_r15) * 0.5;
}

static double s_od(RTK_Residuals_Element_r15 const& src_element) {
    return static_cast<double>(src_element.s_od_r15) * 0.01;
}

static double s_oh(RTK_Residuals_Element_r15 const& src_element) {
    return static_cast<double>(src_element.s_oh_r15) * 0.1;
}

static double s_lc(RTK_Residuals_Element_r15 const& src_element) {
    return static_cast<double>(src_element.s_lc_r15) * 0.5;
}

static double s_ld(RTK_Residuals_Element_r15 const& src_element) {
    return static_cast<double>(src_element.s_ld_r15) * 0.01;
}

// Centre frequency [MHz] of a GNSS-FrequencyID link (TS 37.355, IE GNSS-FrequencyID). GLONASS
// links depend on the frequency channel k, which is not part of the residual IE, so the
// nominal (k = 0) frequency is used.
static bool link_frequency(GenericGnssId gnss, long id, double& mhz) {
    static double const gps[]     = {1575.42, 1227.60, 1176.45};
    static double const glonass[] = {1602.0, 1246.0, 1202.025};
    static double const galileo[] = {1575.420, 1278.750, 1176.450, 1207.140, 1191.795};
    static double const bds[]     = {1561.098, 1575.420, 1207.140, 1268.520, 1176.450};

    double const* table = nullptr;
    long          count = 0;
    switch (gnss) {
    case GenericGnssId::GPS:
        table = gps;
        count = 3;
        break;
    case GenericGnssId::GLONASS:
        table = glonass;
        count = 3;
        break;
    case GenericGnssId::GALILEO:
        table = galileo;
        count = 5;
        break;
    case GenericGnssId::BEIDOU:
        table = bds;
        count = 5;
        break;
    }
    if (!table || id < 0 || id >= count) return false;
    mhz = table[id];
    return true;
}

// s-lc / s-ld are dispersive (ionospheric) standard deviations "as affecting the L1 frequency",
// where L1 is the link given by the l1 field. The RTCM residual messages define them for the
// system's default L1 link (link 0: L1, G1, E1, B1I), and a dispersive delay scales with
// 1/f^2, so a value stated for another link is rescaled by (f_l1 / f_default)^2. Absent, default
// or unknown links need no conversion.
static double dispersive_scale(GenericGnssId gnss, GNSS_RTK_Residuals_r15 const& src_residuals) {
    if (!src_residuals.l1_r15) return 1.0;

    double f_l1      = 0.0;
    double f_default = 0.0;
    if (!link_frequency(gnss, src_residuals.l1_r15->gnss_FrequencyID_r15, f_l1)) return 1.0;
    if (!link_frequency(gnss, 0, f_default)) return 1.0;

    auto ratio = f_l1 / f_default;
    return ratio * ratio;
}

static std::unique_ptr<Residuals>
extract_any_residuas(GenericGnssId gnss_id, GNSS_RTK_Residuals_r15 const& src_residuals) {
    auto  dst_residuals            = std::unique_ptr<Residuals>(new Residuals());
    auto& residuals                = *dst_residuals;
    residuals.reference_station_id = reference_station_id(src_residuals);
    residuals.time                 = epoch_time(src_residuals.epochTime_r15);
    residuals.n_refs               = n_refs(src_residuals);

    auto dispersive = dispersive_scale(gnss_id, src_residuals);

    auto& list = src_residuals.rtk_residuals_list_r15.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        auto&                element = *list.array[i];
        Residuals::Satellite satellite{};
        satellite.id   = satellite_id(gnss_id, element);
        satellite.s_oc = s_oc(element);
        satellite.s_od = s_od(element);
        satellite.s_oh = s_oh(element);
        satellite.s_lc = s_lc(element) * dispersive;
        satellite.s_ld = s_ld(element) * dispersive;
        residuals.satellites.emplace_back(satellite);
    }

    return dst_residuals;
}

extern void extract_residuals(RtkData& data, GenericGnssId gnss_id,
                              GNSS_RTK_Residuals_r15 const& src_residuals) {
    if (gnss_id == GenericGnssId::GPS) {
        data.gps_residuals = extract_any_residuas(gnss_id, src_residuals);
    } else if (gnss_id == GenericGnssId::GLONASS) {
        data.glonass_residuals = extract_any_residuas(gnss_id, src_residuals);
    } else if (gnss_id == GenericGnssId::GALILEO) {
        data.galileo_residuals = extract_any_residuas(gnss_id, src_residuals);
    } else if (gnss_id == GenericGnssId::BEIDOU) {
        data.beidou_residuals = extract_any_residuas(gnss_id, src_residuals);
    }
}

}  // namespace rtcm
}  // namespace generator

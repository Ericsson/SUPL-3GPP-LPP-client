#include "extract.hpp"

#include <GNSS-RTK-Residuals-r15.h>
#include <RTK-Residuals-Element-r15.h>

using namespace generator::rtcm;

namespace decode {

static uint32_t reference_station_id(const GNSS_RTK_Residuals_r15& src_residuals) {
    return static_cast<uint32_t>(src_residuals.referenceStationID_r15.referenceStationID_r15);
}

static uint32_t n_refs(const GNSS_RTK_Residuals_r15& src_residuals) {
    return static_cast<uint32_t>(src_residuals.n_Refs_r15);
}

static SatelliteId satellite_id(GenericGnssId                    gnss_id,
                                const RTK_Residuals_Element_r15& src_element) {
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

static double s_oc(const RTK_Residuals_Element_r15& src_element) {
    return static_cast<double>(src_element.s_oc_r15) * 0.5;
}

static double s_od(const RTK_Residuals_Element_r15& src_element) {
    return static_cast<double>(src_element.s_od_r15) * 0.01;
}

static double s_oh(const RTK_Residuals_Element_r15& src_element) {
    return static_cast<double>(src_element.s_oh_r15) * 0.1;
}

static double s_lc(const RTK_Residuals_Element_r15& src_element) {
    return static_cast<double>(src_element.s_lc_r15) * 0.5;
}

static double s_ld(const RTK_Residuals_Element_r15& src_element) {
    return static_cast<double>(src_element.s_ld_r15) * 0.01;
}
}  // namespace decode

static std::unique_ptr<Residuals>
extract_any_residuas(GenericGnssId gnss_id, const GNSS_RTK_Residuals_r15& src_residuals) {
    auto  dst_residuals            = std::unique_ptr<Residuals>(new Residuals());
    auto& residuals                = *dst_residuals;
    residuals.reference_station_id = decode::reference_station_id(src_residuals);
    residuals.time                 = decode::epoch_time(src_residuals.epochTime_r15);
    residuals.n_refs               = decode::n_refs(src_residuals);

    auto& list = src_residuals.rtk_residuals_list_r15.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        auto&                element = *list.array[i];
        Residuals::Satellite satellite{};
        satellite.id   = decode::satellite_id(gnss_id, element);
        satellite.s_oc = decode::s_oc(element);
        satellite.s_od = decode::s_od(element);
        satellite.s_oh = decode::s_oh(element);
        satellite.s_lc = decode::s_lc(element);
        satellite.s_ld = decode::s_ld(element);
        residuals.satellites.emplace_back(satellite);
    }

    return dst_residuals;
}

extern void extract_residuals(RtkData& data, GenericGnssId gnss_id,
                              const GNSS_RTK_Residuals_r15& src_residuals) {
    if (gnss_id == GenericGnssId::GPS) {
        data.gps_residuals = extract_any_residuas(gnss_id, src_residuals);
    } else if (gnss_id == GenericGnssId::GLONASS) {
        data.glonass_residuals = extract_any_residuas(gnss_id, src_residuals);
    }
}

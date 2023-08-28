#include "extract.hpp"

#include <GNSS-RTK-ReferenceStationInfo-r15.h>
#include <PhysicalReferenceStationInfo-r15.h>

using namespace generator::rtcm;

namespace decode {

static uint32_t reference_station_id(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    return static_cast<uint32_t>(src_reference.referenceStationID_r15.referenceStationID_r15);
}

static double reference_point(const INTEGER_t& src) {
    intmax_t value = 0;
    asn_INTEGER2imax(&src, &value);
    return static_cast<double>(value) * 0.0001;
}

static double reference_point_x(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.antenna_reference_point_ECEF_X_r15);
}

static double reference_point_y(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.antenna_reference_point_ECEF_Y_r15);
}

static double reference_point_z(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.antenna_reference_point_ECEF_Z_r15);
}

static Maybe<double> antenna_height(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    if (src_reference.antennaHeight_r15) {
        return static_cast<double>(*src_reference.antennaHeight_r15) * 0.0001;
    } else {
        return Maybe<double>();
    }
}

static bool is_physical_reference_station(const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    return src_reference.referenceStationIndicator_r15 ==
           GNSS_RTK_ReferenceStationInfo_r15__referenceStationIndicator_r15_physical;
}

static uint32_t reference_station_id(const PhysicalReferenceStationInfo_r15& src_reference) {
    return static_cast<uint32_t>(
        src_reference.physicalReferenceStationID_r15.referenceStationID_r15);
}

static double reference_point_x(const PhysicalReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.physical_ARP_ECEF_X_r15);
}

static double reference_point_y(const PhysicalReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.physical_ARP_ECEF_Y_r15);
}

static double reference_point_z(const PhysicalReferenceStationInfo_r15& src_reference) {
    return reference_point(src_reference.physical_ARP_ECEF_Z_r15);
}
}  // namespace decode

static void
extract_physical_reference_station_info(RtkData&                                data,
                                        const PhysicalReferenceStationInfo_r15& src_physical) {
    auto  dst_physical = std::unique_ptr<PhysicalReferenceStation>(new PhysicalReferenceStation());
    auto& physical     = *dst_physical.get();
    physical.reference_station_id = decode::reference_station_id(src_physical);
    physical.x                    = decode::reference_point_x(src_physical);
    physical.y                    = decode::reference_point_y(src_physical);
    physical.z                    = decode::reference_point_z(src_physical);

    data.physical_reference_station = std::move(dst_physical);
}

extern void extract_reference_station_info(RtkData&                                 data,
                                           const GNSS_RTK_ReferenceStationInfo_r15& src_reference) {
    auto  dst_reference            = std::unique_ptr<ReferenceStation>(new ReferenceStation());
    auto& reference                = *dst_reference.get();
    reference.reference_station_id = decode::reference_station_id(src_reference);
    reference.x                    = decode::reference_point_x(src_reference);
    reference.y                    = decode::reference_point_y(src_reference);
    reference.z                    = decode::reference_point_z(src_reference);
    reference.antenna_height       = decode::antenna_height(src_reference);
    reference.is_physical_reference_station = decode::is_physical_reference_station(src_reference);

    data.reference_station = std::move(dst_reference);

    if (src_reference.physical_reference_station_info_r15) {
        extract_physical_reference_station_info(data,
                                                *src_reference.physical_reference_station_info_r15);
    }
}
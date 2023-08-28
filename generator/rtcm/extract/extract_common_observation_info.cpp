#include "extract.hpp"

#include <GNSS-RTK-CommonObservationInfo-r15.h>
#include <asn.1/bit_string.hpp>

using namespace generator::rtcm;

namespace decode {
static uint32_t reference_station_id(const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    // TODO(ewasjon): Support parsing the reference station provider name.
    return static_cast<uint32_t>(src_common.referenceStationID_r15.referenceStationID_r15);
}

static long clock_steering(const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    return src_common.clockSteeringIndicator_r15;
}

static long external_clock(const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    return src_common.externalClockIndicator_r15;
}

static long smooth_indicator(const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    auto bit_string = helper::BitString::from(&src_common.smoothingIndicator_r15);
    return static_cast<long>(bit_string->get_bit(0));
}

static long smooth_interval(const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    auto bit_string = helper::BitString::from(&src_common.smoothingInterval_r15);
    return static_cast<long>(bit_string->as_int64());
}

}  // namespace decode

extern void extract_common_observation_info(RtkData&                                  data,
                                       const GNSS_RTK_CommonObservationInfo_r15& src_common) {
    auto  dst_common = std::unique_ptr<CommonObservationInfo>(new CommonObservationInfo());
    auto& common     = *dst_common.get();
    common.reference_station_id = decode::reference_station_id(src_common);
    common.clock_steering       = decode::clock_steering(src_common);
    common.external_clock       = decode::external_clock(src_common);
    common.smooth_indicator     = decode::smooth_indicator(src_common);
    common.smooth_interval      = decode::smooth_interval(src_common);

    data.common_observation_info = std::move(dst_common);
}

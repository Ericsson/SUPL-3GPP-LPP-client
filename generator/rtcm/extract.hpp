#pragma once
#include "rtk_data.hpp"

struct GNSS_RTK_CommonObservationInfo_r15;
struct GNSS_RTK_ReferenceStationInfo_r15;
struct GNSS_RTK_Observations_r15;

extern void extract_common_observation_info(RtkData& data, const GNSS_RTK_CommonObservationInfo_r15& src_common);
extern void extract_reference_station_info(RtkData& data, const GNSS_RTK_ReferenceStationInfo_r15& src_reference);
extern void extract_observations(RtkData& data, GenericGnssId gnss_id, const GNSS_RTK_Observations_r15& src_observation);

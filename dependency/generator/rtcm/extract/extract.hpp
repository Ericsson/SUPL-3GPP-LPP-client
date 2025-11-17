#pragma once
#include "rtk_data.hpp"

#include <time/tai.hpp>

struct GNSS_RTK_CommonObservationInfo_r15;
struct GNSS_RTK_ReferenceStationInfo_r15;
struct GNSS_RTK_Observations_r15;
struct GLO_RTK_BiasInformation_r15;
struct GNSS_RTK_Residuals_r15;
struct GNSS_AuxiliaryInformation;
struct GNSS_SystemTime;

namespace generator {
namespace rtcm {

extern void extract_common_observation_info(RtkData&                                  data,
                                            GNSS_RTK_CommonObservationInfo_r15 const& src_common);
extern void extract_reference_station_info(RtkData&                                 data,
                                           GNSS_RTK_ReferenceStationInfo_r15 const& src_reference);
extern void extract_observations(RtkData& data, GenericGnssId gnss_id,
                                 GNSS_RTK_Observations_r15 const& src_observation);
extern void extract_bias_information(RtkData& data, GLO_RTK_BiasInformation_r15 const& src_bias);
extern void extract_residuals(RtkData& data, GenericGnssId gnss_id,
                              GNSS_RTK_Residuals_r15 const& src_residuals);
extern void extract_auxiliary_information(RtkData& data, GNSS_AuxiliaryInformation const& src_aux);

long          day_number(GNSS_SystemTime const& src_time);
double        time_of_day(GNSS_SystemTime const& src_time);
Maybe<double> time_of_day_fraction(GNSS_SystemTime const& src_time);
GenericGnssId gnss_id(GNSS_SystemTime const& src_time);
ts::Tai       epoch_time(GNSS_SystemTime const& src_time);

}  // namespace rtcm
}  // namespace generator

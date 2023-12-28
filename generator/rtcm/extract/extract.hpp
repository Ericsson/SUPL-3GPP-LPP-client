#pragma once
#include "rtk_data.hpp"
#include "time/bdt_time.hpp"
#include "time/glo_time.hpp"
#include "time/gps_time.hpp"
#include "time/gst_time.hpp"
#include "time/tai_time.hpp"

struct GNSS_RTK_CommonObservationInfo_r15;
struct GNSS_RTK_ReferenceStationInfo_r15;
struct GNSS_RTK_Observations_r15;
struct GLO_RTK_BiasInformation_r15;
struct GNSS_RTK_Residuals_r15;
struct GNSS_AuxiliaryInformation;

extern void extract_common_observation_info(generator::rtcm::RtkData&                 data,
                                            const GNSS_RTK_CommonObservationInfo_r15& src_common);
extern void extract_reference_station_info(generator::rtcm::RtkData&                data,
                                           const GNSS_RTK_ReferenceStationInfo_r15& src_reference);
extern void extract_observations(generator::rtcm::RtkData& data, GenericGnssId gnss_id,
                                 const GNSS_RTK_Observations_r15& src_observation);
extern void extract_bias_information(generator::rtcm::RtkData&          data,
                                     const GLO_RTK_BiasInformation_r15& src_bias);
extern void extract_residuals(generator::rtcm::RtkData& data, GenericGnssId gnss_id,
                              const GNSS_RTK_Residuals_r15& src_residuals);
extern void extract_auxiliary_information(generator::rtcm::RtkData&        data,
                                          const GNSS_AuxiliaryInformation& src_aux);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <GNSS-ID.h>
#include <GNSS-SystemTime.h>
#pragma GCC diagnostic pop

namespace decode {

static long day_number(const GNSS_SystemTime& src_time) {
    return src_time.gnss_DayNumber;
}

static double time_of_day(const GNSS_SystemTime& src_time) {
    return static_cast<double>(src_time.gnss_TimeOfDay);
}

static Maybe<double> time_of_day_fraction(const GNSS_SystemTime& src_time) {
    if (src_time.gnss_TimeOfDayFrac_msec) {
        return static_cast<double>(*src_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    } else {
        return Maybe<double>();
    }
}

static GenericGnssId gnss_id(const GNSS_SystemTime& src_time) {
    switch (src_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: return GenericGnssId::GPS;
    case GNSS_ID__gnss_id_glonass: return GenericGnssId::GLONASS;
    case GNSS_ID__gnss_id_galileo: return GenericGnssId::GALILEO;
    case GNSS_ID__gnss_id_bds: return GenericGnssId::BEIDOU;
    }

    RTCM_UNREACHABLE();
    return GenericGnssId::GPS;
}

static ts::TAI_Time epoch_time(const GNSS_SystemTime& src_time) {
    auto day_number           = decode::day_number(src_time);
    auto time_of_day_seconds  = decode::time_of_day(src_time);
    auto time_of_day_fraction = decode::time_of_day_fraction(src_time);
    auto time_of_day          = time_of_day_seconds;
    if (time_of_day_fraction.valid) {
        time_of_day += time_of_day_fraction.value;
    }

    auto gnss = decode::gnss_id(src_time);
    switch (gnss) {
    case GenericGnssId::GPS: return ts::TAI_Time(ts::GPS_Time(day_number, time_of_day));
    case GenericGnssId::GLONASS: return ts::TAI_Time(ts::GLO_Time(day_number, time_of_day));
    case GenericGnssId::GALILEO: return ts::TAI_Time(ts::GST_Time(day_number, time_of_day));
    case GenericGnssId::BEIDOU: return ts::TAI_Time(ts::BDT_Time(day_number, time_of_day));
    }

    return ts::TAI_Time::now();
}
}  // namespace decode

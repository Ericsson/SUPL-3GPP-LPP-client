#pragma once
#include "lpp/location_information.h"

namespace location_information {
struct LocationInformation;
struct HaGnssMetrics;
struct ECIDInformation;
}  // namespace location_information

struct LocationInformationOptions;

location_information::PLI_Result
provide_location_information_callback(location_information::LocationInformation& location,
                                      location_information::HaGnssMetrics& metrics, void* userdata);
location_information::PLI_Result
provide_location_information_callback_ublox(location_information::LocationInformation& location,
                                            location_information::HaGnssMetrics&       metrics,
                                            void*                                      userdata);
location_information::PLI_Result
provide_location_information_callback_nmea(location_information::LocationInformation& location,
                                           location_information::HaGnssMetrics&       metrics,
                                           void*                                      userdata);
location_information::PLI_Result
     provide_location_information_callback_fake(location_information::LocationInformation& location,
                                                location_information::HaGnssMetrics&       metrics,
                                                void*                                      userdata);
bool provide_ecid_callback(location_information::ECIDInformation& ecid, void* userdata);

extern bool   gConvertConfidence95To39;
extern double gOverrideHorizontalConfidence;

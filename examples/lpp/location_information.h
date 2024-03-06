#pragma once

namespace location_information {
struct LocationInformation;
struct HaGnssMetrics;
struct ECIDInformation;
}  // namespace location_information

struct LocationInformationOptions;

bool provide_location_information_callback(location_information::LocationInformation& location,
                                           location_information::HaGnssMetrics&       metrics,
                                           void*                                      userdata);
bool provide_location_information_callback_ublox(
    location_information::LocationInformation& location,
    location_information::HaGnssMetrics& metrics, void* userdata);
bool provide_location_information_callback_nmea(location_information::LocationInformation& location,
                                                location_information::HaGnssMetrics&       metrics,
                                                void* userdata);
bool provide_location_information_callback_fake(location_information::LocationInformation& location,
                                                location_information::HaGnssMetrics&       metrics,
                                                void* userdata);
bool provide_ecid_callback(location_information::ECIDInformation& ecid, void* userdata);

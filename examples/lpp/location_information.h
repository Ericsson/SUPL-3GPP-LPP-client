#pragma once

struct LocationInformation;
struct HaGnssMetrics;
struct ECIDInformation;
struct LocationInformationOptions;

bool provide_location_information_callback(LocationInformation& location, HaGnssMetrics& metrics,
                                           void* userdata);
bool provide_location_information_callback_ublox(LocationInformation& location,
                                                 HaGnssMetrics& metrics, void* userdata);
bool provide_location_information_callback_nmea(LocationInformation& location,
                                                HaGnssMetrics& metrics, void* userdata);
bool provide_location_information_callback_fake(LocationInformation& location,
                                                HaGnssMetrics& metrics, void* userdata);
bool provide_ecid_callback(ECIDInformation& ecid, void* userdata);

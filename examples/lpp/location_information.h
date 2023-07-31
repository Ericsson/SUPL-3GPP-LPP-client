#pragma once

struct LocationInformation;
struct HaGnssMetrics;
struct ECIDInformation;

bool provide_location_information_callback(LocationInformation& location, HaGnssMetrics& metrics,
                                           void* userdata);
bool provide_ecid_callback(ECIDInformation& ecid, void* userdata);

#pragma once
#include "lpp/location_information.h"

#include <streamline/inspector.hpp>

namespace location_information {
struct LocationInformation;
struct HaGnssMetrics;
struct ECIDInformation;
}  // namespace location_information

struct LocationInformationOptions;

location_information::PLI_Result provide_location_information_callback_streamline(
    location_information::LocationInformation& location,
    location_information::HaGnssMetrics& metrics, void* userdata);
location_information::PLI_Result
provide_location_information_callback(location_information::LocationInformation& location,
                                      location_information::HaGnssMetrics& metrics, void* userdata);
location_information::PLI_Result
     provide_location_information_callback_fake(location_information::LocationInformation& location,
                                                location_information::HaGnssMetrics&       metrics,
                                                void*                                      userdata);
bool provide_ecid_callback(location_information::ECIDInformation& ecid, void* userdata);

extern bool   gConvertConfidence95To68;
extern double gOverrideHorizontalConfidence;
extern bool   gOutputEllipse68;

class LocationCollector : public streamline::Inspector<location_information::LocationInformation> {
public:
    void inspect(streamline::System&, DataType const& location) NOEXCEPT override;
};

class MetricsCollector : public streamline::Inspector<location_information::HaGnssMetrics> {
public:
    void inspect(streamline::System&, DataType const& metrics) NOEXCEPT override;
};

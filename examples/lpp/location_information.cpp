#include "location_information.h"
#include <cmath>
#include <format/nmea/gga.hpp>
#include <format/ubx/messages/nav_pvt.hpp>
#include "lpp/location_information.h"
#include "options.hpp"

bool   gConvertConfidence95To68      = false;
double gOverrideHorizontalConfidence = -1;
bool   gOutputEllipse68              = false;

static bool                                      gHasLocation = false;
static location_information::LocationInformation gLocation;

static bool                                gHasMetrics = false;
static location_information::HaGnssMetrics gMetrics;

void LocationCollector::inspect(streamline::System&, DataType const& location) NOEXCEPT {
    gHasLocation = true;
    gLocation    = location;
}

void MetricsCollector::inspect(streamline::System&, DataType const& metrics) NOEXCEPT {
    gHasMetrics = true;
    gMetrics    = metrics;
}

using namespace location_information;

PLI_Result provide_location_information_callback_streamline(LocationInformation& location,
                                                            HaGnssMetrics&       metrics, void*) {
    if (gHasLocation) {
        location = gLocation;
    }
    if (gHasMetrics) {
        metrics = gMetrics;
    }
    if (gHasLocation && gHasMetrics) {
        return PLI_Result::LI_AND_METRICS;
    } else if (gHasLocation) {
        return PLI_Result::LI;
    } else {
        return PLI_Result::NOT_AVAILABLE;
    }
}

PLI_Result provide_location_information_callback(UNUSED LocationInformation& location,
                                                 UNUSED HaGnssMetrics&       metrics,
                                                 UNUSED void*                userdata) {
#if 0
    // Example implementation
    location.time     = ts::Tai::now();
    location.location = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        20, 25, 30, HorizontalAccuracy::from_ellipse(0.5, 0.5, 0),
        VerticalAccuracy::from_1sigma(0.5));

    location.velocity = VelocityShape::horizontal_vertical_with_uncertainty(10, 0.5, 90, 1, 0.5,
                                                                            VerticalDirection::Up);

    metrics.fix_quality          = FixQuality::RTK_FIX;
    metrics.number_of_satellites = 30;
    metrics.age_of_corrections   = 5;
    metrics.hdop                 = 10;
    metrics.pdop                 = 15;
    return PLI_Result::LI_AND_METRICS;
#else
    return PLI_Result::NOT_AVAILABLE;
#endif
}

PLI_Result provide_location_information_callback_fake(LocationInformation&  location,
                                                      UNUSED HaGnssMetrics& metrics,
                                                      void*                 userdata) {
    auto options = reinterpret_cast<LocationInformationOptions*>(userdata);
    if (!options) return PLI_Result::NOT_AVAILABLE;

    location.time     = ts::Tai::now();
    location.location = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        options->latitude, options->longitude, options->altitude,
        HorizontalAccuracy::to_ellipse_39(0.5, 0.5, 0), VerticalAccuracy::from_1sigma(0.5));

    location.velocity = VelocityShape::horizontal_vertical_with_uncertainty(10, 0.5, 90, 1, 0.5,
                                                                            VerticalDirection::Up);

    if (rand() % 2 == 0) {
        metrics.fix_quality = FixQuality::RTK_FIX;
    } else {
        metrics.fix_quality = FixQuality::RTK_FLOAT;
    }
    metrics.age_of_corrections   = 5;
    metrics.number_of_satellites = 1;
    metrics.hdop                 = 10;
    metrics.pdop                 = 15;
    return PLI_Result::LI_AND_METRICS;
}

bool provide_ecid_callback(UNUSED ECIDInformation& ecid, UNUSED void* userdata) {
    // TODO(ewasjon): Implement using information from the control interface
    return false;
}

#include "lpp/messages/provide_location_information.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <A-GNSS-ProvideLocationInformation.h>
#include <CommonIEsProvideLocationInformation.h>
#include <GNSS-LocationInformation.h>
#include <GNSS-SupportElement.h>
#include <GNSS-SupportList.h>
#include <HA-GNSS-Metrics-r17.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <LocationCoordinates.h>
#include <LocationError.h>
#include <PositioningModes.h>
#include <ProvideLocationInformation-r9-IEs.h>
#include <ProvideLocationInformation.h>
#include <Velocity.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>
#include <cmath>
#include <loglet/loglet.hpp>
#include <time/gps.hpp>

LOGLET_MODULE2(lpp, pli);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(lpp, pli)

#define ALLOC_ZERO(type) reinterpret_cast<type*>(calloc(1, sizeof(type)))

namespace lpp {
namespace messages {

static long encode_latitude(double lat) {
    auto value = (lat / 90.0) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return static_cast<long>(value);
}

static long encode_longitude(double lon) {
    auto value = (lon / 180) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return static_cast<long>(value);
}

static long encode_ha_uncertainity(double r) {
    auto C     = 0.3;
    auto x     = 0.02;
    auto k     = log((r / C) + 1) / log(1 + x);
    auto value = static_cast<long>(k);
    if (value <= 0) value = 0;
    if (value >= 255) value = 255;
    return value;
}

static long encode_ha_altitude(double a) {
    long res = static_cast<long>(a * 128.0);
    if (res < -64000) return -64000;
    if (res > 1280000) return 1280000;
    return res;
}

static long encode_orientation(double orientation) {
    auto value = static_cast<long>(orientation);
    if (value < 0) value = 0;
    if (value > 179) value = 179;
    return value;
}

static long encode_confidence(double confidence) {
    auto value = static_cast<long>(confidence * 100.0);
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    return value;
}

static LocationCoordinates_t* location_coordinates_haepue(LocationShape const& shape) {
    VSCOPE_FUNCTION();
    auto location_coordinates = ALLOC_ZERO(LocationCoordinates_t);
    location_coordinates->present =
        LocationCoordinates_PR_highAccuracyEllipsoidPointWithUncertaintyEllipse_v1510;

    auto& data =
        location_coordinates->choice.highAccuracyEllipsoidPointWithUncertaintyEllipse_v1510;
    data.degreesLatitude_r15  = encode_latitude(shape.data.haepue.latitude);
    data.degreesLongitude_r15 = encode_longitude(shape.data.haepue.longitude);
    data.uncertaintySemiMajor_r15 =
        encode_ha_uncertainity(shape.data.haepue.horizontal_accuracy.semi_major);
    data.uncertaintySemiMinor_r15 =
        encode_ha_uncertainity(shape.data.haepue.horizontal_accuracy.semi_minor);
    data.orientationMajorAxis_r15 =
        encode_orientation(shape.data.haepue.horizontal_accuracy.orientation);
    data.confidence_r15 = encode_confidence(shape.data.haepue.horizontal_accuracy.confidence);
    return location_coordinates;
}

static LocationCoordinates_t* location_coordinates_haepaue(LocationShape const& shape) {
    VSCOPE_FUNCTION();
    auto location_coordinates = ALLOC_ZERO(LocationCoordinates_t);
    location_coordinates->present =
        LocationCoordinates_PR_highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510;

    auto& data = location_coordinates->choice
                     .highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510;
    data.degreesLatitude_r15  = encode_latitude(shape.data.haepaue.latitude);
    data.degreesLongitude_r15 = encode_longitude(shape.data.haepaue.longitude);
    data.altitude_r15         = encode_ha_altitude(shape.data.haepaue.altitude);
    data.uncertaintySemiMajor_r15 =
        encode_ha_uncertainity(shape.data.haepaue.horizontal_accuracy.semi_major);
    data.uncertaintySemiMinor_r15 =
        encode_ha_uncertainity(shape.data.haepaue.horizontal_accuracy.semi_minor);
    data.orientationMajorAxis_r15 =
        encode_orientation(shape.data.haepaue.horizontal_accuracy.orientation);
    data.horizontalConfidence_r15 =
        encode_confidence(shape.data.haepaue.horizontal_accuracy.confidence);
    data.uncertaintyAltitude_r15 =
        encode_ha_uncertainity(shape.data.haepaue.vertical_accuracy.uncertainty);
    data.verticalConfidence_r15 =
        encode_confidence(shape.data.haepaue.vertical_accuracy.confidence);
    return location_coordinates;
}

static LocationCoordinates_t* location_coordinates(LocationShape const& shape) {
    switch (shape.kind) {
    case LocationShape::Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse:
        return location_coordinates_haepue(shape);
    case LocationShape::Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
        return location_coordinates_haepaue(shape);
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); return nullptr;
#endif
    }
}

static long encode_velocity(double vel, long max) {
    long N;
    vel = abs(vel) * 3.6;
    if (vel < 0.5) {
        N = 0;
    } else if (vel >= static_cast<double>(max) + 0.5) {
        N = max;
    } else {
        N = static_cast<long>(round(vel));
    }
    return N;
}

static long encode_bearing(double bearing) {
    while (bearing < 0.0)
        bearing += 360.0;
    while (bearing >= 360.0)
        bearing -= 360.0;
    auto value = static_cast<long>(bearing);
    if (value < 0) value = 0;
    if (value > 359) value = 359;
    return value;
}

static Velocity_t* horizontal_velocity(VelocityShape const& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalVelocity;

    auto& data           = element->choice.horizontalVelocity;
    data.bearing         = encode_bearing(shape.data.hv.horizontal.bearing);
    data.horizontalSpeed = encode_velocity(shape.data.hv.horizontal.speed, 2047);
    return element;
}

static Velocity_t* horizontal_velocity_with_uncertainty(VelocityShape const& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalVelocityWithUncertainty;

    auto& data            = element->choice.horizontalVelocityWithUncertainty;
    data.bearing          = encode_bearing(shape.data.hvu.horizontal.bearing);
    data.horizontalSpeed  = encode_velocity(shape.data.hvu.horizontal.speed, 2047);
    data.uncertaintySpeed = encode_velocity(shape.data.hvu.horizontal.uncertainty, 255);
    return element;
}

static Velocity_t* horizontal_with_vertical_velocity(VelocityShape const& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalWithVerticalVelocity;

    auto& data             = element->choice.horizontalWithVerticalVelocity;
    data.bearing           = encode_bearing(shape.data.hvv.horizontal.bearing);
    data.horizontalSpeed   = encode_velocity(shape.data.hvv.horizontal.speed, 2047);
    data.verticalSpeed     = encode_velocity(shape.data.hvv.vertical.speed, 255);
    data.verticalDirection = shape.data.hvv.vertical.direction == VerticalDirection::Up ?
                                 HorizontalWithVerticalVelocity__verticalDirection_upward :
                                 HorizontalWithVerticalVelocity__verticalDirection_downward;
    return element;
}

static Velocity_t* horizontal_with_vertical_velocity_and_uncertainty(VelocityShape const& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalWithVerticalVelocityAndUncertainty;

    auto& data                      = element->choice.horizontalWithVerticalVelocityAndUncertainty;
    data.bearing                    = encode_bearing(shape.data.hvvu.horizontal.bearing);
    data.horizontalSpeed            = encode_velocity(shape.data.hvvu.horizontal.speed, 2047);
    data.horizontalUncertaintySpeed = encode_velocity(shape.data.hvvu.horizontal.uncertainty, 255);
    data.verticalSpeed              = encode_velocity(shape.data.hvvu.vertical.speed, 255);
    data.verticalUncertaintySpeed   = encode_velocity(shape.data.hvvu.vertical.uncertainty, 255);
    data.verticalDirection =
        shape.data.hvvu.vertical.direction == VerticalDirection::Up ?
            HorizontalWithVerticalVelocityAndUncertainty__verticalDirection_upward :
            HorizontalWithVerticalVelocityAndUncertainty__verticalDirection_downward;
    return element;
}

static Velocity_t* velocity(VelocityShape const& shape) {
    switch (shape.kind) {
    case VelocityShape::Kind::HorizontalVelocity: return horizontal_velocity(shape);
    case VelocityShape::Kind::HorizontalVelocityWithUncertainty:
        return horizontal_velocity_with_uncertainty(shape);
    case VelocityShape::Kind::HorizontalWithVerticalVelocity:
        return horizontal_with_vertical_velocity(shape);
    case VelocityShape::Kind::HorizontalWithVerticalVelocityAndUncertainty:
        return horizontal_with_vertical_velocity_and_uncertainty(shape);
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    default: UNREACHABLE(); return nullptr;
#endif
    }
}

static CommonIEsProvideLocationInformation*
common_ies_provide_location_information(ProvideLocationInformation const& data) {
    VSCOPE_FUNCTION();

    auto message = ALLOC_ZERO(CommonIEsProvideLocationInformation);
    if (data.location_information.has_value()) {
        auto const& location_information = data.location_information.const_value();

        if (location_information.location.has_value()) {
            message->locationEstimate =
                location_coordinates(location_information.location.const_value());
        }

        if (location_information.velocity.has_value()) {
            message->velocityEstimate = velocity(location_information.velocity.const_value());
        }
    } else {
        auto error                  = ALLOC_ZERO(LocationError);
        error->locationfailurecause = LocationFailureCause_periodicLocationMeasurementsNotAvailable;
        message->locationError      = error;
    }

    return message;
}

static HA_GNSS_Metrics_r17* lpp_ha_gnss_metrics_r17(HaGnssMetrics const& metrics) {
    auto element                    = ALLOC_ZERO(HA_GNSS_Metrics_r17);
    element->nrOfUsedSatellites_r17 = metrics.number_of_satellites;

    switch (metrics.fix_quality) {
    case FixQuality::INVALID:
    case FixQuality::STANDALONE:
    case FixQuality::DGPS_FIX:
    case FixQuality::PPS_FIX:
    case FixQuality::DEAD_RECKONING:
    case FixQuality::MANUAL_INPUT:
    case FixQuality::SIMULATION:
    case FixQuality::WAAS_FIX: break;
    case FixQuality::RTK_FIX: {
        element->fixType_r17  = ALLOC_ZERO(long);
        *element->fixType_r17 = HA_GNSS_Metrics_r17__fixType_r17_carrier_phase_fix;
    } break;
    case FixQuality::RTK_FLOAT: {
        element->fixType_r17  = ALLOC_ZERO(long);
        *element->fixType_r17 = HA_GNSS_Metrics_r17__fixType_r17_carrier_phase_float;
    } break;
    }

    if (metrics.age_of_corrections.has_value()) {
        auto age = metrics.age_of_corrections.const_value() / 0.1;
        if (age < 0) age = 0;
        if (age > 99) age = 99;
        element->age_r17  = ALLOC_ZERO(long);
        *element->age_r17 = static_cast<long>(age);
    }

    if (metrics.hdop.has_value()) {
        auto hdop = metrics.hdop.const_value() / 0.1;
        if (hdop < 1) hdop = 1;
        if (hdop > 256) hdop = 256;
        element->hdopi_r17  = ALLOC_ZERO(long);
        *element->hdopi_r17 = static_cast<long>(hdop);
    }

    if (metrics.pdop.has_value()) {
        auto pdop = metrics.pdop.const_value() / 0.1;
        if (pdop < 1) pdop = 1;
        if (pdop > 256) pdop = 256;
        element->pdopi_r17  = ALLOC_ZERO(long);
        *element->pdopi_r17 = static_cast<long>(metrics.pdop.const_value() / 0.1);
    }

    return element;
}

static GNSS_LocationInformation* gnss_location_information(ProvideLocationInformation const& data) {
    if (!data.location_information.has_value()) {
        return nullptr;
    }

    auto& location_information = data.location_information.const_value();

    auto  message = ALLOC_ZERO(GNSS_LocationInformation);
    auto& mrt     = message->measurementReferenceTime;

    auto time = ts::Gps{location_information.time};
    auto tod  = time.time_of_day();
    // time of day in milliseconds
    auto msec = static_cast<long>(tod.full_seconds() * 1000);
    // time of day in 250 nanoseconds
    auto nfrac = tod.full_seconds() * 1000.0 - static_cast<double>(msec);
    nfrac *= 1000.0 * 4.0;

    mrt.gnss_TimeID.gnss_id = GNSS_ID__gnss_id_gps;
    // only take the first 3600 * 1000 milliseconds of the day
    mrt.gnss_TOD_msec = msec % (3600 * 1000);

    if (nfrac > 0) {
        mrt.gnss_TOD_frac  = ALLOC_ZERO(long);
        *mrt.gnss_TOD_frac = static_cast<long>(nfrac);
    }

    if (data.gnss_metrics.has_value()) {
        auto ext1 = ALLOC_ZERO(GNSS_LocationInformation::GNSS_LocationInformation__ext1);
        ext1->ha_GNSS_Metrics_r17 = lpp_ha_gnss_metrics_r17(data.gnss_metrics.const_value());
        message->ext1             = ext1;
    }

    return message;
}

static A_GNSS_ProvideLocationInformation*
a_gnss_provide_location_information(ProvideLocationInformation const& data) {
    auto message                      = ALLOC_ZERO(A_GNSS_ProvideLocationInformation);
    message->gnss_LocationInformation = gnss_location_information(data);
    return message;
}

static void provide_location_information_r9(ProvideLocationInformation_r9_IEs& message,
                                            ProvideLocationInformation const&  data) {
    message.commonIEsProvideLocationInformation = common_ies_provide_location_information(data);
    message.a_gnss_ProvideLocationInformation   = a_gnss_provide_location_information(data);
}

Message create_provide_location_information(ProvideLocationInformation const& data) {
    auto body               = ALLOC_ZERO(LPP_MessageBody);
    body->present           = LPP_MessageBody_PR_c1;
    body->choice.c1.present = LPP_MessageBody__c1_PR_provideLocationInformation;

    auto body_ce     = &body->choice.c1.choice.provideLocationInformation.criticalExtensions;
    body_ce->present = ProvideLocationInformation__criticalExtensions_PR_c1;
    body_ce->choice.c1.present =
        ProvideLocationInformation__criticalExtensions__c1_PR_provideLocationInformation_r9;

    auto body_ce_c1 = &body_ce->choice.c1.choice.provideLocationInformation_r9;
    provide_location_information_r9(*body_ce_c1, data);

    auto message             = ALLOC_ZERO(LPP_Message);
    message->lpp_MessageBody = body;
    return Message{message};
}

}  // namespace messages
}  // namespace lpp

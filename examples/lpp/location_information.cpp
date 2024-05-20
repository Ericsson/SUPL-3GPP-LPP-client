#include "location_information.h"
#include <cmath>
#include <receiver/nmea/gga.hpp>
#include <receiver/nmea/threaded_receiver.hpp>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <receiver/ublox/ubx_nav_pvt.hpp>
#include "lpp/location_information.h"
#include "options.hpp"
#include "utility/types.h"

bool   gConvertConfidence95To39      = false;
double gOverrideHorizontalConfidence = -1;

using namespace location_information;

PLI_Result provide_location_information_callback(UNUSED LocationInformation& location,
                                                 UNUSED HaGnssMetrics&       metrics,
                                                 UNUSED void*                userdata) {
#if 0
    // Example implementation
    location.tai_time      = TAI_Time::now();
    location.latitude  = 20;
    location.longitude = 25;
    location.altitude = 30;
    location.bearing = 132;
    location.horizontal_accuracy = 1;
    location.horizontal_speed = 2;
    location.horizontal_speed_accuracy = 3;
    location.vertical_accuracy = 1;
    location.vertical_speed = 2;
    location.vertical_speed_accuracy = 3;
    location.vertical_velocity_direction = VerticalDirection::UP;

    metrics.fixq = FixQuality::RTK_FIX;
    metrics.sats = 30;
    metrics.age = 5;
    metrics.hdop = 10;
    metrics.vdop = 15;
    return PLI_Result::LI_AND_METRICS;
#else
    return PLI_Result::NOT_AVAILABLE;
#endif
}

PLI_Result provide_location_information_callback_ublox(UNUSED LocationInformation& location,
                                                       UNUSED HaGnssMetrics&       metrics,
                                                       void*                       userdata) {
    auto receiver = reinterpret_cast<receiver::ublox::ThreadedReceiver*>(userdata);
    if (!receiver) return PLI_Result::NOT_AVAILABLE;

    auto nav_pvt = receiver->nav_pvt();
    if (!nav_pvt) return PLI_Result::NOT_AVAILABLE;

    // TODO(ewasjon): Should we use the system time if the UTC time from u-blox is invalid?
    if (!nav_pvt->valid_time()) {
        printf("u-blox time is invalid\n");
        return PLI_Result::NOT_AVAILABLE;
    }

    auto semi_major = nav_pvt->h_acc();
    auto semi_minor = nav_pvt->h_acc();
    if (gConvertConfidence95To39) {
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy = HorizontalAccuracy::from_ellipse(semi_major, semi_minor, 0);
    if (gOverrideHorizontalConfidence >= 0.0) {
        horizontal_accuracy.confidence = gOverrideHorizontalConfidence;
    }

    auto location_shape = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        nav_pvt->latitude(), nav_pvt->longitude(), nav_pvt->altitude(), horizontal_accuracy,
        VerticalAccuracy::from_1sigma(nav_pvt->v_acc()));

    auto velocity_shape = VelocityShape::horizontal_vertical_with_uncertainty(
        nav_pvt->h_vel(), nav_pvt->h_vel_acc(), nav_pvt->head_mot(), fabs(nav_pvt->v_vel()),
        nav_pvt->v_vel_acc(),
        nav_pvt->v_vel() > 0 ? VerticalDirection::Down : VerticalDirection::Up);

    location.time     = nav_pvt->tai_time();
    location.location = location_shape;
    location.velocity = velocity_shape;

    metrics.fix_quality = FixQuality::INVALID;
    if (nav_pvt->fix_type() == 3) {
        if (nav_pvt->carr_soln() == 2) {
            metrics.fix_quality = FixQuality::RTK_FIX;
        } else if (nav_pvt->carr_soln() == 1) {
            metrics.fix_quality = FixQuality::RTK_FLOAT;
        } else {
            metrics.fix_quality = FixQuality::STANDALONE;
        }
    } else if (nav_pvt->fix_type() == 2) {
        metrics.fix_quality = FixQuality::STANDALONE;
    } else if (nav_pvt->fix_type() == 1) {
        metrics.fix_quality = FixQuality::DEAD_RECKONING;
    }

    metrics.number_of_satellites = nav_pvt->num_sv();
    metrics.pdop                 = nav_pvt->p_dop();
    return PLI_Result::LI_AND_METRICS;
}

PLI_Result provide_location_information_callback_nmea(LocationInformation& location,
                                                      HaGnssMetrics& metrics, void* userdata) {
    auto receiver = reinterpret_cast<receiver::nmea::ThreadedReceiver*>(userdata);
    if (!receiver) return PLI_Result::NOT_AVAILABLE;

    // NOTE(ewasjon): We require GGA, VTG, and GST message to produce a valid location information,
    // if either is missing we will skip sending a provider location information message.
    auto gga = receiver->gga();
    auto vtg = receiver->vtg();
    auto gst = receiver->gst();
    if (!gga || !vtg || !gst) {
        return PLI_Result::NOT_AVAILABLE;
    }

    auto semi_major = gst->semi_major();
    auto semi_minor = gst->semi_minor();

    if (gConvertConfidence95To39) {
        semi_major = semi_major / 2.4477;
        semi_minor = semi_minor / 2.4477;
    }

    auto horizontal_accuracy =
        HorizontalAccuracy::from_ellipse(semi_major, semi_minor, gst->orientation());
    if (gOverrideHorizontalConfidence >= 0.0) {
        horizontal_accuracy.confidence = gOverrideHorizontalConfidence;
    }

    location.time     = gga->time_of_day();
    location.location = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        gga->latitude(), gga->longitude(), gga->altitude(), horizontal_accuracy,
        VerticalAccuracy::from_1sigma(gst->vertical_position_error()));
    location.velocity =
        VelocityShape::horizontal(vtg->speed_over_ground(), vtg->true_course_over_ground());

    metrics.fix_quality          = FixQuality::INVALID;
    metrics.number_of_satellites = gga->satellites_in_view();
    metrics.hdop                 = gga->h_dop();
    metrics.age_of_corrections   = gga->age_of_differential_corrections();

    switch (gga->fix_quality()) {
    case receiver::nmea::GgaFixQuality::Invalid: metrics.fix_quality = FixQuality::INVALID; break;
    case receiver::nmea::GgaFixQuality::GpsFix: metrics.fix_quality = FixQuality::STANDALONE; break;
    case receiver::nmea::GgaFixQuality::DgpsFix: metrics.fix_quality = FixQuality::DGPS_FIX; break;
    case receiver::nmea::GgaFixQuality::PpsFix: metrics.fix_quality = FixQuality::PPS_FIX; break;
    case receiver::nmea::GgaFixQuality::RtkFixed: metrics.fix_quality = FixQuality::RTK_FIX; break;
    case receiver::nmea::GgaFixQuality::RtkFloat:
        metrics.fix_quality = FixQuality::RTK_FLOAT;
        break;
    case receiver::nmea::GgaFixQuality::DeadReckoning:
        metrics.fix_quality = FixQuality::DEAD_RECKONING;
        break;
    }

    return PLI_Result::LI_AND_METRICS;
}

PLI_Result provide_location_information_callback_fake(LocationInformation&  location,
                                                      UNUSED HaGnssMetrics& metrics,
                                                      void*                 userdata) {
    auto options = reinterpret_cast<LocationInformationOptions*>(userdata);
    if (!options) return PLI_Result::NOT_AVAILABLE;

    location.time     = TAI_Time::now();
    location.location = LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        options->latitude, options->longitude, options->altitude,
        HorizontalAccuracy::from_ellipse(0.5, 0.5, 0), VerticalAccuracy::from_1sigma(0.5));

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

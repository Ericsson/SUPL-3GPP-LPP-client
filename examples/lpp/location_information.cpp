#include "location_information.h"
#include <cmath>
#include <modem.h>
#include <receiver/nmea/gga.hpp>
#include <receiver/nmea/threaded_receiver.hpp>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <receiver/ublox/ubx_nav_pvt.hpp>
#include "lpp/location_information.h"
#include "options.hpp"
#include "utility/types.h"

bool provide_location_information_callback(UNUSED LocationInformation& location,
                                           UNUSED HaGnssMetrics& metrics, UNUSED void* userdata) {
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

    metrics.fixq = FixQuality::MANUAL_INPUT;
    metrics.sats = 5;
    metrics.age = 0;
    metrics.hdop = 0;
    metrics.vdop = 0;
    return true;
#else
    return false;
#endif
}

bool provide_location_information_callback_ublox(UNUSED LocationInformation& location,
                                                 UNUSED HaGnssMetrics& metrics, void* userdata) {
    auto receiver = reinterpret_cast<receiver::ublox::ThreadedReceiver*>(userdata);
    if (!receiver) return false;

    auto nav_pvt = receiver->nav_pvt();
    if (!nav_pvt) return false;

    // TODO(ewasjon): Should we use the system time if the UTC time from u-blox is invalid?
    if (!nav_pvt->valid_time()) {
        printf("u-blox time is invalid\n");
        return false;
    }

    location.tai_time                  = nav_pvt->tai_time();
    location.latitude                  = nav_pvt->latitude();
    location.longitude                 = nav_pvt->longitude();
    location.altitude                  = nav_pvt->altitude();
    location.horizontal_accuracy       = nav_pvt->h_acc();
    location.horizontal_speed          = nav_pvt->h_vel();
    location.horizontal_speed_accuracy = nav_pvt->h_vel_acc();
    location.bearing                   = nav_pvt->head_mot();
    location.vertical_accuracy         = nav_pvt->v_acc();
    location.vertical_speed            = fabs(nav_pvt->v_vel());
    location.vertical_speed_accuracy   = nav_pvt->v_vel_acc();
    location.vertical_velocity_direction =
        nav_pvt->v_vel() > 0 ? VerticalDirection::DOWN : VerticalDirection::UP;

    metrics.fixq = FixQuality::INVALID;
    if (nav_pvt->fix_type() == 3) {
        if (nav_pvt->carr_soln() == 2) {
            metrics.fixq = FixQuality::RTK_FIX;
        } else if (nav_pvt->carr_soln() == 1) {
            metrics.fixq = FixQuality::RTK_FLOAT;
        } else {
            metrics.fixq = FixQuality::STANDALONE;
        }
    } else if (nav_pvt->fix_type() == 2) {
        metrics.fixq = FixQuality::STANDALONE;
    } else if (nav_pvt->fix_type() == 1) {
        metrics.fixq = FixQuality::DEAD_RECKONING;
    }

    metrics.sats = nav_pvt->num_sv();
    metrics.age  = 0;  // TODO(ewasjon): requires another message
    metrics.hdop = 0;
    metrics.pdop = nav_pvt->p_dop();
    return true;
}

bool provide_location_information_callback_nmea(LocationInformation& location,
                                                HaGnssMetrics& metrics, void* userdata) {
    auto receiver = reinterpret_cast<receiver::nmea::ThreadedReceiver*>(userdata);
    if (!receiver) return false;

    // NOTE(ewasjon): We require GGA, VTG, and GST message to produce a valid location information,
    // if either is missing we will skip sending a provider location information message.
    auto gga = receiver->gga();
    auto vtg = receiver->vtg();
    auto gst = receiver->gst();
    if (!gga || !vtg || !gst) {
        return false;
    }

    location.tai_time                  = gga->time_of_day();
    location.latitude                  = gga->latitude();
    location.longitude                 = gga->longitude();
    location.altitude                  = gga->altitude();
    location.horizontal_accuracy       = gst->horizontal_position_error();
    location.horizontal_speed          = vtg->speed_over_ground();
    location.horizontal_speed_accuracy = 0;
    location.bearing                   = vtg->true_course_over_ground();

    // TODO(ewasjon): Are these not available in NMEA?
    location.vertical_accuracy           = gst->vertical_position_error();
    location.vertical_speed              = 0;
    location.vertical_speed_accuracy     = 0;
    location.vertical_velocity_direction = VerticalDirection::UP;

    switch (gga->fix_quality()) {
    case receiver::nmea::GgaFixQuality::Invalid: metrics.fixq = FixQuality::INVALID; break;
    case receiver::nmea::GgaFixQuality::GpsFix: metrics.fixq = FixQuality::STANDALONE; break;
    case receiver::nmea::GgaFixQuality::DgpsFix: metrics.fixq = FixQuality::DGPS_FIX; break;
    case receiver::nmea::GgaFixQuality::PpsFix: metrics.fixq = FixQuality::PPS_FIX; break;
    case receiver::nmea::GgaFixQuality::RtkFixed: metrics.fixq = FixQuality::RTK_FIX; break;
    case receiver::nmea::GgaFixQuality::RtkFloat: metrics.fixq = FixQuality::RTK_FLOAT; break;
    case receiver::nmea::GgaFixQuality::DeadReckoning:
        metrics.fixq = FixQuality::DEAD_RECKONING;
        break;
    default: metrics.fixq = FixQuality::INVALID;
    }

    metrics.sats = static_cast<u8>(gga->satellites_in_view());
    metrics.age  = 0;  // TODO: ?
    metrics.hdop = gga->h_dop();
    metrics.pdop = 0;
    return true;
}

bool provide_location_information_callback_fake(UNUSED LocationInformation& location,
                                                UNUSED HaGnssMetrics& metrics, void* userdata) {
    auto options = reinterpret_cast<LocationInformationOptions*>(userdata);
    if (!options) return false;

    location.tai_time  = TAI_Time::now();
    location.latitude  = options->latitude;
    location.longitude = options->longitude;
    location.altitude  = options->altitude;

    return true;
}

bool provide_ecid_callback(ECIDInformation& ecid, void* userdata) {
    auto modem = reinterpret_cast<Modem_AT*>(userdata);
    if (!modem) return false;

    auto neighbors = modem->neighbor_cells();
    auto cell      = modem->cell();
    if (!cell.initialized()) return false;

    ecid.cell           = cell.value();
    ecid.neighbor_count = 0;

    for (auto& neighbor_cell : neighbors) {
        if (ecid.neighbor_count < 16) {
            ecid.neighbors[ecid.neighbor_count++] = {
                .id     = neighbor_cell.id,
                .earfcn = neighbor_cell.EARFCN,
                .rsrp   = neighbor_cell.rsrp,
                .rsrq   = neighbor_cell.rsrq,
            };
        }
    }

    return true;
}
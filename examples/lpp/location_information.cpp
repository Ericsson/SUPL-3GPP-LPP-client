#include "location_information.h"
#include <cmath>
#include <modem.h>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/threaded_receiver.hpp>
#include <receiver/ublox/ubx_nav_pvt.hpp>
#include "lpp/location_information.h"
#include "utility/types.h"

bool provide_location_information_callback(UNUSED LocationInformation& location,
                                           UNUSED HaGnssMetrics& metrics, UNUSED void* userdata) {
#if 0
    // Example implementation
    location.time      = time(NULL);
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
                                                 UNUSED HaGnssMetrics&       metrics,
                                                 UNUSED void*                userdata) {
    auto receiver = reinterpret_cast<receiver::ublox::ThreadedReceiver*>(userdata);
    printf("provide_location_information_callback_ublox: %p (%p)\n", receiver, userdata);
    if (!receiver) return false;

    auto nav_pvt = receiver->nav_pvt();
    prinf("nav_pvt: %p\n", nav_pvt);
    if(!nav_pvt) return false;

    location.time                      = time(NULL);  // TODO(ewasjon): use time from nav_pvt
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
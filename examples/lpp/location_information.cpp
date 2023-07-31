#include "location_information.h"
#include <modem.h>
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
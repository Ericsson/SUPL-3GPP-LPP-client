#pragma once
#include <ctime>
#include <lpp/cell_id.h>
#include <utility/time.h>

enum class VerticalDirection { UP, DOWN };

struct LocationInformation {
    // Time of the location estimate in TAI.
    TAI_Time tai_time;

    // Latitude represented as a decimal degree value in the interval [-90,90] with positive values
    // north of the equator. E.g u-blox F9P UBX-NAV-PVT attribute lat.
    double latitude;
    // Longitude represented as a decimal degree value in the interval [-180,180] with positive
    // values east of the zero meridian. E.g u-blox F9P UBX-NAV-PVT attribute lon.
    double longitude;
    // Decimal number representing altitude above the WGS84 ellipsoid in meters. E.g u-blox F9P
    // UBX-NAV-PVT attribute height.
    double altitude;

    // Horizontal position uncertainty as stddev in meters. E.g u-blox F9P UBX-NAV-PVT attribute
    // hAcc.
    double horizontal_accuracy;
    // Horizontal velocity in m/s. E.g u-blox F9P UBX-NAV-PVT attribute gSpeed, converted from mm/s
    // to m/s
    double horizontal_speed;
    // Horizontal velocity uncertainty in m/s. E.g u-blox F9P UBX-NAV-PVT attribute sAcc, converted
    // from mm/s to m/s
    double horizontal_speed_accuracy;
    // Heading in decimal degrees. E.g u-blox F9P UBX-NAV-PVT attribute headMot.
    double bearing;

    // Vertical position uncertainty as stddev in meters. E.g u-blox F9P UBX-NAV-PVT attribute vAcc.
    double vertical_accuracy;
    // Vertical velocity in m/s. Is adopted from the velD attribute from the UBX-NAV-PVT message,
    // translated from mm/s to m/s.
    double vertical_speed;
    // Vertical velocity uncertainty in m/s. E.g u-blox F9P UBX-NAV-PVT attribute sAcc, converted
    // from mm/s to m/s
    double vertical_speed_accuracy;
    // Vertical velocity direction as enum. E.g sign of u-blox F9P UBX-NAV-PVT attribute velD, with
    // VerticalDirection::DOWN encoded for a positive sign and VerticalDirection::UP for a negative
    // sign.
    VerticalDirection vertical_velocity_direction;
};

enum class FixQuality : unsigned char {
    // invalid, or no position available, E.g u-blox F9P UBX-NAV-PVT attribute fixType = 0.
    INVALID = 0,
    // Standalone GNSS without correction data, E.g u-blox F9P UBX-NAV-PVT attribute fixType = 2 or
    // 3 (and carrSoln = 0)
    STANDALONE = 1,
    // DGPS fix
    DGPS_FIX = 2,
    // PPS fix
    PPS_FIX = 3,
    // RTK fix. E.g u-blox F9P UBX-NAV-PVT attribute fixType = 3 and carrSoln = 2
    RTK_FIX = 4,
    // RTK float. E.g u-blox F9P UBX-NAV-PVT attribute fixType = 3 and carrSoln = 1
    RTK_FLOAT = 5,
    // Estimated fix (dead reckoning). E.g u-blox F9P UBX-NAV-PVT attribute fixType = 1
    DEAD_RECKONING = 6,
    // Manual input mode
    MANUAL_INPUT = 7,
    // Simulation mode
    SIMULATION = 8,
    // WAAS fix (not NMEA standard, but NovAtel receivers report this instead of a 2)
    WAAS_FIX = 9,
};

struct HaGnssMetrics {
    // GNSS RTK position fix information.
    FixQuality fixq;
    // Number of satellites used to determine the position. E.g u-blox F9P UBX-NAV-PVT attribute
    // numSV.
    unsigned char sats;
    // Age of most recently used assistance data E.g u-blox F9P UBX-NAV-PVT attribute age.
    double age;
    // Horizontal diluion of precision. Not available in u-blox F9P UBX-NAV-PVT attributes.
    double hdop;
    // Positioning diluion of precision. E.g u-blox F9P UBX-NAV-PVT attribute pDOP.
    double pdop;
};

struct ECIDInformation {
    struct Neighbor {
        long id;
        long earfcn;
        long rsrp;
        long rsrq;
    };

    CellID cell;

    int      neighbor_count;
    Neighbor neighbors[16];
};

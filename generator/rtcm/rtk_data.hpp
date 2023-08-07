#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "maybe.hpp"
#include "satellite_id.hpp"
#include "time/tai_time.hpp"
#include "types.hpp"

struct CommonObservationInfo {
    uint32_t reference_station_id;
    long     clock_steering;
    long     external_clock;
    long     smooth_indicator;
    long     smooth_interval;
};

struct ReferenceStationInfo {
    uint32_t      reference_station_id;
    double        x;
    double        y;
    double        z;
    Maybe<double> antenna_height;
    bool          is_physical_reference_station;
};

struct PhysicalReferenceStationInfo {
    uint32_t reference_station_id;
    double   x;
    double   y;
    double   z;
};

struct Signal {
    uint32_t      id;
    SatelliteId   satellite;
    Maybe<double> fine_phase_range;
    Maybe<double> fine_pseudo_range;
    Maybe<double> fine_phase_range_rate;
    Maybe<double> carrier_to_noise_ratio;
    Maybe<double> lock_time;
    bool          half_cycle_ambiguity;
};

struct Satellite {
    SatelliteId    id;
    Maybe<int32_t> integer_ms;
    Maybe<double>  rough_range;
    Maybe<double>  rough_phase_range_rate;
};

enum GenericGnssId {
    GPS,
    GLONASS,
    GALILEO,
    BEIDOU,
};

struct Observations {
    TAI_Time               time;
    std::vector<Signal>    signals;
    std::vector<Satellite> satellites;
};

class RtkData {
public:
    RtkData()  = default;
    ~RtkData() = default;

    std::unique_ptr<CommonObservationInfo>        common_observation_info;
    std::unique_ptr<ReferenceStationInfo>         reference_station_info;
    std::unique_ptr<PhysicalReferenceStationInfo> physical_reference_station_info;

    std::unordered_map<GenericGnssId, std::unique_ptr<Observations>> observations;

private:
};

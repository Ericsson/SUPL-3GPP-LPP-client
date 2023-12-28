#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include "maybe.hpp"
#include "satellite_id.hpp"
#include "signal_id.hpp"
#include "time/tai_time.hpp"
#include "types.hpp"

enum GenericGnssId {
    GPS,
    GLONASS,
    GALILEO,
    BEIDOU,
};

namespace std {
template <>
struct hash<GenericGnssId> {
    auto operator()(const GenericGnssId& id) const -> size_t { return hash<int>{}(id); }
};
}  // namespace std

namespace generator {
namespace rtcm {

struct CommonObservationInfo {
    uint32_t reference_station_id;
    long     clock_steering;
    long     external_clock;
    long     smooth_indicator;
    long     smooth_interval;
};

struct ReferenceStation {
    uint32_t      reference_station_id;
    double        x;
    double        y;
    double        z;
    Maybe<double> antenna_height;
    bool          is_physical_reference_station;
};

struct PhysicalReferenceStation {
    uint32_t reference_station_id;
    double   x;
    double   y;
    double   z;
};

struct Signal {
    SignalId      id;
    SatelliteId   satellite;
    Maybe<double> fine_phase_range;
    Maybe<double> fine_pseudo_range;
    Maybe<double> fine_phase_range_rate;
    Maybe<double> carrier_to_noise_ratio;
    Maybe<double> lock_time;
    bool          half_cycle_ambiguity;

    uint32_t lowest_msm_version() const;
};

struct Satellite {
    SatelliteId    id;
    Maybe<int32_t> integer_ms;
    Maybe<double>  rough_range;
    Maybe<double>  rough_phase_range_rate;

    Maybe<int32_t> frequency_channel;

    uint32_t lowest_msm_version() const;
};

struct Observations {
    ts::TAI_Time           time;
    std::vector<Signal>    signals;
    std::vector<Satellite> satellites;

    uint32_t lowest_msm_version() const;
};

struct BiasInformation {
    uint32_t      reference_station_id;
    uint8_t       mask;
    Maybe<double> l1_ca;
    Maybe<double> l1_p;
    Maybe<double> l2_ca;
    Maybe<double> l2_p;
    bool          indicator;
};

struct Residuals {
    struct Satellite {
        SatelliteId id;
        double      s_oc;
        double      s_od;
        double      s_oh;
        double      s_lc;
        double      s_ld;
    };

    ts::TAI_Time           time;
    uint32_t               reference_station_id;
    uint32_t               n_refs;
    std::vector<Satellite> satellites;
};

struct AuxiliaryInformation {
    struct Satellite {
        SatelliteId    id;
        Maybe<int32_t> frequency_channel;
    };

    std::unordered_map<SatelliteId, Satellite> satellites;
};

class RtkData {
public:
    RtkData()  = default;
    ~RtkData() = default;

    std::unique_ptr<CommonObservationInfo>    common_observation_info;
    std::unique_ptr<ReferenceStation>         reference_station;
    std::unique_ptr<PhysicalReferenceStation> physical_reference_station;

    std::unordered_map<GenericGnssId, std::unique_ptr<Observations>> observations;

    std::unique_ptr<BiasInformation>      bias_information;
    std::unique_ptr<Residuals>            gps_residuals;
    std::unique_ptr<Residuals>            glonass_residuals;
    std::unique_ptr<AuxiliaryInformation> auxiliary_information;

private:
};

}  // namespace rtcm
}  // namespace generator

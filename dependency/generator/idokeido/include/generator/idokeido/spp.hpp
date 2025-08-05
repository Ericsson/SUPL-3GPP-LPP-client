#pragma once
#include <core/core.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/klobuchar.hpp>
#include <generator/idokeido/satellite.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace idokeido {

struct SppConfiguration {
    RelativisticModel relativistic_model;
    IonosphericMode ionospheric_mode;
    WeightFunction  weight_function;
    EpochSelection  epoch_selection;

    struct {
        bool gps;
        bool glo;
        bool gal;
        bool bds;
    } gnss;

    double observation_window;

    double elevation_cutoff;
    double snr_cutoff;
    double outlier_cutoff;

    bool reject_cycle_slip;
    bool reject_halfcycle_slip;
    bool reject_outliers;
};

class EphemerisEngine;
class SppEngine {
public:
    struct Observation {
        ts::Tai     time;
        SatelliteId satellite_id;
        SignalId    signal_id;
        double      pseudo_range;
        double      carrier_phase;
        double      doppler;
        double      snr;
        double      lock_time;
    };

    SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine) NOEXCEPT;
    ~SppEngine();

    void klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT;

    /// Add an observation
    void observation(RawObservation const& observation) NOEXCEPT;

    /// Evaluate the SPP at next epoch
    NODISCARD Solution evaluate() NOEXCEPT;

    /// Evaluate the SPP at epoch
    NODISCARD Solution evaluate(ts::Tai time) NOEXCEPT;

protected:
    void select_best_observations(ts::Tai const& time);
    void compute_satellite_states(ts::Tai const& time);

    NODISCARD Satellite& find_satellite(SatelliteId id);

    void datatrace_report() NOEXCEPT;

private:
    SppConfiguration mConfiguration;
    EphemerisEngine& mEphemerisEngine;
    long             mFirstObservationId;
    long             mLastObservationId;
    long             mEpochObservationCount;
    double           mEpochTotalObservationTime;

    std::bitset<SATELLITE_ID_MAX> mSatelliteMask;
    std::bitset<SIGNAL_ID_MAX>    mObservationMask;

    std::array<Satellite, SATELLITE_ID_MAX> mSatelliteStates;
    std::array<Observation, SIGNAL_ID_MAX>  mObservationStates;

    KlobucharModelParameters mKlobucharModel;
    bool                     mKlobucharModelSet;
};

}  // namespace idokeido

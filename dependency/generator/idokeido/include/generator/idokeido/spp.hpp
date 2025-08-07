#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/klobuchar.hpp>
#include <generator/idokeido/satellite.hpp>
#include <generator/idokeido/correction.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace idokeido {

struct SppConfiguration {
    RelativisticModel relativistic_model;
    IonosphericMode   ionospheric_mode;
    WeightFunction    weight_function;
    EpochSelection    epoch_selection;

    struct {
        bool gps;
        bool glo;
        bool gal;
        bool bds;
    } gnss;

    Scalar observation_window;

    Scalar elevation_cutoff;
    Scalar snr_cutoff;
    Scalar outlier_cutoff;

    bool reject_cycle_slip;
    bool reject_halfcycle_slip;
    bool reject_outliers;
};

class EphemerisEngine;
class CorrectionCache;
class SppEngine {
public:
    struct Measurment {
        ts::Tai  time;
        SignalId signal_id;
        Scalar   pseudo_range;
        Scalar   carrier_phase;
        Scalar   doppler;
        Scalar   snr;
        Scalar   lock_time;
    };

    struct Observation {
        ts::Tai                                  time;
        SatelliteId                              satellite_id;
        size_t                                   measurement_count;
        std::bitset<SIGNAL_ABS_COUNT>            measurement_mask;
        std::array<Measurment, SIGNAL_ABS_COUNT> measurements;

        Vector3 position;
        Vector3 velocity;
        Scalar  clock_bias;
        Scalar  group_delay[SIGNAL_ABS_COUNT];

        Scalar azimuth;
        Scalar elevation;
        Scalar nadir;

        long selected0;
        long selected1;

        ephemeris::Ephemeris ephemeris;

        void add_measurement(Measurment const& measurment) NOEXCEPT {
            auto id = measurment.signal_id.absolute_id();
            if (id < 0 || id >= SIGNAL_ABS_COUNT) return;
            measurement_mask[id] = true;
            measurements[id]     = measurment;
            measurement_count += 1;
        }
    };

    SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine,
              CorrectionCache& correction_cache) NOEXCEPT;
    ~SppEngine();

    void klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT;

    /// Add an observation
    void add_measurement(RawMeasurement const& measurment) NOEXCEPT;

    /// Evaluate the SPP at next epoch
    NODISCARD Solution evaluate() NOEXCEPT;

    /// Evaluate the SPP at epoch
    NODISCARD Solution evaluate(ts::Tai time) NOEXCEPT;

protected:
    void select_best_observations(ts::Tai const& time);
    void compute_satellite_states(ts::Tai const& time);

    void datatrace_report() NOEXCEPT;

private:
    SppConfiguration mConfiguration;
    EphemerisEngine& mEphemerisEngine;
    CorrectionCache& mCorrectionCache;

    bool    mEpochFirstTimeSet;
    ts::Tai mEpochFirstTime;
    bool    mEpochLastTimeSet;
    ts::Tai mEpochLastTime;
    long    mEpochObservationCount;
    double  mEpochTotalObservationTime;

    std::bitset<SATELLITE_ID_MAX>             mObservationMask;
    std::array<Observation, SATELLITE_ID_MAX> mObservations;

    KlobucharModelParameters mKlobucharModel;
    bool                     mKlobucharModelSet;
};

}  // namespace idokeido

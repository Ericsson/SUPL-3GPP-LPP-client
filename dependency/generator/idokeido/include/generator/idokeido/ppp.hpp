#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/kalman.hpp>

#include <bitset>
#include <unordered_map>
#include <vector>

namespace idokeido {

class EphemerisEngine;
class CorrectionCache;

enum class PppMode {
    Static,
    Kinematic,
};

enum class PppIonosphereMode {
    None,
    Klobuchar,
    DualFrequency,
    Ssr,
    Estimated,
};

enum class PppTroposphereMode {
    None,
    Estimated,
};

struct PppConfiguration {
    PppMode            mode;
    PppIonosphereMode  ionosphere_mode;
    PppTroposphereMode troposphere_wet_mode;
    PppTroposphereMode troposphere_dry_mode;

    bool apply_phase_windup;
    bool apply_shapiro;

    RelativisticModel relativistic_model;
    WeightFunction    weight_function;
    EpochSelection    epoch_selection;

    struct {
        bool gps;
        bool glo;
        bool gal;
        bool bds;
    } gnss;

    Scalar elevation_cutoff;
    Scalar snr_cutoff;
    Scalar observation_window;

    bool reject_cycle_slip;
    bool reject_halfcycle_slip;
    bool use_carrier_phase;

    Scalar sigma_pseudorange;
    Scalar sigma_carrier_phase;

    Scalar process_noise_position;
    Scalar process_noise_clock;
    Scalar process_noise_ztd_wet;
    Scalar process_noise_ztd_dry;
    Scalar process_noise_iono;
};

struct PppSolution {
    enum class Status {
        None,
        Standard,
    };

    struct SatelliteDetail {
        SatelliteId id;
        Scalar      elevation;
        Scalar      azimuth;
        Scalar      residual_pseudorange;
        Scalar      residual_carrier_phase;
        Scalar      ambiguity;
        Scalar      ionosphere;
        Scalar      geometric_range;
        Scalar      satellite_clock;
        Scalar      troposphere_dry;
        Scalar      troposphere_wet;
        Scalar      phase_windup;
        Scalar      shapiro;
        Scalar      code_bias;
        Scalar      phase_bias;
    };

    ts::Tai time;
    Status  status;

    double latitude;
    double longitude;
    double altitude;

    Vector3 position_ecef;
    Scalar  receiver_clock;
    Scalar  ztd_wet;
    Scalar  ztd_dry;

    MatrixX covariance;

    std::vector<SatelliteDetail> satellites;
    size_t                       satellite_count;
};

class PppEngine {
public:
    struct Measurement {
        ts::Tai  time;
        SignalId signal_id;
        Scalar   pseudo_range;
        Scalar   carrier_phase;
        Scalar   doppler;
        Scalar   snr;
        Scalar   lock_time;
    };

    struct Observation {
        ts::Tai                                   time;
        SatelliteId                               satellite_id;
        size_t                                    measurement_count;
        std::bitset<SIGNAL_ABS_COUNT>             measurement_mask;
        std::array<Measurement, SIGNAL_ABS_COUNT> measurements;

        Vector3 true_position;
        Vector3 true_velocity;
        Scalar  true_clock_bias;

        Vector3 eph_position;
        Vector3 eph_velocity;
        Scalar  eph_clock_bias;
        Scalar  group_delay;

        Scalar azimuth;
        Scalar elevation;
        Scalar nadir;

        long selected0;
        long selected1;

        ephemeris::Ephemeris ephemeris;

        long   ambiguity_state_idx;
        long   iono_state_idx;
        Scalar prev_lock_time;

        void add_measurement(Measurement const& m) NOEXCEPT {
            auto id = m.signal_id.absolute_id();
            if (id < 0 || static_cast<size_t>(id) >= SIGNAL_ABS_COUNT) return;
            auto idx              = static_cast<size_t>(id);
            measurement_mask[idx] = true;
            measurements[idx]     = m;
            measurement_count += 1;
        }
    };

    PppEngine(PppConfiguration configuration, EphemerisEngine& ephemeris_engine,
              CorrectionCache& correction_cache) NOEXCEPT;
    ~PppEngine();

    void add_measurement(RawMeasurement const& measurement) NOEXCEPT;

    NODISCARD PppSolution evaluate() NOEXCEPT;
    NODISCARD PppSolution evaluate(ts::Tai time) NOEXCEPT;

    void seed(Solution const& spp) NOEXCEPT;

protected:
    void select_best_observations(ts::Tai const& time) NOEXCEPT;
    void compute_satellite_states(ts::Tai const& time) NOEXCEPT;
    void initialize_filter() NOEXCEPT;
    void predict_filter() NOEXCEPT;
    void manage_ambiguity_states(ts::Tai const& time) NOEXCEPT;

private:
    PppConfiguration mConfiguration;
    EphemerisEngine& mEphemerisEngine;
    CorrectionCache& mCorrectionCache;

    bool    mFilterInitialized;
    ts::Tai mLastEpochTime;
    bool    mLastEpochTimeSet;

    // Fixed state indices
    static constexpr long kIdxX   = 0;
    static constexpr long kIdxY   = 1;
    static constexpr long kIdxZ   = 2;
    static constexpr long kIdxClk = 3;
    long                  mIdxZtdDry;
    long                  mIdxZtdWet;
    long                  mBaseStateSize;

    KalmanFilter mFilter;

    bool    mEpochFirstTimeSet;
    ts::Tai mEpochFirstTime;
    bool    mEpochLastTimeSet;
    ts::Tai mEpochLastTime;
    long    mEpochObservationCount;
    double  mEpochTotalObservationTime;

    std::bitset<SATELLITE_ID_MAX>             mObservationMask;
    std::array<Observation, SATELLITE_ID_MAX> mObservations;
};

}  // namespace idokeido

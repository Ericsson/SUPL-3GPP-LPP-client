#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>
#include <generator/idokeido/correction.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/kalman.hpp>
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
    IonosphericMode   ionospheric_mode;
    TroposphericMode  tropospheric_mode;
    WeightFunction    weight_function;
    EpochSelection    epoch_selection;
    FilterMode        filter_mode;

    struct {
        bool gps;
        bool glo;
        bool gal;
        bool bds;
        bool qzs;
    } gnss;

    Scalar observation_window;

    Scalar elevation_cutoff;
    Scalar snr_cutoff;
    Scalar outlier_cutoff;

    // Variance weight model: σ = sigma_a + sigma_b / sin(el), weight = 1/σ²
    Scalar sigma_a;
    Scalar sigma_b;

    bool reject_cycle_slip;
    bool reject_halfcycle_slip;
    bool reject_outliers;

    struct {
        Scalar position_horizontal;  // m²/s — Q for East/North axes
        Scalar position_vertical;    // m²/s — Q for Up axis
        Scalar velocity;             // m²/s — KinematicVelocity only
        Scalar clock;                // m²/s — receiver clock (always large)
    } process_noise;
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

        Vector3 eph_position;
        Vector3 eph_velocity;
        Scalar  eph_clock_bias;
        Scalar  group_delay[SIGNAL_ABS_COUNT];

        Scalar azimuth;
        Scalar elevation;
        Scalar nadir;

        Vector3 true_position;
        Vector3 true_velocity;
        Scalar  true_clock_bias;

        long selected0;
        long selected1;

        ephemeris::Ephemeris ephemeris;

        void add_measurement(Measurment const& measurment) NOEXCEPT {
            auto id = measurment.signal_id.absolute_id();
            if (id < 0 || static_cast<size_t>(id) >= SIGNAL_ABS_COUNT) return;
            auto idx              = static_cast<size_t>(id);
            measurement_mask[idx] = true;
            measurements[idx]     = measurment;
            measurement_count += 1;
        }
    };

    SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine,
              CorrectionCache& correction_cache) NOEXCEPT;
    ~SppEngine();

    void klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT;
    void bds_klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT;

    /// Add an observation
    void add_measurement(RawMeasurement const& measurment) NOEXCEPT;

    /// Evaluate the SPP at next epoch
    NODISCARD Solution evaluate() NOEXCEPT;

    /// Evaluate the SPP at epoch
    NODISCARD Solution evaluate(ts::Tai time) NOEXCEPT;

protected:
    void select_best_observations(ts::Tai const& time);
    void compute_satellite_states(ts::Tai const& time);
    void initialize_filter(VectorX const& ls_solution) NOEXCEPT;
    void predict_filter(ts::Tai const& time) NOEXCEPT;

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
    KlobucharModelParameters mBdsKlobucharModel;
    bool                     mBdsKlobucharModelSet;

    // Kalman filter (FilterMode != None)
    KalmanFilter mFilter;
    bool         mFilterInitialized;
    ts::Tai      mLastEpochTime;
    bool         mLastEpochTimeSet;

    // State indices for KinematicVelocity
    static constexpr long kIdxX      = 0;
    static constexpr long kIdxY      = 1;
    static constexpr long kIdxZ      = 2;
    static constexpr long kIdxClk    = 3;
    static constexpr long kIdxVx     = 4;
    static constexpr long kIdxVy     = 5;
    static constexpr long kIdxVz     = 6;
    static constexpr long kIdxClkDot = 7;

    // ISB state indices (-1 = reference or disabled)
    // Reference constellation has no ISB state (its clock IS clk at kIdxClk)
    long mIsbGps;  // -1 if GPS is reference or disabled
    long mIsbGal;
    long mIsbGlo;
    long mIsbBds;
    long mIsbQzs;
    long mBaseStates;  // 4 (or 8 for KinematicVelocity) before ISB

    // Returns the ISB state index for a satellite (-1 = use reference clock)
    long isb_index(SatelliteId const& id) const NOEXCEPT {
        if (id.is_gps()) return mIsbGps;
        if (id.is_galileo()) return mIsbGal;
        if (id.is_glonass()) return mIsbGlo;
        if (id.is_beidou()) return mIsbBds;
        if (id.is_qzss()) return mIsbQzs;
        return -1;
    }
};

}  // namespace idokeido

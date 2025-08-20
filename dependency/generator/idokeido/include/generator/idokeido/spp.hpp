#pragma once
#include <core/core.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <Eigen/Eigen>
#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <time/tai.hpp>

namespace idokeido {

struct RawObservation {
    ts::Tai time;

    SatelliteId satellite_id;
    SignalId    signal_id;

    double pseudo_range;   // meters
    double carrier_phase;  // cycles
    double doppler;        // Hz
    double snr;            // dBHz
    double lock_time;      // seconds
};

class EphemerisEngine {
public:
    EphemerisEngine()  = default;
    ~EphemerisEngine() = default;

    void add(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void add(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void add(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    struct Satellite {
        SatelliteId     id;
        Eigen::Vector3d position;
        Eigen::Vector3d velocity;
        double          clock;

        bool is_valid() const NOEXCEPT { return id.is_valid(); }
    };

    Satellite evaluate(SatelliteId satellite_id, ts::Tai const& time) const NOEXCEPT;

protected:
    ephemeris::GpsEphemeris const* find_gps(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;
    ephemeris::GalEphemeris const* find_gal(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;
    ephemeris::BdsEphemeris const* find_bds(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;

    Satellite evaluate_gps(SatelliteId satellite_id, ts::Tai const& time,
                           ephemeris::GpsEphemeris const& eph) const NOEXCEPT;
    Satellite evaluate_gal(SatelliteId satellite_id, ts::Tai const& time,
                           ephemeris::GalEphemeris const& eph) const NOEXCEPT;
    Satellite evaluate_bds(SatelliteId satellite_id, ts::Tai const& time,
                           ephemeris::BdsEphemeris const& eph) const NOEXCEPT;

private:
    std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> mGpsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> mGalEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> mBdsEphemeris;
};

struct SppConfiguration {
    enum WeightFunction {
        Uniform,
        Snr,
        Elevation,
    };

    enum class FrequencyMode { Single, Dual };

    FrequencyMode  frequency_mode;
    WeightFunction weight_function;

    struct {
        bool gps;
        bool glo;
        bool gal;
        bool bds;
    } gnss;

    double elevation_cutoff;
    double snr_cutoff;
    double outlier_cutoff;

    bool reject_cycle_slip;
    bool reject_halfcycle_slip;
    bool reject_outliers;
};

class SppEngine {
public:
    struct Satellite {
        ts::Tai         time;
        SatelliteId     id;
        Eigen::Vector3d position;
        long            main_observation_id;
    };

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

    /// Add an observation
    void observation(RawObservation const& observation) NOEXCEPT;

    /// Evaluate the SPP epoch
    void evaluate() NOEXCEPT;

protected:
    void group_by_satellite();
    void select_best_observations();
    void compute_satellite_states();

    NODISCARD Satellite& find_satellite(SatelliteId id);

private:
    SppConfiguration mConfiguration;
    EphemerisEngine& mEphemerisEngine;

    std::bitset<SATELLITE_ID_MAX> mSatelliteMask;
    std::bitset<SIGNAL_ID_MAX>    mObservationMask;

    std::array<Satellite, SATELLITE_ID_MAX> mSatelliteStates;
    std::array<Observation, SIGNAL_ID_MAX>  mObservationStates;
};

}  // namespace idokeido

#pragma once
#include <core/core.hpp>
#include <generator/idokeido/idokeido.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <time/tai.hpp>

namespace idokeido {

class EphemerisEngine {
public:
    EphemerisEngine()  = default;
    ~EphemerisEngine() = default;

    void load_or_create_cache(std::string const& file_path) NOEXCEPT;
    void save_cache() NOEXCEPT;

    void add(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void add(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void add(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    struct Satellite {
        SatelliteId     id;
        Eigen::Vector3d position;
        Eigen::Vector3d velocity;
        double          clock;
        double          group_delay;

        bool is_valid() const NOEXCEPT { return id.is_valid(); }
    };

    bool evaluate(SatelliteId satellite_id, ts::Tai const& time,
                  RelativisticModel relativistic_model, Satellite& result) const NOEXCEPT;

    bool clock_bias(SatelliteId satellite_id, ts::Tai const& time,
                    Scalar& clock_bias) const NOEXCEPT;

    bool find(SatelliteId satellite_id, ts::Tai const& time,
              ephemeris::Ephemeris& ephemeris) const NOEXCEPT;

protected:
    ephemeris::GpsEphemeris const* find_gps(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;
    ephemeris::GalEphemeris const* find_gal(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;
    ephemeris::BdsEphemeris const* find_bds(SatelliteId    satellite_id,
                                            ts::Tai const& time) const NOEXCEPT;

    Satellite evaluate_gps(SatelliteId satellite_id, ts::Tai const& time,
                           RelativisticModel              relativistic_model,
                           ephemeris::GpsEphemeris const& eph) const NOEXCEPT;
    Satellite evaluate_gal(SatelliteId satellite_id, ts::Tai const& time,
                           RelativisticModel              relativistic_model,
                           ephemeris::GalEphemeris const& eph) const NOEXCEPT;
    Satellite evaluate_bds(SatelliteId satellite_id, ts::Tai const& time,
                           RelativisticModel              relativistic_model,
                           ephemeris::BdsEphemeris const& eph) const NOEXCEPT;

private:
    std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> mGpsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> mGalEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> mBdsEphemeris;
    std::unique_ptr<std::string>                                          mCacheFile;
};

}  // namespace idokeido

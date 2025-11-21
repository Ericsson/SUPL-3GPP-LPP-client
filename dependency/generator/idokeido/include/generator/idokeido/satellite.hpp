#pragma once
#include <core/core.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/klobuchar.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ephemeris/ephemeris.hpp>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include <time/tai.hpp>

namespace idokeido {

class EphemerisEngine;
struct SatellitePosition {
    SatelliteId id;
    ts::Tai     receive_time;
    ts::Tai     transmit_time;

    Vector3 eph_position;
    Vector3 eph_velocity;
    Scalar  eph_clock_bias;
    Scalar  group_delay;

    Vector3 true_position;
    Vector3 true_velocity;
    Scalar  true_clock_bias;
};

struct OrbitCorrection;

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        EphemerisEngine const& ephemeris, RelativisticModel relativistic_model,
                        SatellitePosition& result) NOEXCEPT;

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        ephemeris::Ephemeris const& ephemeris, RelativisticModel relativistic_model,
                        OrbitCorrection const* orbit_correction,
                        SatellitePosition&     result) NOEXCEPT;

}  // namespace idokeido

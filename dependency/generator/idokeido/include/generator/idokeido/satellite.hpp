#pragma once
#include <core/core.hpp>
#include <generator/idokeido/idokeido.hpp>
#include <generator/idokeido/klobuchar.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <time/tai.hpp>
#include <ephemeris/ephemeris.hpp>

namespace idokeido {

class EphemerisEngine;
struct SatellitePosition {
    SatelliteId id;
    ts::Tai     receive_time;
    ts::Tai     transmit_time;

    Vector3 position;
    Vector3 velocity;
    Scalar  clock_bias;
    Scalar  group_delay;
};

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        EphemerisEngine const& ephemeris, RelativisticModel relativistic_model,
                        SatellitePosition& result) NOEXCEPT;

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        ephemeris::Ephemeris const& ephemeris, RelativisticModel relativistic_model,
                        SatellitePosition& result) NOEXCEPT;

}  // namespace idokeido

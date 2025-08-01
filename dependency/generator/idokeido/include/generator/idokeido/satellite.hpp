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

namespace idokeido {

struct Satellite {
    SatelliteId id;
    ts::Tai     receive_time;
    ts::Tai     transmit_time;

    Vector3     position;
    Vector3     velocity;

    Scalar      clock_bias;
    Scalar      group_delay;

    Scalar      azimuth;
    Scalar      elevation;
    Scalar      nadir;

    void compute_position_and_velocity() NOEXCEPT;
};


}  // namespace idokeido

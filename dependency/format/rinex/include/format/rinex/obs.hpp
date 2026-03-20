#pragma once
#include <core/core.hpp>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include <time/tai.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace format {
namespace rinex {

struct ObsEpoch {
    ts::Tai time;

    struct Measurement {
        SatelliteId satellite_id;
        SignalId    signal_id;
        double      pseudorange;    // meters (0 if missing)
        double      carrier_phase;  // cycles (0 if missing)
        double      doppler;        // Hz (0 if missing)
        double      snr;            // dBHz (0 if missing)
        bool        loss_of_lock;
    };

    std::vector<Measurement> measurements;
};

// Calls callback once per epoch. Returns false on file open error.
bool parse_obs(std::string const&                          path,
               std::function<void(ObsEpoch const&)> const& callback) NOEXCEPT;

}  // namespace rinex
}  // namespace format

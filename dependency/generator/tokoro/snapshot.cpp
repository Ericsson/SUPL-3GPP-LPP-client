#include "generator.hpp"
#include "observation.hpp"
#include "satellite.hpp"

#include <generator/tokoro/snapshot.hpp>

namespace generator {
namespace tokoro {

void extract_observations(std::shared_ptr<ReferenceStation> const& station,
                          SnapshotOutput&                          output) {
    output.observations.clear();

    for (auto const& sat : station->satellites()) {
        for (auto const& obs : sat.observations()) {
            if (!obs.is_valid()) continue;

            SnapshotObservation so;
            so.gnss          = static_cast<int32_t>(obs.sv_id().gnss());
            so.prn           = static_cast<int32_t>(obs.sv_id().prn().value);
            so.signal        = static_cast<int32_t>(obs.signal_id().lpp_id());
            so.pseudorange   = obs.code_range();
            so.carrier_phase = obs.phase_range();
            so.doppler       = obs.phase_range_rate();
            so.cnr           = obs.carrier_to_noise_ratio();
            so.elevation     = sat.elevation();
            so.azimuth       = sat.current_state().true_azimuth;

            output.observations.push_back(so);
        }
    }
}

}  // namespace tokoro
}  // namespace generator

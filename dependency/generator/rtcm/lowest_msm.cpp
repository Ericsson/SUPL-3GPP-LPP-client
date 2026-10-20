#include "constant.hpp"
#include "messages/msm.hpp"
#include "rtk_data.hpp"

#include <cmath>

namespace generator {
namespace rtcm {

// MSM versions form a 2x2 capability matrix, not a total order:
//   - resolution:   {4,5} low-resolution (DF400/401/402/403) vs {6,7} high-resolution
//                   (DF405/406/407/408)
//   - extended info: {4,6} no rate/extended satellite info vs {5,7} rate + extended info
// "Lowest usable version" must therefore be derived by independently determining whether
// high resolution and/or extended info/rate are required, then mapping that pair to the
// single MSM version that satisfies both - never by taking the numeric max of two
// requirements that were each only evaluated against one axis.
uint32_t msm_version_from_requirements(bool needs_high_resolution, bool needs_extended) {
    if (needs_high_resolution) {
        return needs_extended ? 7 : 6;
    }
    return needs_extended ? 5 : 4;
}

static bool signal_requires_high_resolution(Signal const& signal) {
#define ROUND(x) static_cast<long>(round(x))
    // fine pseudo range: DF405 vs DF400
    auto df400_encoded = ROUND(signal.fine_pseudo_range.value / RTCM_N2_24);
    auto df400_decoded = static_cast<double>(df400_encoded) * RTCM_N2_24;
    auto df405_encoded = ROUND(signal.fine_pseudo_range.value / RTCM_N2_29);
    auto df405_decoded = static_cast<double>(df405_encoded) * RTCM_N2_29;
    if (fabs(df400_decoded - df405_decoded) > 0.0) return true;

    // fine phase range: DF406 vs DF401
    auto df401_encoded = ROUND(signal.fine_phase_range.value / RTCM_N2_29);
    auto df401_decoded = static_cast<double>(df401_encoded) * RTCM_N2_29;
    auto df406_encoded = ROUND(signal.fine_phase_range.value / RTCM_N2_31);
    auto df406_decoded = static_cast<double>(df406_encoded) * RTCM_N2_31;
    if (fabs(df401_decoded - df406_decoded) > 0.0) return true;

    // lock time indicator: DF407 vs DF402
    auto df402_encoded = to_msm_lock(signal.lock_time.value);
    auto df402_decoded = from_msm_lock(df402_encoded);
    if (fabs(df402_decoded - signal.lock_time.value) > 0.0001) return true;

    if (signal.carrier_to_noise_ratio.valid) {
        // carrier to noise ratio: DF408 vs DF403
        auto df403_encoded = ROUND(signal.carrier_to_noise_ratio.value);
        auto df403_decoded = static_cast<double>(df403_encoded);
        auto df408_encoded = ROUND(signal.carrier_to_noise_ratio.value / 0.0625);
        auto df408_decoded = static_cast<double>(df408_encoded) * 0.0625;
        if (fabs(df403_decoded - df408_decoded) > 0.0001) return true;
    }
#undef ROUND

    return false;
}

uint32_t Signal::lowest_msm_version() const {
    // RTCM cannot encode DF402 as missing
    if (!lock_time.valid) return 0;

    auto needs_high_resolution = signal_requires_high_resolution(*this);
    auto needs_extended        = fine_phase_range_rate.valid;
    return msm_version_from_requirements(needs_high_resolution, needs_extended);
}

uint32_t Satellite::lowest_msm_version() const {
    // RTCM cannot encode DF398 as missing
    if (!rough_range.valid) return 0;

    auto needs_extended = (id.gnss() == SatelliteId::Gnss::GLONASS && frequency_channel.valid) ||
                          rough_phase_range_rate.valid;
    return msm_version_from_requirements(/*needs_high_resolution=*/false, needs_extended);
}

Observations::MsmRequirements Observations::msm_requirements() const {
    MsmRequirements result{/*possible=*/true, /*needs_high_resolution=*/false,
                           /*needs_extended=*/false};

    for (auto& satellite : satellites) {
        if (!satellite.rough_range.valid) {
            result.possible = false;
            return result;
        }
        if (satellite.id.gnss() == SatelliteId::Gnss::GLONASS &&
            satellite.frequency_channel.valid) {
            result.needs_extended = true;
        }
        if (satellite.rough_phase_range_rate.valid) {
            result.needs_extended = true;
        }
    }

    for (auto& signal : signals) {
        if (!signal.lock_time.valid) {
            result.possible = false;
            return result;
        }
        if (signal_requires_high_resolution(signal)) {
            result.needs_high_resolution = true;
        }
        if (signal.fine_phase_range_rate.valid) {
            result.needs_extended = true;
        }
    }

    return result;
}

uint32_t Observations::lowest_msm_version() const {
    auto requirements = msm_requirements();
    if (!requirements.possible) return 0;
    return msm_version_from_requirements(requirements.needs_high_resolution,
                                         requirements.needs_extended);
}

}  // namespace rtcm
}  // namespace generator

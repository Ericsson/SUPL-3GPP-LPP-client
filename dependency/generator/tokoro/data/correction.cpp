#include "correction.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(tokoro, data, correction);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, correction)

namespace generator {
namespace tokoro {

OrbitCorrection const* CorrectionData::orbit_correction(SatelliteId id) const NOEXCEPT {
    auto it = mOrbit.find(id);
    if (it == mOrbit.end()) return nullptr;
    if (max_orbit_age > 0.0) {
        auto age = mLatestCorrectionTime - it->second.reference_time;
        if (age > max_orbit_age) {
            WARNF("orbit correction for %s is too old (%.1fs > %.1fs limit) - dropping", id.name(),
                  age, max_orbit_age);
            return nullptr;
        }
    }
    return &it->second;
}

ClockCorrection const* CorrectionData::clock_correction(SatelliteId id) const NOEXCEPT {
    auto it = mClock.find(id);
    if (it == mClock.end()) return nullptr;
    if (max_clock_age > 0.0) {
        auto age = mLatestCorrectionTime - it->second.reference_time;
        if (age > max_clock_age) {
            WARNF("clock correction for %s is too old (%.1fs > %.1fs limit) - dropping", id.name(),
                  age, max_clock_age);
            return nullptr;
        }
    }
    return &it->second;
}

SignalCorrection const* CorrectionData::signal_corrections(SatelliteId id) const NOEXCEPT {
    auto it = mSignal.find(id);
    if (it == mSignal.end()) return nullptr;
    return &it->second;
}

template <typename T>
static bool bias_is_fresh(char const* kind, SatelliteId id, SignalId signal, T const& entry,
                          ts::Tai const& latest, double max_age) {
    if (max_age <= 0.0) return true;
    auto age = latest - entry.epoch_time;
    if (age > max_age) {
        WARNF("%s bias for %s %s is too old (%.1fs > %.1fs limit) - dropping", kind, id.name(),
              signal.name(), age, max_age);
        return false;
    }
    return true;
}

CodeBiasCorrection const* CorrectionData::code_bias(SatelliteId id,
                                                    SignalId    signal) const NOEXCEPT {
    auto it = mSignal.find(id);
    if (it == mSignal.end()) return nullptr;
    auto bias = it->second.code_bias.find(signal);
    if (bias == it->second.code_bias.end()) return nullptr;
    if (!bias_is_fresh("code", id, signal, bias->second, mLatestCorrectionTime, max_bias_age))
        return nullptr;
    return &bias->second;
}

PhaseBiasCorrection const* CorrectionData::phase_bias(SatelliteId id,
                                                      SignalId    signal) const NOEXCEPT {
    auto it = mSignal.find(id);
    if (it == mSignal.end()) return nullptr;
    auto bias = it->second.phase_bias.find(signal);
    if (bias == it->second.phase_bias.end()) return nullptr;
    if (!bias_is_fresh("phase", id, signal, bias->second, mLatestCorrectionTime, max_bias_age))
        return nullptr;
    return &bias->second;
}

}  // namespace tokoro
}  // namespace generator

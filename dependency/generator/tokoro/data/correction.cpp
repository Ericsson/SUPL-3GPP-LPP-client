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
    if (max_bias_age > 0.0) {
        // Code bias and phase bias share the same epoch per satellite message; check either.
        // Prefer code bias entry; fall back to phase bias if code bias is absent.
        bool checked = false;
        for (auto const& cb : it->second.code_bias) {
            auto age = mLatestCorrectionTime - cb.second.epoch_time;
            if (age > max_bias_age) {
                WARNF("bias correction for %s is too old (%.1fs > %.1fs limit) - dropping",
                      id.name(), age, max_bias_age);
                return nullptr;
            }
            checked = true;
            break;
        }
        if (!checked) {
            for (auto const& pb : it->second.phase_bias) {
                auto age = mLatestCorrectionTime - pb.second.epoch_time;
                if (age > max_bias_age) {
                    WARNF("bias correction for %s is too old (%.1fs > %.1fs limit) - dropping",
                          id.name(), age, max_bias_age);
                    return nullptr;
                }
                break;
            }
        }
    }
    return &it->second;
}

}  // namespace tokoro
}  // namespace generator

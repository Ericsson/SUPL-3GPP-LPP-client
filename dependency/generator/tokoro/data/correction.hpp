#pragma once
#include <core/core.hpp>

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "sv_id.hpp"

#include "clock.hpp"
#include "grid.hpp"
#include "ionosphere.hpp"
#include "orbit.hpp"
#include "set.hpp"
#include "signal.hpp"
#include "troposphere.hpp"

namespace generator {
namespace tokoro {

struct CorrectionData {
    ts::Tai const& latest_correction_time() const { return mLatestCorrectionTime; }

    SignalCorrection const* signal_corrections(SatelliteId id) const {
        auto it = mSignal.find(id);
        if (it != mSignal.end()) return &it->second;
        return nullptr;
    }

    OrbitCorrection const* orbit_correction(SatelliteId id) const {
        auto it = mOrbit.find(id);
        if (it != mOrbit.end()) return &it->second;
        return nullptr;
    }

    ClockCorrection const* clock_correction(SatelliteId id) const {
        auto it = mClock.find(id);
        if (it != mClock.end()) return &it->second;
        return nullptr;
    }

    bool tropospheric(SatelliteId sv_id, Float3 llh,
                      TroposphericCorrection& correction) const NOEXCEPT;
    bool ionospheric(SatelliteId sv_id, Float3 llh,
                     IonosphericCorrection& correction) const NOEXCEPT;

    void add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15 const* orbit) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15 const* clock) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16 const* code_bias) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16 const* stec,
                        CorrectionPointSet const& correction_point_set) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* grid,
                        CorrectionPointSet const& correction_point_set) NOEXCEPT;

    NODISCARD std::unordered_set<SignalId> const* signals(SatelliteId id) const {
        auto it = mSignals.find(id);
        if (it != mSignals.end()) return &it->second;
        return nullptr;
    }

    CorrectionPointSet const* mCorrectionPointSet;

private:
    ts::Tai mLatestCorrectionTime;

    std::unordered_map<SatelliteId, std::unordered_set<SignalId>> mSignals;

    std::unordered_map<SatelliteId, OrbitCorrection>  mOrbit;
    std::unordered_map<SatelliteId, ClockCorrection>  mClock;
    std::unordered_map<SatelliteId, SignalCorrection> mSignal;

    std::unordered_map<SatelliteId, IonosphericPolynomial> mIonosphericPolynomial;
    std::unordered_map<SatelliteId::Gnss, GridData>        mGrid;
};

}  // namespace tokoro
}  // namespace generator

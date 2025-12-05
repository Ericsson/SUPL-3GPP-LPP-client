#pragma once
#include <core/core.hpp>

#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>

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

#ifdef ENABLE_TOKORO_SNAPSHOT
struct SnapshotOrbitCorrection;
struct SnapshotClockCorrection;
struct SnapshotCodeBias;
struct SnapshotPhaseBias;
struct SnapshotGridData;
struct SnapshotIonosphericPolynomial;
#endif

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

#ifdef ENABLE_TOKORO_SNAPSHOT
    void snapshot(std::vector<SnapshotOrbitCorrection>&       orbit_corrections,
                  std::vector<SnapshotClockCorrection>&       clock_corrections,
                  std::vector<SnapshotCodeBias>&              code_biases,
                  std::vector<SnapshotPhaseBias>&             phase_biases,
                  std::vector<SnapshotIonosphericPolynomial>& ionospheric_polynomials,
                  std::vector<SnapshotGridData>&              grid_data) const NOEXCEPT;

    void load_snapshot(std::vector<SnapshotOrbitCorrection> const&       orbit_corrections,
                       std::vector<SnapshotClockCorrection> const&       clock_corrections,
                       std::vector<SnapshotCodeBias> const&              code_biases,
                       std::vector<SnapshotPhaseBias> const&             phase_biases,
                       std::vector<SnapshotIonosphericPolynomial> const& ionospheric_polynomials,
                       std::vector<SnapshotGridData> const&              grid_data) NOEXCEPT;
#endif

    CorrectionPointSet const* correction_point_set;

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

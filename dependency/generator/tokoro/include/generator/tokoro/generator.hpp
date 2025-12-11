#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ephemeris/bds.hpp>
#include <ephemeris/ephemeris.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#ifdef INCLUDE_FORMAT_ANTEX
#include <format/antex/antex.hpp>
#endif
#ifdef INCLUDE_FORMAT_RINEX
#include <format/rinex/builder.hpp>
#endif
#include <generator/tokoro/sv_id.hpp>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include <time/tai.hpp>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;

namespace generator {
namespace rtcm {
class Message;
struct Observations;
}  // namespace rtcm

namespace tokoro {

struct ReferenceStationConfig {
    Float3 itrf_ground_position;
    Float3 rtcm_ground_position;
    bool   generate_gps;
    bool   generate_glo;
    bool   generate_gal;
    bool   generate_bds;
    bool   generate_qzs;
};

struct CorrectionData;
struct CorrectionPointSet;
struct Satellite;
struct Observation;
struct RangeTimeDivision;
struct TokoroOutput;
#ifdef ENABLE_TOKORO_SNAPSHOT
struct SnapshotInput;
#endif
class Generator;
class ReferenceStation {
public:
    ReferenceStation(Generator& generator, ReferenceStationConfig const& config) NOEXCEPT;
    ~ReferenceStation();

    // Generate a new set of observations.
    bool generate(ts::Tai const& reception_time) NOEXCEPT;

    // Produce RTCM messages based on latest observations.
    std::vector<rtcm::Message> produce() NOEXCEPT;

#ifdef ENABLE_TOKORO_SNAPSHOT
    // Create a snapshot of current state for regression testing.
    SnapshotInput snapshot(ts::Tai const& time) const NOEXCEPT;
#endif

    void include_satellite(SatelliteId sv_id) NOEXCEPT { mSatelliteIncludeSet.insert(sv_id); }
    void include_signal(SignalId signal_id) NOEXCEPT { mSignalIncludeSet.insert(signal_id); }

    void set_physical_ground_position(Float3 position) NOEXCEPT {
        mRtcmPhysicalGroundPosition    = position;
        mRtcmPhysicalGroundPositionSet = true;
    }

    void set_shapiro_correction(bool enabled) { mShapiroCorrection = enabled; }
    void set_earth_solid_tides_correction(bool enabled) { mEarthSolidTidesCorrection = enabled; }
    void set_phase_windup_correction(bool enabled) { mPhaseWindupCorrection = enabled; }
    void set_antenna_phase_variation_correction(bool enabled) { mAntennaPhaseVariation = enabled; }
    void set_tropospheric_height_correction(bool enabled) { mTropoHeightCorrection = enabled; }
    void set_elevation_mask(double mask) { mElevationMask = mask; }
    void set_phase_range_rate(bool enabled) { mPhaseRangeRate = enabled; }
    void set_reference_station_id(uint32_t id) { mRtcmReferenceStationId = id; }
    void set_msm_type(uint32_t type) { mRtcmMsmType = type; }
    void set_negative_phase_windup(bool enabled) { mNegativePhaseWindup = enabled; }
#ifdef INCLUDE_FORMAT_RINEX
    void set_generate_rinex(bool enabled) { mGenerateRinex = enabled; }
#endif
    void set_require_code_bias(bool enabled) { mRequireCodeBias = enabled; }
    void set_require_phase_bias(bool enabled) { mRequirePhaseBias = enabled; }
    void set_require_tropo(bool enabled) { mRequireTropo = enabled; }
    void set_require_iono(bool enabled) { mRequireIono = enabled; }
    void set_use_tropospheric_model(bool enabled) { mUseTroposphericModel = enabled; }
    void set_use_ionospheric_height_correction(bool enabled) {
        mUseIonosphericHeightCorrection = enabled;
    }

    NODISCARD Float3 const& ground_position() const NOEXCEPT { return mGroundPosition; }
    NODISCARD Float3 const& rtcm_ground_position() const NOEXCEPT { return mRtcmGroundPosition; }
    NODISCARD bool          generate_gps() const NOEXCEPT { return mGenerateGps; }
    NODISCARD bool          generate_glo() const NOEXCEPT { return mGenerateGlo; }
    NODISCARD bool          generate_gal() const NOEXCEPT { return mGenerateGal; }
    NODISCARD bool          generate_bds() const NOEXCEPT { return mGenerateBds; }
    NODISCARD bool          generate_qzs() const NOEXCEPT { return mGenerateQzs; }
    NODISCARD std::vector<Satellite> const& satellites() const NOEXCEPT { return mSatellites; }

protected:
    void initialize_satellites() NOEXCEPT;
    void initialize_observation(Satellite& satellite, SignalId signal_id) NOEXCEPT;
    void build_rtcm_observation(Satellite const& satellite, Observation const& observation,
                                RangeTimeDivision const& rtd, double reference_phase_range_rate,
                                rtcm::Observations& observations) NOEXCEPT;
    void build_rtcm_satellite(Satellite const&    satellite,
                              rtcm::Observations& observations) NOEXCEPT;

private:
    Float3 mGroundPosition;
    Float3 mRtcmGroundPosition;
    Float3 mRtcmPhysicalGroundPosition;
    bool   mRtcmPhysicalGroundPositionSet;

    bool     mGenerateGps;
    bool     mGenerateGlo;
    bool     mGenerateGal;
    bool     mGenerateBds;
    bool     mGenerateQzs;
    bool     mShapiroCorrection;
    bool     mEarthSolidTidesCorrection;
    bool     mPhaseWindupCorrection;
    bool     mAntennaPhaseVariation;
    bool     mTropoHeightCorrection;
    double   mElevationMask;
    bool     mPhaseRangeRate;
    uint32_t mRtcmReferenceStationId;
    uint32_t mRtcmMsmType;
    bool     mNegativePhaseWindup;
#ifdef INCLUDE_FORMAT_RINEX
    bool mGenerateRinex;
#endif

    bool mRequireCodeBias;
    bool mRequirePhaseBias;
    bool mRequireTropo;
    bool mRequireIono;
    bool mUseTroposphericModel;
    bool mUseIonosphericHeightCorrection;

    std::vector<Satellite>          mSatellites;
    std::unordered_set<SatelliteId> mSatelliteIncludeSet;
    std::unordered_set<SignalId>    mSignalIncludeSet;

    std::unordered_map<SatelliteSignalId, ts::Tai> mLockTime;

    ts::Tai mGenerationTime;
#ifdef INCLUDE_FORMAT_RINEX
    ts::Tai mLastRinexEpoch;

    format::rinex::Builder mRinexBuilder;
#endif
    Generator& mGenerator;

    friend void extract_observations(std::shared_ptr<ReferenceStation> const&, TokoroOutput&);
};

class Generator {
public:
    Generator() NOEXCEPT;
    ~Generator();

    bool process_lpp(LPP_Message const& lpp_message) NOEXCEPT;
    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::QzsEphemeris const& ephemeris) NOEXCEPT;

    void set_iod_consistency_check(bool enabled) NOEXCEPT { mIodConsistencyCheck = enabled; }
    void set_rtoc(bool enabled) NOEXCEPT { mUseReceptionTimeForOrbitAndClockCorrections = enabled; }
    void set_ocit(bool enabled) NOEXCEPT { mUseOrbitCorrectionInIteration = enabled; }
    void set_ignore_bitmask(bool enabled) NOEXCEPT { mIgnoreBitmask = enabled; }
#ifdef INCLUDE_FORMAT_ANTEX
    void set_antex(std::unique_ptr<format::antex::Antex> antex) NOEXCEPT {
        mAntex = std::move(antex);
    }
#endif

    std::shared_ptr<ReferenceStation>
    define_reference_station(ReferenceStationConfig const& config) NOEXCEPT;

    NODISCARD ts::Tai const& last_correction_data_time() const NOEXCEPT {
        return mLastCorrectionDataTime;
    }

    std::vector<std::pair<SatelliteId, uint32_t>> missing_ephemeris() NOEXCEPT {
        return std::move(mMissingEphemeris);
    }

    CorrectionPointSet const* correction_point_set() const NOEXCEPT {
        return mCorrectionPointSet.get();
    }

    bool get_grid_position(int east, int north, double* lat, double* lon) const NOEXCEPT;
    bool get_grid_cell_center_position(int east, int north, double* lat,
                                       double* lon) const NOEXCEPT;

    NODISCARD std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> const&
              gps_ephemeris() const NOEXCEPT {
        return mGpsEphemeris;
    }
    NODISCARD std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> const&
              gal_ephemeris() const NOEXCEPT {
        return mGalEphemeris;
    }
    NODISCARD std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> const&
              bds_ephemeris() const NOEXCEPT {
        return mBdsEphemeris;
    }
    NODISCARD std::unordered_map<SatelliteId, std::vector<ephemeris::QzsEphemeris>> const&
              qzs_ephemeris() const NOEXCEPT {
        return mQzsEphemeris;
    }

#ifdef ENABLE_TOKORO_SNAPSHOT
    // Create a snapshot of ephemeris data for regression testing.
    SnapshotInput snapshot() const NOEXCEPT;

    // Load a snapshot and populate internal state.
    void load_snapshot(SnapshotInput const& input) NOEXCEPT;
#endif

private:
    void find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;
    void find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;

    bool find_ephemeris(SatelliteId sv_id, ts::Tai const& time, uint16_t iod,
                        ephemeris::Ephemeris& eph) const NOEXCEPT;
    bool compute_tropospheric_residual(double& residual) NOEXCEPT;

    std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> mGpsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> mGalEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> mBdsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::QzsEphemeris>> mQzsEphemeris;

    ts::Tai                             mLastCorrectionDataTime;
    std::unique_ptr<CorrectionData>     mCorrectionData;
    std::unique_ptr<CorrectionPointSet> mCorrectionPointSet;
#ifdef INCLUDE_FORMAT_ANTEX
    std::unique_ptr<format::antex::Antex> mAntex;
#endif

    bool mIodConsistencyCheck;
    bool mUseReceptionTimeForOrbitAndClockCorrections;
    bool mUseOrbitCorrectionInIteration;
    bool mIgnoreBitmask;

    mutable std::vector<std::pair<SatelliteId, uint32_t>> mMissingEphemeris;

    friend struct Satellite;
    friend class ReferenceStation;
};

}  // namespace tokoro
}  // namespace generator

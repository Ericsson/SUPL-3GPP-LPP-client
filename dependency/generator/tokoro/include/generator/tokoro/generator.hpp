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
#include <format/rinex/builder.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <generator/tokoro/sv_id.hpp>
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
};

struct CorrectionData;
struct CorrectionPointSet;
struct Satellite;
struct Observation;
struct RangeTimeDivision;
class Generator;
class ReferenceStation {
public:
    ReferenceStation(Generator& generator, ReferenceStationConfig const& config) NOEXCEPT;
    ~ReferenceStation() NOEXCEPT;

    // Generate a new set of observations.
    bool generate(ts::Tai const& reception_time) NOEXCEPT;

    // Produce RTCM messages based on latest observations.
    std::vector<rtcm::Message> produce() NOEXCEPT;

    void include_satellite(SatelliteId sv_id) NOEXCEPT { mSatelliteIncludeSet.insert(sv_id); }
    void include_signal(SignalId signal_id) NOEXCEPT { mSignalIncludeSet.insert(signal_id); }

    void set_physical_ground_position(Float3 position) NOEXCEPT {
        mRtcmPhysicalGroundPosition    = position;
        mRtcmPhysicalGroundPositionSet = true;
    }

    void set_shaprio_correction(bool enabled) { mShapiroCorrection = enabled; }
    void set_earth_solid_tides_correction(bool enabled) { mEarthSolidTidesCorrection = enabled; }
    void set_phase_windup_correction(bool enabled) { mPhaseWindupCorrection = enabled; }
    void set_antenna_phase_variation_correction(bool enabled) { mAntennaPhaseVariation = enabled; }
    void set_tropospheric_height_correction(bool enabled) { mTropoHeightCorrection = enabled; }
    void set_elevation_mask(double mask) { mElevationMask = mask; }
    void set_phase_range_rate(bool enabled) { mPhaseRangeRate = enabled; }
    void set_reference_station_id(uint32_t id) { mRtcmReferenceStationId = id; }
    void set_msm_type(uint32_t type) { mRtcmMsmType = type; }
    void set_negative_phase_windup(bool enabled) { mNegativePhaseWindup = enabled; }
    void set_generate_rinex(bool enabled) { mGenerateRinex = enabled; }
    void set_require_code_bias(bool enabled) { mRequireCodeBias = enabled; }
    void set_require_phase_bias(bool enabled) { mRequirePhaseBias = enabled; }
    void set_require_tropo(bool enabled) { mRequireTropo = enabled; }
    void set_require_iono(bool enabled) { mRequireIono = enabled; }
    void set_use_tropospheric_model(bool enabled) { mUseTroposphericModel = enabled; }
    void set_use_ionospheric_height_correction(bool enabled) {
        mUseIonosphericHeightCorrection = enabled;
    }

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
    bool     mGenerateRinex;

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
    ts::Tai mLastRinexEpoch;

    format::rinex::Builder mRinexBuilder;
    Generator&             mGenerator;
};

class Generator {
public:
    Generator() NOEXCEPT;
    ~Generator() NOEXCEPT;

    bool process_lpp(LPP_Message const& lpp_message) NOEXCEPT;
    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    void set_iod_consistency_check(bool enabled) NOEXCEPT { mIodConsistencyCheck = enabled; }
    void set_rtoc(bool enabled) NOEXCEPT { mUseReceptionTimeForOrbitAndClockCorrections = enabled; }
    void set_ocit(bool enabled) NOEXCEPT { mUseOrbitCorrectionInIteration = enabled; }

    std::shared_ptr<ReferenceStation>
    define_reference_station(ReferenceStationConfig const& config) NOEXCEPT;

    NODISCARD ts::Tai const& last_correction_data_time() const NOEXCEPT {
        return mLastCorrectionDataTime;
    }

private:
    void find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;
    void find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;

    bool find_ephemeris(SatelliteId sv_id, ts::Tai const& time, uint16_t iod,
                        ephemeris::Ephemeris& eph) const NOEXCEPT;
    bool compute_tropospheric_residual(double& residual) NOEXCEPT;

    std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> mGpsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> mGalEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> mBdsEphemeris;

    ts::Tai                             mLastCorrectionDataTime;
    std::unique_ptr<CorrectionData>     mCorrectionData;
    std::unique_ptr<CorrectionPointSet> mCorrectionPointSet;

    bool mIodConsistencyCheck;
    bool mUseReceptionTimeForOrbitAndClockCorrections;
    bool mUseOrbitCorrectionInIteration;

    friend struct Satellite;
    friend class ReferenceStation;
};

}  // namespace tokoro
}  // namespace generator

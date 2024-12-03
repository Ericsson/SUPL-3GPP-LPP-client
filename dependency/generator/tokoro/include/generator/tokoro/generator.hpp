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

    bool   mGenerateGps;
    bool   mGenerateGlo;
    bool   mGenerateGal;
    bool   mGenerateBds;
    bool   mShapiroCorrection;
    bool   mEarthSolidTidesCorrection;
    bool   mPhaseWindupCorrection;
    bool   mAntennaPhaseVariation;
    bool   mTropoHeightCorrection;
    double mElevationMask;

    std::vector<Satellite>          mSatellites;
    std::unordered_set<SatelliteId> mSatelliteIncludeSet;
    std::unordered_set<SignalId>    mSignalIncludeSet;

    ts::Tai mGenerationTime;

    Generator& mGenerator;
};

class Generator {
public:
    Generator() NOEXCEPT;
    ~Generator() NOEXCEPT;

    void process_lpp(LPP_Message const& lpp_message) NOEXCEPT;
    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    std::shared_ptr<ReferenceStation>
    define_reference_station(ReferenceStationConfig const& config) NOEXCEPT;

    NODISCARD ts::Tai const& last_correction_data_time() const NOEXCEPT {
        return mLastCorrectionDataTime;
    }

private:
    void find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;
    void find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;

    bool find_ephemeris(SatelliteId sv_id, ts::Tai const& time, uint16_t iode,
                        ephemeris::Ephemeris& eph) const NOEXCEPT;
    bool compute_tropospheric_residual(double& residual) NOEXCEPT;

    std::unordered_map<SatelliteId, std::vector<ephemeris::GpsEphemeris>> mGpsEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::GalEphemeris>> mGalEphemeris;
    std::unordered_map<SatelliteId, std::vector<ephemeris::BdsEphemeris>> mBdsEphemeris;

    ts::Tai                             mLastCorrectionDataTime;
    std::unique_ptr<CorrectionData>     mCorrectionData;
    std::unique_ptr<CorrectionPointSet> mCorrectionPointSet;

    friend struct Satellite;
    friend class ReferenceStation;
};

}  // namespace tokoro
}  // namespace generator

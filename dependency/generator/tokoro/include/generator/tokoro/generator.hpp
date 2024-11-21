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
#include <generator/tokoro/ecef.hpp>
#include <generator/tokoro/sv_id.hpp>
#include <generator/tokoro/wgs84.hpp>
#include <time/tai.hpp>

struct LPP_Message;
struct ProvideAssistanceData_r9_IEs;

namespace generator {
namespace rtcm {
class Message;
struct Observations;
}  // namespace rtcm

namespace tokoro {

struct CorrectionData;
struct CorrectionPointSet;
struct Satellite;
struct Observation;

class Generator {
public:
    Generator() NOEXCEPT;
    ~Generator() NOEXCEPT;

    void process_lpp(LPP_Message const& lpp_message) NOEXCEPT;
    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    void include_satellite(SatelliteId sv_id) NOEXCEPT { mSatelliteIncludeSet.insert(sv_id); }
    void include_signal(SignalId signal_id) NOEXCEPT { mSignalIncludeSet.insert(signal_id); }

    void set_gps_supported(bool supported) { mGenerateGps = supported; }
    void set_glonass_supported(bool supported) { mGenerateGlo = supported; }
    void set_galileo_supported(bool supported) { mGenerateGal = supported; }
    void set_beidou_supported(bool supported) { mGenerateBds = supported; }

    void set_shaprio_correction(bool enabled) { mShapiroCorrection = enabled; }
    void set_earth_solid_tides_correction(bool enabled) { mEarthSolidTidesCorrection = enabled; }
    void set_phase_windup_correction(bool enabled) { mPhaseWindupCorrection = enabled; }
    void set_antenna_phase_variation_correction(bool enabled) { mAntennaPhaseVariation = enabled; }

    void set_tropospheric_height_correction(bool enabled) { mTropoHeightCorrection = enabled; }

    void set_physical_reference_station(EcefPosition position) {
        mGeneratePhysicalReferenceStation = true;
        mPhysicalPosition = position;
    }

    std::vector<rtcm::Message> generate(ts::Tai const& reception_time,
                                        EcefPosition   reception_position) NOEXCEPT;

    NODISCARD ts::Tai const& last_correction_data_time() const NOEXCEPT {
        return mLastCorrectionDataTime;
    }

private:
    void find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;
    void find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;

    void create_satellites() NOEXCEPT;
    void create_satellite(SatelliteId sv_id) NOEXCEPT;

    bool find_ephemeris(SatelliteId sv_id, ephemeris::Ephemeris& eph) NOEXCEPT;

    bool compute_tropospheric_residual(double& residual) NOEXCEPT;

    bool generate_observation(Satellite const& satellite, SignalId signal_id) NOEXCEPT;
    void build_rtcm_satellite(Satellite const&    satellite,
                              rtcm::Observations& observations) NOEXCEPT;
    void build_rtcm_observations(SatelliteId::Gnss gnss, rtcm::Observations& observations) NOEXCEPT;
    void build_rtcm_messages(std::vector<rtcm::Message>& messages) NOEXCEPT;

    std::unordered_map<SatelliteId, ephemeris::GpsEphemeris> mGpsEphemeris;
    std::unordered_map<SatelliteId, ephemeris::GalEphemeris> mGalEphemeris;
    std::unordered_map<SatelliteId, ephemeris::BdsEphemeris> mBdsEphemeris;

    std::unordered_set<SatelliteId> mSatelliteIncludeSet;
    std::unordered_set<SignalId>    mSignalIncludeSet;

    ts::Tai                             mTimeReception;
    ts::Tai                             mLastCorrectionDataTime;
    EcefPosition                        mEcefLocation;
    Wgs84Position                       mWgs84Location;
    std::unique_ptr<CorrectionData>     mCorrectionData;
    std::unique_ptr<CorrectionPointSet> mCorrectionPointSet;

    bool mGenerateGps{true};
    bool mGenerateGlo{false};
    bool mGenerateGal{false};
    bool mGenerateBds{false};

    bool mShapiroCorrection{true};
    bool mEarthSolidTidesCorrection{false};
    bool mPhaseWindupCorrection{false};
    bool mAntennaPhaseVariation{false};

    bool mTropoHeightCorrection{true};

    bool mGeneratePhysicalReferenceStation{false};
    EcefPosition mPhysicalPosition;

    double mElevationMask{10.0};

    std::unordered_map<SatelliteId, std::unique_ptr<Satellite>> mSatellites;
    std::vector<Observation>                                    mObservations;
};

}  // namespace tokoro
}  // namespace generator

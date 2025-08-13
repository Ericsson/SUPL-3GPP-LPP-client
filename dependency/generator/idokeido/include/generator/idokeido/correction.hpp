
#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>
#include <generator/idokeido/idokeido.hpp>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct LPP_Message;
struct GNSS_SSR_PhaseBias_r16;
struct ProvideAssistanceData_r9_IEs;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;

namespace idokeido {

struct ClockCorrection {
    ts::Tai reference_time;
    Scalar  c0;
    Scalar  c1;
    Scalar  c2;

    NODISCARD Scalar evaluate(ts::Tai const& time) const NOEXCEPT;
};

struct OrbitCorrection {
    ts::Tai  reference_time;
    uint16_t iod;
    Vector3  delta;      // {radial, along_track, cross_track}
    Vector3  dot_delta;  // {radial, along_track, cross_track}

    NODISCARD Vector3 evaluate(ts::Tai const& time, Vector3 const& eph_position,
                               Vector3 const& eph_velocity) const NOEXCEPT;
};

struct IonosphericPolynomial {
    Scalar  c00;
    Scalar  c01;
    Scalar  c10;
    Scalar  c11;
    Vector3 reference_point;
    Scalar  quality_indicator;
    bool    quality_indicator_valid;

    NODISCARD Scalar evaluate(Vector3 const& llh) const NOEXCEPT;
};

struct TroposphericResult {
    Scalar wet;
    Scalar dry;
};

struct SatelliteCorrection {
    bool                                          clock_valid;
    ClockCorrection                               clock;
    std::unordered_map<uint16_t, OrbitCorrection> orbit;
    std::unordered_map<SignalId, Scalar>          code_bias;
    std::unordered_map<SignalId, Scalar>          phase_bias;
};

struct CorrectionPointSet {
    struct Point {
        bool valid;
        bool has_ionospheric;
        bool has_tropospheric;

        long absolute_index;  // includes missing points
        long array_index;     // excludes missing points

        Vector3                                 position;
        std::unordered_map<SatelliteId, Scalar> ionospheric_residual;
        Scalar                                  tropospheric_wet;
        Scalar                                  tropospheric_dry;
    };

    uint16_t                                               id;
    ts::Tai                                                created_at;
    ts::Tai                                                updated_at;
    Vector3                                                reference_point;
    Vector3                                                delta;
    long                                                   latitude_count;
    long                                                   longitude_count;
    size_t                                                 point_count;
    std::array<Point, 8 * 8>                               points;
    std::unordered_map<SatelliteId, IonosphericPolynomial> ionospheric_polynomials;

    NODISCARD bool contains(Vector3 const& llh) const NOEXCEPT {
        return llh.x() >= reference_point.x() && llh.x() < reference_point.x() + delta.x() &&
               llh.y() >= reference_point.y() && llh.y() < reference_point.y() + delta.y();
    }

    NODISCARD bool troposphere(Vector3 const& llh, TroposphericResult& result) const NOEXCEPT;
    NODISCARD bool ionospheric_residual(Vector3 const& llh, SatelliteId sv_id,
                                        Scalar& residual) const NOEXCEPT;
    NODISCARD bool ionospheric_polynomial(Vector3 const& llh, SatelliteId sv_id,
                                          Scalar& result) const NOEXCEPT;
};

class CorrectionCache {
public:
    CorrectionCache()  = default;
    ~CorrectionCache() = default;

    void process(LPP_Message const& message) NOEXCEPT;

    SatelliteCorrection const* satellite_correction(SatelliteId id) const NOEXCEPT;
    CorrectionPointSet const*  correction_point_set(Vector3 const& llh) const NOEXCEPT;

private:
    void add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15 const* correction) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15 const* correction) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* correction) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16 const* correction) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16 const* correction) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* correction) NOEXCEPT;
    void find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT;

    CorrectionPointSet& create_correction_point_set(uint16_t id) NOEXCEPT;
    CorrectionPointSet* get_correction_point_set(uint16_t id) NOEXCEPT;

    SatelliteCorrection& get_or_create_satellite_correction(SatelliteId id) NOEXCEPT;

    std::unordered_map<SatelliteId, std::unique_ptr<SatelliteCorrection>> mSatelliteCorrections;
    std::unordered_map<uint16_t, std::unique_ptr<CorrectionPointSet>>     mCorrectionPointSets;
};

}  // namespace idokeido

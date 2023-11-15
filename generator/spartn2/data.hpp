#pragma once
#include <generator/spartn2/types.hpp>
#include "time.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct IodGnss {
    /// The IOD of the correction (defined by LPP)
    long iod;
    /// The GNSS ID of the correction (defined by LPP)
    long gnss_id;
    /// The epoch time of the correction (in SPARTN time)
    uint32_t epoch_time;
};

namespace std {
template <>
struct hash<IodGnss> {
    size_t operator()(const IodGnss& iod_gnss) const {
        return hash<long>()(iod_gnss.iod) ^ hash<long>()(iod_gnss.gnss_id) ^
               hash<uint32_t>()(iod_gnss.epoch_time);
    }
};
}  // namespace std

static bool operator==(const IodGnss& lhs, const IodGnss& rhs) {
    return lhs.iod == rhs.iod && lhs.gnss_id == rhs.gnss_id && lhs.epoch_time == rhs.epoch_time;
}

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_URA_r16;

struct SSR_OrbitCorrectionSatelliteElement_r15;
struct SSR_ClockCorrectionSatelliteElement_r15;
struct SSR_CodeBiasSatElement_r15;
struct SSR_PhaseBiasSatElement_r16;
struct SSR_URA_SatElement_r16;

namespace generator {
namespace spartn {

struct CorrectionPointSet {
    long id;
    long grid_points;
    long referencePointLatitude_r16;
    long referencePointLongitude_r16;
    long numberOfStepsLatitude_r16;
    long numberOfStepsLongitude_r16;
    long stepOfLatitude_r16;
    long stepOfLongitude_r16;
};

struct OcbSatellite {
    long                                     id;
    long                                     iod;
    SSR_OrbitCorrectionSatelliteElement_r15* orbit;
    SSR_ClockCorrectionSatelliteElement_r15* clock;
    SSR_CodeBiasSatElement_r15*              code_bias;
    SSR_PhaseBiasSatElement_r16*             phase_bias;
    SSR_URA_SatElement_r16*                  ura;

    void add_correction(SSR_OrbitCorrectionSatelliteElement_r15* orbit);
    void add_correction(SSR_ClockCorrectionSatelliteElement_r15* clock);
    void add_correction(SSR_CodeBiasSatElement_r15* code_bias);
    void add_correction(SSR_PhaseBiasSatElement_r16* phase_bias);
    void add_correction(SSR_URA_SatElement_r16* ura);

    uint32_t prn() const;
};

struct OcbCorrections {
    long   gnss_id;
    long   iod;
    SpartnTime epoch_time;

    GNSS_SSR_OrbitCorrections_r15* orbit;
    GNSS_SSR_ClockCorrections_r15* clock;
    GNSS_SSR_CodeBias_r15*         code_bias;
    GNSS_SSR_PhaseBias_r16*        phase_bias;
    GNSS_SSR_URA_r16*              ura;

    // Generate a set of satellite ids for this correction
    // this is the union of all satellite ids that have at least
    // one correction type.
    std::vector<OcbSatellite> satellites() const;
};

struct OcbData {
    // OCB corrections keyed by GNSS ID
    std::unordered_map<IodGnss, OcbCorrections> mKeyedCorrections;

    void add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit);
    void add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock);
    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias);
    void add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias);
    void add_correction(long gnss_id, GNSS_SSR_URA_r16* ura);
};

}  // namespace spartn
}  // namespace generator

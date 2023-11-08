#pragma once
#include <functional>
#include <unordered_map>
#include <unordered_set>

struct IodGnss {
    long iod;
    long gnss_id;
};

// implement hash for IodGnss
namespace std {
template <>
struct hash<IodGnss> {
    size_t operator()(const IodGnss& iod_gnss) const {
        return hash<long>()(iod_gnss.iod) ^ hash<long>()(iod_gnss.gnss_id);
    }
};
}  // namespace std

// implement equality for IodGnss
static bool operator==(const IodGnss& lhs, const IodGnss& rhs) {
    return lhs.iod == rhs.iod && lhs.gnss_id == rhs.gnss_id;
}

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_URA_r16;

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

struct OcbCorrections {
    GNSS_SSR_OrbitCorrections_r15* orbit;
    GNSS_SSR_ClockCorrections_r15* clock;
    GNSS_SSR_CodeBias_r15*         code_bias;
    GNSS_SSR_PhaseBias_r16*        phase_bias;
    GNSS_SSR_URA_r16*              ura;

    std::unordered_set<long> sv_list();
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

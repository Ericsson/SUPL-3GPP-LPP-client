#pragma once
#include <generator/spartn2/types.hpp>
#include "time.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <GNSS-ID.h>
#pragma GCC diagnostic pop

struct OcbKey {
    long     gnss_id;
    uint32_t epoch_time;

    bool operator==(OcbKey const& other) const {
        return gnss_id == other.gnss_id && epoch_time == other.epoch_time;
    }
};

struct HpacKey {
    uint16_t set_id;
    long     gnss_id;
    uint32_t epoch_time;

    bool operator==(HpacKey const& other) const {
        return gnss_id == other.gnss_id && set_id == other.set_id && epoch_time == other.epoch_time;
    }
};

namespace std {
template <>
struct hash<OcbKey> {
    size_t operator()(OcbKey const& iod_gnss) const {
        return hash<long>()(iod_gnss.gnss_id) ^ hash<uint32_t>()(iod_gnss.epoch_time);
    }
};

template <>
struct hash<HpacKey> {
    size_t operator()(HpacKey const& iod_gnss_set) const {
        return hash<long>()(iod_gnss_set.gnss_id) ^ hash<uint16_t>()(iod_gnss_set.set_id) ^
               hash<uint32_t>()(iod_gnss_set.epoch_time);
    }
};
}  // namespace std

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_URA_r16;

struct GNSS_SSR_GriddedCorrection_r16;
struct GNSS_SSR_STEC_Correction_r16;

struct SSR_OrbitCorrectionSatelliteElement_r15;
struct SSR_ClockCorrectionSatelliteElement_r15;
struct SSR_CodeBiasSatElement_r15;
struct SSR_PhaseBiasSatElement_r16;
struct SSR_URA_SatElement_r16;

struct STEC_SatElement_r16;
struct STEC_ResidualSatElement_r16;
struct GridElement_r16;

namespace generator {
namespace spartn {

struct GridPoint {
    long   id;
    double latitude;
    double longitude;
};

struct CorrectionPointSet {
    uint16_t set_id;
    uint16_t area_id;
    long     grid_point_count;
    long     referencePointLatitude_r16;
    long     referencePointLongitude_r16;
    long     numberOfStepsLatitude_r16;
    long     numberOfStepsLongitude_r16;
    long     stepOfLatitude_r16;
    long     stepOfLongitude_r16;
    uint64_t bitmask;

    double                 reference_point_latitude;
    double                 reference_point_longitude;
    double                 latitude_delta;
    double                 longitude_delta;
    std::vector<GridPoint> internal_grid_points;

    void calculate_grid_points();

    inline bool has_grid_point(long grid_point) const {
        auto index = 64 - 1 - grid_point;
        return (bitmask & (1ULL << index)) != 0;
    }

    inline double latitude_grid_spacing() const {
        return latitude_delta * numberOfStepsLatitude_r16;
    }
    inline double longitude_grid_spacing() const {
        return longitude_delta * numberOfStepsLongitude_r16;
    }

    std::vector<GridPoint> const& grid_points() const;
};

struct OcbSatellite {
    long                                     id;
    uint16_t                                 iod;
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
    long       gnss_id;
    uint16_t   iod;
    SpartnTime epoch_time;

    GNSS_SSR_OrbitCorrections_r15* orbit;
    GNSS_SSR_ClockCorrections_r15* clock;
    GNSS_SSR_CodeBias_r15*         code_bias;
    GNSS_SSR_PhaseBias_r16*        phase_bias;
    GNSS_SSR_URA_r16*              ura;

    double orbit_update_interval;
    double clock_update_interval;

    // Generate a set of satellite ids for this correction
    // this is the union of all satellite ids that have at least
    // one correction type.
    std::vector<OcbSatellite> satellites() const;

    // Check if there is _any_ correction for the given satellite.
    bool has_satellite(long id) const;
};

struct OcbData {
    std::unordered_map<OcbKey, OcbCorrections> mKeyedCorrections;
};

struct HpacSatellite {
    long                                                   id;
    uint16_t                                               iod;
    STEC_SatElement_r16*                                   stec;
    std::unordered_map<long, STEC_ResidualSatElement_r16*> residuals;

    void add_correction(STEC_SatElement_r16* stec);
    void add_correction(long grid_id, STEC_ResidualSatElement_r16* residual);

    uint32_t prn() const;
    bool     has_full_data() const { return stec && residuals.size() > 0; }
    bool     has_all_residuals(CorrectionPointSet const& cps) const;
};

struct HpacCorrections {
    long       gnss_id;
    uint16_t   iod;
    uint16_t   set_id;
    SpartnTime epoch_time;

    GNSS_SSR_GriddedCorrection_r16* gridded;
    GNSS_SSR_STEC_Correction_r16*   stec;

    // Generate a set of satellite ids for this correction
    // this is the union of all satellite ids that have at least
    // one correction type.
    std::vector<HpacSatellite> satellites() const;

    // Find GridElement_r16 for the given grid_id
    GridElement_r16* find_grid_point(long grid_id) const;
};

struct HpacData {
    std::unordered_map<HpacKey, HpacCorrections> mKeyedCorrections;

    void set_ids(std::vector<uint16_t>& ids) const;
};

struct CorrectionData {
    bool                                   mGroupByEpochTime;
    std::unordered_map<uint16_t, OcbData>  mOcbData;
    std::unordered_map<uint16_t, HpacData> mHpacData;

    CorrectionData(bool group_by_epoch_time) : mGroupByEpochTime(group_by_epoch_time) {}

    std::vector<uint16_t> iods() const;
    std::vector<uint16_t> set_ids() const;

    OcbData* ocb(uint16_t iod) {
        auto it = mOcbData.find(iod);
        if (it == mOcbData.end()) return nullptr;
        return &it->second;
    }

    HpacData* hpac(uint16_t iod) {
        auto it = mHpacData.find(iod);
        if (it == mHpacData.end()) return nullptr;
        return &it->second;
    }

    bool find_gad_epoch_time(uint16_t iod, SpartnTime* epoch_time) const;

    void add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit);
    void add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock);
    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias);
    void add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias);
    void add_correction(long gnss_id, GNSS_SSR_URA_r16* ura);

    void add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16* gridded);
    void add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16* stec);
};

inline uint8_t subtype_from_gnss_id(long gnss_id) {
    if (gnss_id == GNSS_ID__gnss_id_gps) return 0;
    if (gnss_id == GNSS_ID__gnss_id_glonass) return 1;
    if (gnss_id == GNSS_ID__gnss_id_galileo) return 2;
    if (gnss_id == GNSS_ID__gnss_id_bds) return 3;
    if (gnss_id == GNSS_ID__gnss_id_qzss) return 4;

    SPARTN_UNREACHABLE();
}

}  // namespace spartn
}  // namespace generator

namespace generator {
namespace spartn {
class Message;
}  // namespace spartn
}  // namespace generator

#pragma once
#include <generator/spartn2/types.hpp>
#include "time.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <GNSS-ID.h>

struct OcbKey {
    long     gnss_id;
    uint32_t epoch_time;
};

struct HpacKey {
    long     set_id;
    long     gnss_id;
    uint32_t epoch_time;
};

namespace std {
template <>
struct hash<OcbKey> {
    size_t operator()(const OcbKey& iod_gnss) const {
        return hash<long>()(iod_gnss.gnss_id) ^ hash<uint32_t>()(iod_gnss.epoch_time);
    }
};

template <>
struct hash<HpacKey> {
    size_t operator()(const HpacKey& iod_gnss_set) const {
        return hash<long>()(iod_gnss_set.gnss_id) ^ hash<long>()(iod_gnss_set.set_id) ^
               hash<uint32_t>()(iod_gnss_set.epoch_time);
    }
};
}  // namespace std

static bool operator==(const OcbKey& lhs, const OcbKey& rhs) {
    return lhs.gnss_id == rhs.gnss_id && lhs.epoch_time == rhs.epoch_time;
}

static bool operator==(const HpacKey& lhs, const HpacKey& rhs) {
    return lhs.gnss_id == rhs.gnss_id && lhs.set_id == rhs.set_id &&
           lhs.epoch_time == rhs.epoch_time;
}

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

namespace generator {
namespace spartn {

struct CorrectionPointSet {
    long     set_id;
    uint16_t area_id;
    long     grid_points;
    long     referencePointLatitude_r16;
    long     referencePointLongitude_r16;
    long     numberOfStepsLatitude_r16;
    long     numberOfStepsLongitude_r16;
    long     stepOfLatitude_r16;
    long     stepOfLongitude_r16;
    uint64_t bitmask;
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
    long       gnss_id;
    long       iod;
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
    std::unordered_map<OcbKey, OcbCorrections> mKeyedCorrections;
};

struct HpacSatellite {
    long                                                   id;
    long                                                   iod;
    STEC_SatElement_r16*                                   stec;
    std::unordered_map<long, STEC_ResidualSatElement_r16*> residuals;

    void add_correction(STEC_SatElement_r16* stec);
    void add_correction(long grid_id, STEC_ResidualSatElement_r16* residual);

    uint32_t prn() const;
    bool     has_full_data() const { return stec && residuals.size() > 0; }
};

struct HpacCorrections {
    long       gnss_id;
    long       iod;
    long       set_id;
    SpartnTime epoch_time;

    GNSS_SSR_GriddedCorrection_r16* gridded;
    GNSS_SSR_STEC_Correction_r16*   stec;

    // Generate a set of satellite ids for this correction
    // this is the union of all satellite ids that have at least
    // one correction type.
    std::vector<HpacSatellite> satellites() const;
};

struct HpacData {
    std::unordered_map<HpacKey, HpacCorrections> mKeyedCorrections;

    void set_ids(std::vector<long>& ids) const;
};

struct CorrectionData {
    bool                               group_by_epoch_time;
    std::unordered_map<long, OcbData>  mOcbData;
    std::unordered_map<long, HpacData> mHpacData;

    CorrectionData(bool group_by_epoch_time) : group_by_epoch_time(group_by_epoch_time) {}

    std::vector<long> iods() const;
    std::vector<long> set_ids() const;

    OcbData* ocb(long iod) {
        auto it = mOcbData.find(iod);
        if (it == mOcbData.end()) return nullptr;
        return &it->second;
    }

    HpacData* hpac(long iod) {
        auto it = mHpacData.find(iod);
        if (it == mHpacData.end()) return nullptr;
        return &it->second;
    }

    bool find_gad_epoch_time(long iod, SpartnTime* epoch_time) const;

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

#include "correction.hpp"

#include <generator/tokoro/coordinate.hpp>
#include <loglet/loglet.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GridElement-r16.h>
#include <SSR-ClockCorrectionSatelliteElement-r15.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#include <STEC-ResidualSatElement-r16.h>
#include <STEC-ResidualSatList-r16.h>
#include <STEC-SatElement-r16.h>
#include <TropospericDelayCorrection-r16.h>
#pragma GCC diagnostic pop

LOGLET_MODULE2(idokeido, corr);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, corr)

namespace idokeido {

static uint8_t gnss_from_satellite_id(SatelliteId id) NOEXCEPT {
    switch (id.gnss()) {
    case SatelliteId::Gnss::GPS: return 0;
    case SatelliteId::Gnss::GLONASS: return 1;
    case SatelliteId::Gnss::GALILEO: return 2;
    case SatelliteId::Gnss::BEIDOU: return 3;
    default: UNREACHABLE();
    }
}

static uint8_t gnss_from_id(long gnss_id) NOEXCEPT {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return 0;
    case GNSS_ID__gnss_id_galileo: return 2;
    case GNSS_ID__gnss_id_glonass: return 1;
    case GNSS_ID__gnss_id_bds: return 3;
    default: UNREACHABLE();
    }
}

static SatelliteId::Gnss satellite_gnss_from_id(long gnss_id) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return SatelliteId::GPS;
    case GNSS_ID__gnss_id_galileo: return SatelliteId::GALILEO;
    case GNSS_ID__gnss_id_glonass: return SatelliteId::GLONASS;
    case GNSS_ID__gnss_id_bds: return SatelliteId::BEIDOU;
    default: UNREACHABLE();
    }
}

static SignalId::Gnss signal_gnss_from_id(long gnss_id) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return SignalId::GPS;
    case GNSS_ID__gnss_id_galileo: return SignalId::GALILEO;
    case GNSS_ID__gnss_id_glonass: return SignalId::GLONASS;
    case GNSS_ID__gnss_id_bds: return SignalId::BEIDOU;
    default: UNREACHABLE();
    }
}

static SignalId signal_id_from(SignalId::Gnss gnss, GNSS_SignalID_t& signal_id) {
    auto id = signal_id.gnss_SignalID;
    if (signal_id.ext1 && signal_id.ext1->gnss_SignalID_Ext_r15) {
        id = *signal_id.ext1->gnss_SignalID_Ext_r15;
    }

    return SignalId::from_lpp(gnss, id);
}

Scalar CorrectionCache::code_bias(uint8_t iod, SatelliteId satellite_id, SignalId signal_id) NOEXCEPT {
    auto  gnss  = gnss_from_satellite_id(satellite_id);
    auto* entry = get_iod_entry(gnss, iod);
    if (entry == nullptr) return 0;
    return entry->code_bias(satellite_id, signal_id);
}

void CorrectionCache::add_correction(long                         gnss_id,
                                     GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT {
    FUNCTION_SCOPE();

    auto gnss           = gnss_from_id(gnss_id);
    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(code_bias->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(code_bias->epochTime_r15);

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& entry = create_iod_entry(gnss, ssr_iod);

    auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("code bias rejected: invalid satellite id (%ld)",
                  satellite->svID_r15.satellite_id);
            continue;
        }

        auto& signal_list = satellite->ssr_CodeBiasSignalList_r15.list;
        for (int j = 0; j < signal_list.count; j++) {
            auto signal = signal_list.array[j];
            if (!signal) continue;

            auto signal_id = signal_id_from(signal_gnss, signal->signal_and_tracking_mode_ID_r15);
            auto value = decode::codeBias_r15(signal->codeBias_r15);
            entry.set_code_bias(satellite_id, signal_id, value);

            VERBOSEF("code bias: %3s %-16s %+f", satellite_id.name(), signal_id.name(),
                     correction.bias);
        }
    }
}

CorrectionCache::IodEntry* CorrectionCache::get_iod_entry(uint8_t gnss, uint8_t iod) NOEXCEPT {
    uint16_t key = (static_cast<uint16_t>(gnss) << 8) | iod;
    auto     it  = mData.find(key);
    if (it == mData.end()) return nullptr;
    return &it->second;
}

CorrectionCache::IodEntry& CorrectionCache::create_iod_entry(uint8_t gnss, uint8_t iod) NOEXCEPT {
    FUNCTION_SCOPEF("%d, %d", gnss, iod);
    uint16_t key = (static_cast<uint16_t>(gnss) << 8) | iod;
    auto     it  = mData.find(key);
    if (it == mData.end()) {
        // TODO(ewasjon): Better eviction policy
        if (mData.size() >= 128) {
            DEBUGF("evicting iod entries to fit new entry: %d, %d", gnss, iod);
            mData.erase(mData.begin());
        }
        auto result = mData.emplace(key, IodEntry{gnss, iod});
        return result.first->second;
    } else {
        return it->second;
    }
}

}  // namespace idokeido

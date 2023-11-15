#include "data.hpp"
#include "time.hpp"

#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-URA-r16.h>

#include <SSR-OrbitCorrectionList-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>

#include <algorithm>

namespace generator {
namespace spartn {

uint32_t OcbSatellite::prn() const {
    return id;
}

void OcbSatellite::add_correction(SSR_OrbitCorrectionSatelliteElement_r15* orbit) {
    if (!orbit) return;
    this->orbit = orbit;
}

void OcbSatellite::add_correction(SSR_ClockCorrectionSatelliteElement_r15* clock) {
    if (!clock) return;
    this->clock = clock;
}

void OcbSatellite::add_correction(SSR_CodeBiasSatElement_r15* code_bias) {
    if (!code_bias) return;
    this->code_bias = code_bias;
}

void OcbSatellite::add_correction(SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) return;
    this->phase_bias = phase_bias;
}

void OcbSatellite::add_correction(SSR_URA_SatElement_r16* ura) {
    if (!ura) return;
    this->ura = ura;
}

std::vector<OcbSatellite> OcbCorrections::satellites() const {
    std::unordered_map<long, OcbSatellite> satellites;

    // Orbit
    if (orbit) {
        auto& list = orbit->ssr_OrbitCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Clock
    if (clock) {
        auto& list = clock->ssr_ClockCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Code bias
    if (code_bias) {
        auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Phase bias
    if (phase_bias) {
        auto& list = phase_bias->ssr_PhaseBiasSatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r16.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // URA
    if (ura) {
        auto& list = ura->ssr_URA_SatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r16.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Convert to vector
    std::vector<OcbSatellite> result;
    for (auto& kv : satellites) {
        result.push_back(kv.second);
    }

    // Sort by satellite id
    std::sort(result.begin(), result.end(), [](const OcbSatellite& a, const OcbSatellite& b) {
        return a.id < b.id;
    });

    return result;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit) {
    if (!orbit) return;
    // TODO(ewasjon): filter based on satellite reference datum
    auto  epoch_time       = spartn_time_from(orbit->epochTime_r15);
    auto  iod_gnss         = IodGnss{orbit->iod_ssr_r15, gnss_id, epoch_time.rounded_seconds};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.gnss_id    = iod_gnss.gnss_id;
    corrections.iod        = iod_gnss.iod;
    corrections.epoch_time = epoch_time;
    corrections.orbit      = orbit;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock) {
    if (!clock) return;
    auto  epoch_time       = spartn_time_from(clock->epochTime_r15);
    auto  iod_gnss         = IodGnss{clock->iod_ssr_r15, gnss_id, epoch_time.rounded_seconds};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.gnss_id    = iod_gnss.gnss_id;
    corrections.iod        = iod_gnss.iod;
    corrections.epoch_time = epoch_time;
    corrections.clock      = clock;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias) {
    if (!code_bias) return;
    auto  epoch_time       = spartn_time_from(code_bias->epochTime_r15);
    auto  iod_gnss         = IodGnss{code_bias->iod_ssr_r15, gnss_id, epoch_time.rounded_seconds};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.gnss_id    = iod_gnss.gnss_id;
    corrections.iod        = iod_gnss.iod;
    corrections.epoch_time = epoch_time;
    corrections.code_bias  = code_bias;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias) {
    if (!phase_bias) return;
    auto  epoch_time       = spartn_time_from(phase_bias->epochTime_r16);
    auto  iod_gnss         = IodGnss{phase_bias->iod_ssr_r16, gnss_id, epoch_time.rounded_seconds};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.gnss_id    = iod_gnss.gnss_id;
    corrections.iod        = iod_gnss.iod;
    corrections.epoch_time = epoch_time;
    corrections.phase_bias = phase_bias;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_URA_r16* ura) {
    if (!ura) return;
    auto  epoch_time       = spartn_time_from(ura->epochTime_r16);
    auto  iod_gnss         = IodGnss{ura->iod_ssr_r16, gnss_id, epoch_time.rounded_seconds};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.gnss_id    = iod_gnss.gnss_id;
    corrections.iod        = iod_gnss.iod;
    corrections.epoch_time = epoch_time;
    corrections.ura        = ura;
}

}  // namespace spartn
}  // namespace generator
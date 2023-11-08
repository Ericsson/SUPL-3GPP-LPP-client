#include "data.hpp"

#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-URA-r16.h>

namespace generator {
namespace spartn {

std::unordered_set<long> OcbCorrections::sv_list() {
    std::unordered_set<long> sv_list;

    return sv_list;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit) {
    if (!orbit) return;
    auto  iod_gnss    = IodGnss{orbit->iod_ssr_r15, gnss_id};
    auto& corrections = mKeyedCorrections[iod_gnss];
    corrections.orbit = orbit;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock) {
    if (!clock) return;
    auto  iod_gnss    = IodGnss{clock->iod_ssr_r15, gnss_id};
    auto& corrections = mKeyedCorrections[iod_gnss];
    corrections.clock = clock;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias) {
    if (!code_bias) return;
    auto  iod_gnss        = IodGnss{code_bias->iod_ssr_r15, gnss_id};
    auto& corrections     = mKeyedCorrections[iod_gnss];
    corrections.code_bias = code_bias;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias) {
    if (!phase_bias) return;
    auto  iod_gnss         = IodGnss{phase_bias->iod_ssr_r16, gnss_id};
    auto& corrections      = mKeyedCorrections[iod_gnss];
    corrections.phase_bias = phase_bias;
}

void OcbData::add_correction(long gnss_id, GNSS_SSR_URA_r16* ura) {
    if (!ura) return;
    auto  iod_gnss    = IodGnss{ura->iod_ssr_r16, gnss_id};
    auto& corrections = mKeyedCorrections[iod_gnss];
    corrections.ura   = ura;
}

}  // namespace spartn
}  // namespace generator
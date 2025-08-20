#include "troposphere.hpp"
#include "constant.hpp"
#include "correction.hpp"
#include "decode.hpp"

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

#include <asn.1/bit_string.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE3(tokoro, data, troposphere);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, troposphere)

namespace generator {
namespace tokoro {

bool CorrectionData::tropospheric(SatelliteId sv_id, Float3 llh,
                                  TroposphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    if (mCorrectionPointSet == nullptr) {
        WARNF("tropospheric correction grid not available");
        return false;
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it == mGrid.end()) {
        WARNF("tropospheric correction for satellite not found");
        return false;
    }

    auto& grid = grid_it->second;
    if (!grid.tropospheric(llh, correction)) {
        VERBOSEF("tropospheric correction not found");
        return false;
    }

    return true;
}

}  // namespace tokoro
}  // namespace generator

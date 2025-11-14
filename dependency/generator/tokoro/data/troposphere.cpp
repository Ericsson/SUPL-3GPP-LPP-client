#include "troposphere.hpp"
#include "constant.hpp"
#include "correction.hpp"
#include "decode.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
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
EXTERNAL_WARNINGS_POP

#include <asn.1/bit_string.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE3(tokoro, data, troposphere);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, troposphere)

namespace generator {
namespace tokoro {

bool CorrectionData::tropospheric(SatelliteId sv_id, Float3 llh,
                                  TroposphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    if (correction_point_set == nullptr) {
        WARNF("tropospheric correction grid not available: no correction point set (missing "
              "assistance data)");
        return false;
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it == mGrid.end()) {
        WARNF("tropospheric correction grid not available: no grid for GNSS (missing assistance "
              "data)");
        return false;
    }

    auto& grid   = grid_it->second;
    auto  status = grid.tropospheric(llh, correction);
    if (status == GridData::GridStatus::PositionOutsideGrid) {
        WARNF("tropospheric correction not available: position outside grid");
        return false;
    } else if (status == GridData::GridStatus::MissingSatelliteData) {
        WARNF("tropospheric correction not available: missing grid point data");
        return false;
    }

    return true;
}

}  // namespace tokoro
}  // namespace generator

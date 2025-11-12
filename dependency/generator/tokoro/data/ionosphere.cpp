#include "ionosphere.hpp"
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

LOGLET_MODULE3(tokoro, data, ionosphere);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, ionosphere)

namespace generator {
namespace tokoro {

bool CorrectionData::ionospheric(SatelliteId sv_id, Float3 llh,
                                 IonosphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto has_polynomial = false;
    auto has_gridded    = false;

    correction = {};

    auto it = mIonosphericPolynomial.find(sv_id);
    if (it != mIonosphericPolynomial.end()) {
        auto& polynomial = it->second;
        auto  latitude   = (llh.x * constant::RAD2DEG) - polynomial.reference_point_latitude;
        auto  longitude  = (llh.y * constant::RAD2DEG) - polynomial.reference_point_longitude;

        VERBOSEF("polynomial:");
        VERBOSEF("  c00: %.14f", polynomial.c00);
        VERBOSEF("  c01: %.14f", polynomial.c01);
        VERBOSEF("  c10: %.14f", polynomial.c10);
        VERBOSEF("  c11: %.14f", polynomial.c11);

        VERBOSEF("  px: %.14f", llh.x * constant::RAD2DEG);
        VERBOSEF("  rx: %.14f", polynomial.reference_point_latitude);
        VERBOSEF("  dx: %.14f", latitude);

        VERBOSEF("  py: %.14f", llh.y * constant::RAD2DEG);
        VERBOSEF("  ry: %.14f", polynomial.reference_point_longitude);
        VERBOSEF("  dy: %.14f", longitude);

        auto c00 = polynomial.c00;
        auto c01 = polynomial.c01 * latitude;
        auto c10 = polynomial.c10 * longitude;
        auto c11 = polynomial.c11 * latitude * longitude;

        VERBOSEF("    c00: %.14f", c00);
        VERBOSEF("    c01: %.14f", c01);
        VERBOSEF("    c10: %.14f", c10);
        VERBOSEF("    c11: %.14f", c11);

        has_polynomial                 = true;
        correction.polynomial_residual = c00 + c01 + c10 + c11;

        if (polynomial.quality_indicator_valid) {
            correction.quality_valid = true;
            correction.quality       = polynomial.quality_indicator;
        }
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it != mGrid.end()) {
        auto status = grid_it->second.ionospheric(sv_id, llh, correction.grid_residual);
        if (status == GridData::GridStatus::Success) {
            has_gridded = true;
        } else if (status == GridData::GridStatus::PositionOutsideGrid) {
            WARNF("ionospheric correction not available for %s: position outside grid",
                  sv_id.name());
        } else if (status == GridData::GridStatus::MissingSatelliteData) {
            WARNF("ionospheric correction not available for %s: satellite data missing from grid "
                  "points",
                  sv_id.name());
        }
    } else if (!has_polynomial) {
        WARNF("ionospheric correction not available: no grid for GNSS (missing assistance data)");
    }

    return has_polynomial || has_gridded;
}

}  // namespace tokoro
}  // namespace generator

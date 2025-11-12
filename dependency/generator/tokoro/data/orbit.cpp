#include "orbit.hpp"
#include "constant.hpp"
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

LOGLET_MODULE3(tokoro, data, orbit);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, orbit)

namespace generator {
namespace tokoro {

bool OrbitCorrection::correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                                 Float3& result, Float3* output_radial, Float3* output_along,
                                 Float3* output_cross, double* output_delta) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto e_along = eph_velocity;
    if (!e_along.normalize()) {
        WARNF("failed to normalize e_along");
        return false;
    }
    VERBOSEF("e_along:   %+24.14f, %+24.14f, %+24.14f", e_along.x, e_along.y, e_along.z);

    auto e_cross = cross_product(eph_position, eph_velocity);
    if (!e_cross.normalize()) {
        WARNF("failed to normalize e_cross");
        return false;
    }
    VERBOSEF("e_cross:   %+24.14f, %+24.14f, %+24.14f", e_cross.x, e_cross.y, e_cross.z);

    auto e_radial = cross_product(e_along, e_cross);
    VERBOSEF("e_radial:  %+24.14f, %+24.14f, %+24.14f", e_radial.x, e_radial.y, e_radial.z);

    VERBOSEF("t:   %s", time.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", reference_time.rtklib_time_string().c_str());

    auto t_k = time.difference_seconds(reference_time);
    VERBOSEF("t_k: %+.14f", t_k);

    auto delta_at = delta + dot_delta * t_k;
    VERBOSEF("delta:     %+24.14f, %+24.14f, %+24.14f", delta.x, delta.y, delta.z);
    VERBOSEF("dot_delta: %+24.14f, %+24.14f, %+24.14f", dot_delta.x, dot_delta.y, dot_delta.z);
    VERBOSEF("delta_at:  %+24.14f, %+24.14f, %+24.14f", delta_at.x, delta_at.y, delta_at.z);

    auto x = e_radial.x * delta_at.x + e_along.x * delta_at.y + e_cross.x * delta_at.z;
    auto y = e_radial.y * delta_at.x + e_along.y * delta_at.y + e_cross.y * delta_at.z;
    auto z = e_radial.z * delta_at.x + e_along.z * delta_at.y + e_cross.z * delta_at.z;

    VERBOSEF("result:    %+24.14f, %+24.14f, %+24.14f", x, y, z);

    if (output_radial) *output_radial = e_radial;
    if (output_along) *output_along = e_along;
    if (output_cross) *output_cross = e_cross;
    if (output_delta) *output_delta = t_k;

    result = eph_position - Float3{x, y, z};
    return true;
}

}  // namespace tokoro
}  // namespace generator

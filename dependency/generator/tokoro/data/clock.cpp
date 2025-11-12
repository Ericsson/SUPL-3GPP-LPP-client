#include "clock.hpp"
#include "constant.hpp"
#include "decode.hpp"

#include <time/gps.hpp>
#include <time/utc.hpp>

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

LOGLET_MODULE3(tokoro, data, clock);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, clock)

namespace generator {
namespace tokoro {

double ClockCorrection::correction(ts::Tai time) const NOEXCEPT {
    FUNCTION_SCOPE();

    VERBOSEF("t:   %s", ts::Utc{time}.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", ts::Utc{reference_time}.rtklib_time_string().c_str());

    auto t_k = ts::Gps{time}.difference(ts::Gps{reference_time}).full_seconds();
    VERBOSEF("t_k: %+.14f", t_k);

    VERBOSEF("c:      %+24.14f, %+24.14f, %+24.14f", c0, c1, c2);

    auto r0 = c0;
    auto r1 = c1 * t_k;
    auto r2 = c2 * t_k * t_k;
    VERBOSEF("parts:  %+24.14f, %+24.14f, %+24.14f", r0, r1, r2);

    auto result = r0 + r1 + r2;
    VERBOSEF("result: %+24.14f", result);
    return result;
}

}  // namespace tokoro
}  // namespace generator

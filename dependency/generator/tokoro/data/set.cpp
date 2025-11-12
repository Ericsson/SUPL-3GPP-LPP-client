#include "set.hpp"
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

LOGLET_MODULE3(tokoro, data, set);

namespace generator {
namespace tokoro {

bool CorrectionPointSet::array_to_index(long                 array_index,
                                        CorrectionPointInfo* result) const NOEXCEPT {
    long array_count    = 0;
    long absolute_index = 0;
    for (long y = 0; y <= number_of_steps_latitude; y++) {
        for (long x = 0; x <= number_of_steps_longitude; x++) {
            auto i        = y * (number_of_steps_longitude + 1) + x;
            auto bit      = 1ULL << (64 - 1 - i);
            auto is_valid = (bitmask & bit) != 0;

            if (array_count == array_index) {
                if (result) {
                    result->array_index     = array_index;
                    result->absolute_index  = absolute_index;
                    result->is_valid        = is_valid;
                    result->latitude_index  = y;
                    result->longitude_index = x;
                    result->position.x =
                        reference_point_latitude - static_cast<double>(y) * step_of_latitude;
                    result->position.y =
                        reference_point_longitude + static_cast<double>(x) * step_of_longitude;
                    result->position.z = 0;
                }
                return true;
            }

            // only valid grid points are included in the array
            if (is_valid) {
                array_count++;
            }
            absolute_index++;
        }
    }

    return false;
}

}  // namespace tokoro
}  // namespace generator

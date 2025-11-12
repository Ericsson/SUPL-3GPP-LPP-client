#include "data.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <GNSS-ID.h>
EXTERNAL_WARNINGS_POP

namespace generator {
namespace spartn {

uint8_t subtype_from_gnss_id(long gnss_id) {
    if (gnss_id == GNSS_ID__gnss_id_gps) return 0;
    if (gnss_id == GNSS_ID__gnss_id_glonass) return 1;
    if (gnss_id == GNSS_ID__gnss_id_galileo) return 2;
    if (gnss_id == GNSS_ID__gnss_id_bds) return 3;
    if (gnss_id == GNSS_ID__gnss_id_qzss) return 4;

    CORE_UNREACHABLE();
}

}  // namespace spartn
}  // namespace generator

#include "data.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#include <GNSS-ID.h>
#pragma GCC diagnostic pop

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

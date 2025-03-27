#include "location_information.hpp"
#include "client.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, li);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, li)

void LocationCollector::inspect(streamline::System&, DataType const& location) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_location_information(location);
}

void MetricsCollector::inspect(streamline::System&, DataType const& metrics) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_gnss_metrics(metrics);
}

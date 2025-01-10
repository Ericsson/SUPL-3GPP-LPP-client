#include "location_information.hpp"
#include "client.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "p/li"

void LocationCollector::inspect(streamline::System&, DataType const& location) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_location_information(location);
}

void MetricsCollector::inspect(streamline::System&, DataType const& metrics) NOEXCEPT {
    VSCOPE_FUNCTION();
    mProgram.update_gnss_metrics(metrics);
}

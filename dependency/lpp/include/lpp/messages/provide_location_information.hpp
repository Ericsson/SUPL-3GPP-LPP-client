#pragma once
#include <lpp/location_information.hpp>
#include <lpp/message.hpp>
#include <lpp/periodic_session.hpp>
#include <supl/cell.hpp>

namespace lpp {
namespace messages {
struct ProvideLocationInformation {
    Optional<LocationInformation> location_information;
    Optional<HaGnssMetrics>       gnss_metrics;
};

Message create_provide_location_information(ProvideLocationInformation const& data);

}  // namespace messages
}  // namespace lpp

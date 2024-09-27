#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

namespace generator {
namespace rtcm {
extern Message generate_1005(ReferenceStation const& reference_station, bool gps_indicator,
                             bool glonass_indicator, bool galileo_indicator);
extern Message generate_1006(ReferenceStation const& reference_station, bool gps_indicator,
                             bool glonass_indicator, bool galileo_indicator);
extern Message generate_1032(ReferenceStation const&         reference_station,
                             PhysicalReferenceStation const& physical_reference_station);
}  // namespace rtcm
}  // namespace generator

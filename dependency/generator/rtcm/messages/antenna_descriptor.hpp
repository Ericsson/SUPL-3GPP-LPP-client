#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

namespace generator {
namespace rtcm {
extern Message generate_1033(ReferenceStation const&  reference_station,
                             AntennaDescriptor const& antenna_descriptor);
}  // namespace rtcm
}  // namespace generator

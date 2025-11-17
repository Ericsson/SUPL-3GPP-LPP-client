#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

namespace generator {
namespace rtcm {

extern generator::rtcm::Message generate_1030(generator::rtcm::Residuals const& residuals);
extern generator::rtcm::Message generate_1031(generator::rtcm::Residuals const& residuals);

}  // namespace rtcm
}  // namespace generator

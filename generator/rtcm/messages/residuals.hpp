#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message generate_1030(const generator::rtcm::Residuals& residuals);
extern generator::rtcm::Message generate_1031(const generator::rtcm::Residuals& residuals);

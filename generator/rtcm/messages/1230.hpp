#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message
generate_1230(const generator::rtcm::BiasInformation& bias_information);

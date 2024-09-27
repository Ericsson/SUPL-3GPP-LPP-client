#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message
generate_1230(generator::rtcm::BiasInformation const& bias_information);

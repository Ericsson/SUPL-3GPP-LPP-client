#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message
generate_1005(const generator::rtcm::ReferenceStation& reference_station, bool gps_indicator,
              bool glonass_indicator, bool galileo_indicator);
extern generator::rtcm::Message
generate_1006(const generator::rtcm::ReferenceStation& reference_station, bool gps_indicator,
              bool glonass_indicator, bool galileo_indicator);
extern generator::rtcm::Message
generate_1032(const generator::rtcm::ReferenceStation&         reference_station,
              const generator::rtcm::PhysicalReferenceStation& physical_reference_station);

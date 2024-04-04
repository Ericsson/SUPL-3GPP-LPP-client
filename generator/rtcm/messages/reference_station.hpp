#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message
generate_1005(generator::rtcm::ReferenceStation const& reference_station, bool gps_indicator,
              bool glonass_indicator, bool galileo_indicator);
extern generator::rtcm::Message
generate_1006(generator::rtcm::ReferenceStation const& reference_station, bool gps_indicator,
              bool glonass_indicator, bool galileo_indicator);
extern generator::rtcm::Message
generate_1032(generator::rtcm::ReferenceStation const&         reference_station,
              generator::rtcm::PhysicalReferenceStation const& physical_reference_station);

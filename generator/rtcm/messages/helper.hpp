#pragma once
#include "encoder.hpp"
#include "time/bdt_time.hpp"
#include "time/glo_time.hpp"
#include "time/gps_time.hpp"
#include "time/gst_time.hpp"
#include "time/tai_time.hpp"
#include "time/utc_time.hpp"
#include <cmath>

#define ROUND(x) (floor((x) + 0.5))

static void epoch_time(Encoder& encoder, const ts::TAI_Time& time, GenericGnssId gnss) {
    switch (gnss) {
    case GenericGnssId::GPS: {
        auto tow          = ts::GPS_Time(time).time_of_week();
        auto milliseconds = tow.full_seconds() * 1000;
        encoder.u32(30, static_cast<uint32_t>(milliseconds));
    } break;
    case GenericGnssId::GLONASS: {
        auto glo          = ts::GLO_Time(time);
        auto dow          = glo.days() % 7;
        auto tow          = glo.time_of_day();
        auto milliseconds = tow.full_seconds() * 1000;
        encoder.u8(3, static_cast<uint8_t>(dow));
        encoder.u32(27, static_cast<uint32_t>(milliseconds));
    } break;
    case GenericGnssId::GALILEO: {
        auto tow          = ts::GST_Time(time).time_of_week();
        auto milliseconds = tow.full_seconds() * 1000;
        encoder.u32(30, static_cast<uint32_t>(milliseconds));
    } break;
    case GenericGnssId::BEIDOU: {
        auto tow          = ts::BDT_Time(time).time_of_week();
        auto milliseconds = tow.full_seconds() * 1000;
        encoder.u32(30, static_cast<uint32_t>(milliseconds));
    } break;
    }
}

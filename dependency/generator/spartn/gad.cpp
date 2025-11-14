#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(spartn, gad);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(spartn, gad)

namespace generator {
namespace spartn {

void Generator::generate_gad(uint16_t iod, uint32_t epoch_time, uint16_t set_id) {
    auto cps_it = mCorrectionPointSets.find(set_id);
    if (cps_it == mCorrectionPointSets.end()) return;
    auto& correction_point_set = *(cps_it->second.get());

    VERBOSEF("  grid points: %ld", correction_point_set.grid_point_count);

    char buffer[256];
    for (auto i = 0; i < correction_point_set.grid_point_count; i++) {
        if (i < 255) {
            buffer[i] = correction_point_set.has_grid_point(i) ? '1' : '0';
        }
    }
    buffer[255] = '\0';
    if (correction_point_set.grid_point_count < 256) {
        buffer[correction_point_set.grid_point_count] = '\0';
    }
    VERBOSEF("  bitmask:  %s", buffer);
    VERBOSEF("  ref-lat:  %9.6f", decode::reference_point_latitude_r16(
                                      correction_point_set.reference_point_latitude_r16));
    VERBOSEF("  ref-lng: %10.6f", decode::reference_point_longitude_r16(
                                      correction_point_set.reference_point_longitude_r16));
    VERBOSEF("  steps-lat: %ld", correction_point_set.number_of_steps_latitude_r16);
    VERBOSEF("  steps-lng: %ld", correction_point_set.number_of_steps_longitude_r16);
    VERBOSEF("  delta-lat:  %9.6f",
             decode::step_of_latitude_r16(correction_point_set.step_of_latitude_r16));
    VERBOSEF("  delta-lng: %10.6f",
             decode::step_of_longitude_r16(correction_point_set.step_of_longitude_r16));

    size_t buffer_count = 0;
    size_t buffer_size  = sizeof(buffer);

    auto i           = 0;
    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        if (gp.id < 0) {
            buffer_count += static_cast<size_t>(
                snprintf(buffer + buffer_count, buffer_size - buffer_count, "-- "));
        } else {
            buffer_count += static_cast<size_t>(
                snprintf(buffer + buffer_count, buffer_size - buffer_count, "%02ld ", gp.id));
        }

        if (++i % (correction_point_set.number_of_steps_longitude_r16 + 1) == 0) {
            VERBOSEF("%s", buffer);
            buffer_count = 0;
        }
    }

    if (buffer_count > 0) {
        VERBOSEF("%s", buffer);
    }

    auto siou = iod;
    if (mIncreasingSiou) {
        siou = mSiouIndex;
    }

    MessageBuilder builder{2 /* GAD */, 0, epoch_time};
    builder.sf005(siou);  // TODO(ewasjon): We could include AIOU in the correction point set, to
                          // handle overflow
    builder.sf068(0);
    builder.sf069();

    // NOTE(ewasjon): 3GPP LPP can only handle one area
    builder.sf030(1);

    {
        // TODO(ewasjon): Not sure why the area_id is a uint16_t, because the value is only 8 bits.
        builder.sf031(static_cast<uint8_t>(correction_point_set.area_id));

        auto reference_point_lat =
            decode::reference_point_latitude_r16(correction_point_set.reference_point_latitude_r16);
        auto reference_point_lng = decode::reference_point_longitude_r16(
            correction_point_set.reference_point_longitude_r16);
        builder.sf032(reference_point_lat);
        builder.sf033(reference_point_lng);

        // NOTE(ewasjon): 3GPP LPP has the number of steps, not the number of points
        auto grid_count_lat = correction_point_set.number_of_steps_latitude_r16 + 1;
        auto grid_count_lng = correction_point_set.number_of_steps_longitude_r16 + 1;
        builder.sf034(static_cast<uint8_t>(grid_count_lat));
        builder.sf035(static_cast<uint8_t>(grid_count_lng));

        auto delta_lat = decode::step_of_latitude_r16(correction_point_set.step_of_latitude_r16);
        auto delta_lng = decode::step_of_longitude_r16(correction_point_set.step_of_longitude_r16);
        builder.sf036(delta_lat);
        builder.sf037(delta_lng);
    }

    mMessages.push_back(builder.build());
}

}  // namespace spartn
}  // namespace generator

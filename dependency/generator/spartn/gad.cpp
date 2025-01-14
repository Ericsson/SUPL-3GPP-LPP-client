#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"

#include <loglet/loglet.hpp>
#define LOGLET_CURRENT_MODULE "spartn/g"

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
    VERBOSEF("  ref-lat:  %9.6f",
             decode::referencePointLatitude_r16(correction_point_set.referencePointLatitude_r16));
    VERBOSEF("  ref-lng: %10.6f",
             decode::referencePointLongitude_r16(correction_point_set.referencePointLongitude_r16));
    VERBOSEF("  steps-lat: %ld", correction_point_set.numberOfStepsLatitude_r16);
    VERBOSEF("  steps-lng: %ld", correction_point_set.numberOfStepsLongitude_r16);
    VERBOSEF("  delta-lat:  %9.6f",
             decode::stepOfLatitude_r16(correction_point_set.stepOfLatitude_r16));
    VERBOSEF("  delta-lng: %10.6f",
             decode::stepOfLongitude_r16(correction_point_set.stepOfLongitude_r16));

    auto buffer_count = 0;

    auto i           = 0;
    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        if (gp.id < 0) {
            buffer_count += snprintf(buffer + buffer_count, sizeof(buffer) - buffer_count, "-- ");
        } else {
            buffer_count +=
                snprintf(buffer + buffer_count, sizeof(buffer) - buffer_count, "%02ld ", gp.id);
        }

        if (++i % (correction_point_set.numberOfStepsLongitude_r16 + 1) == 0) {
            VERBOSEF("%s", buffer);
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
            decode::referencePointLatitude_r16(correction_point_set.referencePointLatitude_r16);
        auto reference_point_lng =
            decode::referencePointLongitude_r16(correction_point_set.referencePointLongitude_r16);
        builder.sf032(reference_point_lat);
        builder.sf033(reference_point_lng);

        // NOTE(ewasjon): 3GPP LPP has the number of steps, not the number of points
        auto grid_count_lat = correction_point_set.numberOfStepsLatitude_r16 + 1;
        auto grid_count_lng = correction_point_set.numberOfStepsLongitude_r16 + 1;
        builder.sf034(static_cast<uint8_t>(grid_count_lat));
        builder.sf035(static_cast<uint8_t>(grid_count_lng));

        auto delta_lat = decode::stepOfLatitude_r16(correction_point_set.stepOfLatitude_r16);
        auto delta_lng = decode::stepOfLongitude_r16(correction_point_set.stepOfLongitude_r16);
        builder.sf036(delta_lat);
        builder.sf037(delta_lng);
    }

    mMessages.push_back(builder.build());
}

}  // namespace spartn
}  // namespace generator

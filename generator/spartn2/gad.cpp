#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"

namespace generator {
namespace spartn {

void Generator::generate_gad(long iod, uint32_t epoch_time, long set_id) {
    auto cps_it = mCorrectionPointSets.find(set_id);
    if (cps_it == mCorrectionPointSets.end()) return;
    auto& correction_point_set = *(cps_it->second.get());

#ifdef SPARTN_DEBUG_PRINT
    printf("  grid points: %ld\n", correction_point_set.grid_points);
    printf("  bitmask:     ");
    for (auto i = 0; i < correction_point_set.grid_points; i++) {
        auto bit_index = 64 - i - 1;
        auto bit       = (correction_point_set.bitmask >> bit_index) & 1;
        printf("%d", bit ? 1 : 0);
    }
    printf("\n");
    printf("  ref-lat:  %9.6f\n",
           decode::referencePointLatitude_r16(correction_point_set.referencePointLatitude_r16));
    printf("  ref-lng: %10.6f\n",
           decode::referencePointLongitude_r16(correction_point_set.referencePointLongitude_r16));
    printf("  steps-lat: %ld\n", correction_point_set.numberOfStepsLatitude_r16);
    printf("  steps-lng: %ld\n", correction_point_set.numberOfStepsLongitude_r16);
    printf("  delta-lat:  %9.6f\n",
           decode::stepOfLatitude_r16(correction_point_set.stepOfLatitude_r16));
    printf("  delta-lng: %10.6f\n",
           decode::stepOfLongitude_r16(correction_point_set.stepOfLongitude_r16));

    // print the grid in ascii
    for (auto i = 0; i < correction_point_set.numberOfStepsLatitude_r16 + 1; i++) {
        for (auto j = 0; j < correction_point_set.numberOfStepsLongitude_r16 + 1; j++) {
            auto index     = i * (correction_point_set.numberOfStepsLongitude_r16 + 1) + j;
            auto bit_index = 64 - index - 1;
            auto bit       = (correction_point_set.bitmask >> bit_index) & 1;
            if (bit == 0) {
                printf("-- ");
            } else {
                printf("%02ld ", index);
            }
        }
        printf("\n");
    }
#endif

    MessageBuilder builder{2 /* GAD */, 0, epoch_time};
    builder.sf005(iod);  // TODO(ewasjon): We could include AIOU in the correction point set, to
                         // handle overflow
    builder.sf068(0);
    builder.sf069();

    // NOTE(ewasjon): 3GPP LPP can only handle one area
    builder.sf030(1);

    {
        builder.sf031(correction_point_set.area_id);

        auto reference_point_lat =
            decode::referencePointLatitude_r16(correction_point_set.referencePointLatitude_r16);
        auto reference_point_lng =
            decode::referencePointLongitude_r16(correction_point_set.referencePointLongitude_r16);
        builder.sf032(reference_point_lat);
        builder.sf033(reference_point_lng);

        // NOTE(ewasjon): 3GPP LPP has the number of steps, not the number of points
        auto grid_count_lat = correction_point_set.numberOfStepsLatitude_r16 + 1;
        auto grid_count_lng = correction_point_set.numberOfStepsLongitude_r16 + 1;
        builder.sf034(grid_count_lat);
        builder.sf035(grid_count_lng);

        auto delta_lat = decode::stepOfLatitude_r16(correction_point_set.stepOfLatitude_r16);
        auto delta_lng = decode::stepOfLongitude_r16(correction_point_set.stepOfLongitude_r16);
        builder.sf036(delta_lat);
        builder.sf037(delta_lng);
    }

    mMessages.push_back(builder.build());
}

}  // namespace spartn
}  // namespace generator

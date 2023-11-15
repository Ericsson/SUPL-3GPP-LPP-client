#include "generator.hpp"
#include "data.hpp"
#include "decode.hpp"
#include "message.hpp"

#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>

#include <unordered_map>

static void generate_gad(std::vector<generator::spartn::Message>& messages,
                         const ProvideAssistanceData_r9_IEs*      message) {}

struct PhaseBias {
    long   signal;
    double phase_bias;
};

static std::vector<PhaseBias> phase_biases(const SSR_PhaseBiasSatElement_r16& phase_bias) {
    std::vector<PhaseBias> biases;

    auto& list = phase_bias.ssr_PhaseBiasSignalList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto signal_id  = decode::signal_id(element->signal_and_tracking_mode_ID_r16);
        auto phase_bias = decode::phaseBias_r16(element->phaseBias_r16);
        biases.push_back(PhaseBias{signal_id, phase_bias});
    }

    return biases;
}

static std::unordered_map<uint8_t, PhaseBias> gps_phase_biases(std::vector<PhaseBias> biases) {
    static SPARTN_CONSTEXPR uint8_t GPS_MAPPABLE[32] = {
        1, 0, 0, 0, 0, 0, 0, 0, 2, 0,  //
        3, 0, 0, 4, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0,
    };

    static SPARTN_CONSTEXPR uint8_t GPS_CONVERTABLE[32] = {
        0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 10, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0,
    };

    static SPARTN_CONSTEXPR double GPS_FREQ[32] = {
        1575.42,  // 0: L1 C/A
        1575.42,  // 1: L1C
        1227.60,  // 2: L2C
        1176.45,  // 3: L5
        1575.42,  // 4: L! P
        1227.60,  // 5: L1 Z-tracking
        1227.60,  // 6: L2 C/A
        1227.60,  // 7: L2 P
        1227.60,  // 8: L2 Z-tracking
        1227.60,  // 9: L2 L2C(M)
        1227.60,  // 10: L2 L2C(L)
        1227.60,  // 11: L2 L2C(M+L)
        1176.45,  // 12: L5 I
        1176.45,  // 13: L5 Q
        1176.45,  // 14: L5 I+Q
        1575.42,  // 15: L1 L1C(D)
        1575.42,  // 16: L1 L1C(P)
        1575.42,  // 17: L1 L1C(D+P)
        0,       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    std::unordered_map<uint8_t, PhaseBias> biases_by_type;

    // find direct mappable signals
    for (auto& phase_bias : biases) {
        if (phase_bias.signal >= 64) continue;
        if (GPS_MAPPABLE[phase_bias.signal] == 0) continue;

        auto type = GPS_MAPPABLE[phase_bias.signal];
        if (biases_by_type.count(type) == 0) {
            biases_by_type[type] = phase_bias;
        } else {
            // TODO(ewasjon): report error?
        }
    }

    // find convertable signals
    for (auto& phase_bias : biases) {
        if (phase_bias.signal >= 64) continue;
        if (GPS_CONVERTABLE[phase_bias.signal] == 0) continue;

        auto convert_to_id = GPS_CONVERTABLE[phase_bias.signal];
        assert(GPS_MAPPABLE[convert_to_id] != 0);

        auto type = GPS_MAPPABLE[convert_to_id];
        if (biases_by_type.count(type) == 0) {
            auto from_freq  = GPS_FREQ[phase_bias.signal];
            auto to_freq    = GPS_FREQ[convert_to_id];
            auto correction = phase_bias.phase_bias * (to_freq / from_freq);

            biases_by_type[type] = PhaseBias{phase_bias.signal, correction};
        }
    }

    return biases_by_type;
}

static void generate_gps_bias_block(MessageBuilder&                    builder,
                                    const SSR_CodeBiasSatElement_r15*  code_bias,
                                    const SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) {
        builder.sf025_raw(false, 0);
    } else {
        auto list_of_biases  = phase_biases(*phase_bias);
        auto types_of_biases = gps_phase_biases(list_of_biases);

        builder.sf025(&types_of_biases);

        // TODO(ewasjon): include the biases!!!
    }

    if (!code_bias) {
        builder.sf027_raw(false, 0);
    } else {
        builder.sf027_raw(false, 0);
    }
}

static void generate_ocb(std::vector<generator::spartn::Message>& messages,
                         const generator::spartn::OcbData*        data) {
    for (auto& kvp : data->mKeyedCorrections) {
        auto  iod_gnss    = kvp.first;
        auto& corrections = kvp.second;

        auto satellites = corrections.satellites();

        // TODO(ewasjon): remove
        if (iod_gnss.gnss_id != 0) continue;

        printf("OCB: time=%u, gnss=%ld, iod=%ld\n", iod_gnss.epoch_time, iod_gnss.gnss_id,
               iod_gnss.iod);
        for (auto& satellite : satellites) {
            printf("  satellite: %4ld  ", satellite.id);
            if (satellite.orbit) {
                printf("O");
            } else {
                printf("-");
            }
            if (satellite.clock) {
                printf("C");
            } else {
                printf("-");
            }
            if (satellite.code_bias) {
                printf("B");
            } else {
                printf("-");
            }
            if (satellite.phase_bias) {
                printf("P");
            } else {
                printf("-");
            }
            if (satellite.ura) {
                printf("U");
            } else {
                printf("-");
            }
            printf("\n");
        }

        // TODO(ewasjon): End of OCB set. How does this work with different epoch-times?
        auto eos               = false;
        auto yaw_angle_present = true;

        // TODO(ewasjon): Confirm that the subtypes are correct
        uint8_t subtype = 0;
        if (iod_gnss.gnss_id == GNSS_ID__gnss_id_gps) subtype = 0;
        if (iod_gnss.gnss_id == GNSS_ID__gnss_id_glonass) subtype = 1;
        if (iod_gnss.gnss_id == GNSS_ID__gnss_id_galileo) subtype = 2;
        if (iod_gnss.gnss_id == GNSS_ID__gnss_id_bds) subtype = 3;
        if (iod_gnss.gnss_id == GNSS_ID__gnss_id_qzss) subtype = 4;

        MessageBuilder builder{0 /* OCB */, subtype, iod_gnss.epoch_time};
        builder.sf005(iod_gnss.iod);
        builder.sf010(eos);
        builder.sf069();
        builder.sf008(yaw_angle_present);
        builder.sf009(0 /* ITRF */);  // we assume that satellite reference datum is ITRF
        builder.ephemeris_type(iod_gnss.gnss_id);
        builder.satellite_mask(iod_gnss.gnss_id, satellites);

        for (auto& satellite : satellites) {
            builder.sf013(false /* do use this satellite */);
            builder.sf014(satellite.orbit != nullptr, satellite.clock != nullptr,
                          satellite.code_bias != nullptr || satellite.phase_bias != nullptr);
            builder.sf015(320.0);  // TODO(ewasjon): compute the continuity indicator

            if (satellite.orbit) {
                auto& orbit = *satellite.orbit;
                builder.orbit_iode(iod_gnss.gnss_id, orbit.iod_r15);

                auto radial = decode::delta_radial_r15(orbit.delta_radial_r15);
                auto along  = decode::delta_AlongTrack_r15(orbit.delta_AlongTrack_r15);
                auto cross  = decode::delta_CrossTrack_r15(orbit.delta_CrossTrack_r15);
                builder.sf020(radial);
                builder.sf020(along);
                builder.sf020(cross);
                if (yaw_angle_present) {
                    // NOTE(ewasjon): Yaw angle is assumed to be zero, 3GPP LPP inherits this
                    // assumption from CLAS specification.
                    builder.sf021(0.0 /* 0 degrees */);
                }
            }

            if (satellite.clock) {
                auto& clock = *satellite.clock;
                builder.sf022(320.0);  // TODO(ewasjon): compute the continuity indicator

                // NOTE(ewasjon): SPARTN has a single value of the clock correction term. 3GPP LPP
                // has model this as an polynomial around the middle of the ssr update rate. Thus,
                // the single value at epoch time must be computed.

                auto c0 = decode::delta_Clock_C0_r15(clock.delta_Clock_C0_r15);
                auto c1 = decode::delta_Clock_C2_r15(clock.delta_Clock_C1_r15);
                auto c2 = decode::delta_Clock_C1_r15(clock.delta_Clock_C2_r15);

                // t_0 = epochTime + (0.5 * ssrUpdateInterval)
                auto t0 =
                    corrections.epoch_time.seconds;  // TODO(ewasjon): include ssr update interval.
                auto t = t0;  // TODO(ewasjon): What should we do here? Why are we using t_0 == t?

                // delta_c = c_0 + c_1 (t - t_0) + [c_2 (t - t_0)]^2
                auto dt = t - t0;
                auto dc = c0 + c1 * dt + c2 * dt * dt;

                // NOTE(ewasjon): Geo++ observed that changing the sign of the clock corrections (in
                // there correction feed) improved the result. They assumed that u-blox implemented
                // it with a flipped sign. Thus, we also need to flip the sign to conform to the
                // u-blox implementation.
                if (true) {
                    dc *= -1;
                }

                builder.sf020(dc);
                builder.sf024_raw(0);  // TODO(ewasjon): calculate the URE
            }

            if (satellite.code_bias || satellite.phase_bias) {
                generate_gps_bias_block(builder, satellite.code_bias, satellite.phase_bias);
            }
        }

        auto message = builder.build();
        messages.push_back(message);
    }
}

namespace generator {
namespace spartn {

Generator::Generator() : mGenerationIndex(0) {}

Generator::~Generator() = default;

std::vector<Message> Generator::generate(const LPP_Message* lpp_message) {
    std::vector<Message> messages;
    if (!lpp_message) return messages;

    auto body = lpp_message->lpp_MessageBody;
    if (body->present != LPP_MessageBody_PR_c1) return messages;
    if (body->choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return messages;

    auto& pad = body->choice.c1.choice.provideAssistanceData;
    if (pad.criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1)
        return messages;
    if (pad.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return messages;

    // Initialze (and clear previous) OCB data
    mOcbData = std::unique_ptr<OcbData>(new OcbData());

    auto message = &pad.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    find_correction_point_set(message);
    find_ocb_corrections(message);

    for (auto kvp : mOcbData->mKeyedCorrections) {
        printf("OCB: gnss=%ld, iod=%ld\n", kvp.first.gnss_id, kvp.first.iod);
        printf("  orbit:      %p\n", kvp.second.orbit);
        printf("  clock:      %p\n", kvp.second.clock);
        printf("  code_bias:  %p\n", kvp.second.code_bias);
        printf("  phase_bias: %p\n", kvp.second.phase_bias);
        printf("  ura:        %p\n", kvp.second.ura);
    }

    generate_gad(messages, message);
    generate_ocb(messages, mOcbData.get());

    // Increment generation index
    mGenerationIndex++;
    return messages;
}

void Generator::find_correction_point_set(const ProvideAssistanceData_r9_IEs* message) {
    if (!message->a_gnss_ProvideAssistanceData) return;
    if (!message->a_gnss_ProvideAssistanceData->gnss_CommonAssistData) return;

    auto& cad = *message->a_gnss_ProvideAssistanceData->gnss_CommonAssistData;
    if (!cad.ext2) return;
    if (!cad.ext2->gnss_SSR_CorrectionPoints_r16) return;

    auto& ssr = *cad.ext2->gnss_SSR_CorrectionPoints_r16;
    if (ssr.correctionPoints_r16.present ==
        GNSS_SSR_CorrectionPoints_r16__correctionPoints_r16_PR_arrayOfCorrectionPoints_r16) {
        auto& array = ssr.correctionPoints_r16.choice.arrayOfCorrectionPoints_r16;

        CorrectionPointSet correction_point_set{};
        correction_point_set.id = ssr.correctionPointSetID_r16;
        correction_point_set.grid_points =
            (array.numberOfStepsLatitude_r16 + 1) * (array.numberOfStepsLongitude_r16 + 1);
        correction_point_set.referencePointLatitude_r16  = array.referencePointLatitude_r16;
        correction_point_set.referencePointLongitude_r16 = array.referencePointLongitude_r16;
        correction_point_set.numberOfStepsLatitude_r16   = array.numberOfStepsLatitude_r16;
        correction_point_set.numberOfStepsLongitude_r16  = array.numberOfStepsLongitude_r16;
        correction_point_set.stepOfLatitude_r16          = array.stepOfLatitude_r16;
        correction_point_set.stepOfLongitude_r16         = array.stepOfLongitude_r16;

        // TODO(ewasjon): how do we handle a new correction point set with a different id?
        mCorrectionPointSet =
            std::unique_ptr<CorrectionPointSet>(new CorrectionPointSet(correction_point_set));
    } else {
        // TODO(ewasjon): handle other types
    }
}

void Generator::find_ocb_corrections(const ProvideAssistanceData_r9_IEs* message) {
    if (!message->a_gnss_ProvideAssistanceData) return;
    if (!message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    auto& gad = *message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (int i = 0; i < gad.list.count; i++) {
        auto element = gad.list.array[i];
        if (!element) continue;

        auto gnss_id = element->gnss_ID.gnss_id;
        if (element->ext2) {
            mOcbData->add_correction(gnss_id, element->ext2->gnss_SSR_OrbitCorrections_r15);
            mOcbData->add_correction(gnss_id, element->ext2->gnss_SSR_ClockCorrections_r15);
            mOcbData->add_correction(gnss_id, element->ext2->gnss_SSR_CodeBias_r15);
        }

        if (element->ext3) {
            mOcbData->add_correction(gnss_id, element->ext3->gnss_SSR_PhaseBias_r16);
            mOcbData->add_correction(gnss_id, element->ext3->gnss_SSR_URA_r16);
        }
    }
}

}  // namespace spartn
}  // namespace generator

#include "generator.hpp"
#include <unordered_map>
#include "data.hpp"

#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>

static void generate_gad(std::vector<generator::spartn::Message>& messages,
                         const ProvideAssistanceData_r9_IEs*      message) {}

static void generate_ocb(std::vector<generator::spartn::Message>& messages,
                         const generator::spartn::OcbData*        data) {
    for (auto& kvp : mOcbData->mKeyedCorrections) {
        auto  iod_gnss    = kvp.first;
        auto& corrections = kvp.second;

        // List of SVs that are included in the OCB data
        auto sv_list = corrections.sv_list();
    
        
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

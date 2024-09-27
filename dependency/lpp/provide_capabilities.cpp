#include "lpp/provide_capabilities.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <ProvideCapabilities-r9-IEs.h>
#include <ProvideCapabilities.h>
#include <CommonIEsProvideCapabilities.h>
#include <GNSS-SupportElement.h>
#include <PositioningModes.h>
#include <GNSS-SupportList.h>
#include <A-GNSS-ProvideCapabilities.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>

#define ALLOC_ZERO(type) reinterpret_cast<type*>(calloc(1, sizeof(type)))

namespace lpp {

static CommonIEsProvideCapabilities* common_ies_provide_capabilities() {
    auto ext1 = ALLOC_ZERO(CommonIEsProvideCapabilities::CommonIEsProvideCapabilities__ext1);
// TODO(ewasjon): Make this configurable
#if 0
    ext1->lpp_message_segmentation_r14 = helper::BitStringBuilder{}
                                             .set(CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_serverToTarget)
                                             .set(CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_targetToServer)
                                             .to_bit_string(2);
#endif

    auto message  = ALLOC_ZERO(CommonIEsProvideCapabilities);
    message->ext1 = ext1;
    return message;
}

static GNSS_SupportElement* gnss_support_element(long gnss_id) {
    auto ha_gnss_modes_r15 = ALLOC_ZERO(PositioningModes);
    helper::BitStringBuilder{}
        .set(PositioningModes__posModes_ue_based)
        .into_bit_string(8, &ha_gnss_modes_r15->posModes);

    auto ext1               = ALLOC_ZERO(GNSS_SupportElement::GNSS_SupportElement__ext1);
    ext1->ha_gnss_Modes_r15 = ha_gnss_modes_r15;

    auto message             = ALLOC_ZERO(GNSS_SupportElement);
    message->gnss_ID.gnss_id = gnss_id;
    message->ext1            = ext1;

    // TODO(ewasjon): Are these needed?
    helper::BitStringBuilder{}
        .set(PositioningModes__posModes_ue_based)
        .set(PositioningModes__posModes_ue_assisted)
        .into_bit_string(8, &message->agnss_Modes.posModes);

    helper::BitStringBuilder{}
        .integer(0, 8, 0xFF)
        .into_bit_string(8, &message->gnss_Signals.gnss_SignalIDs);

    return message;
}

static GNSS_SupportList* gnss_support_list() {
    auto message = ALLOC_ZERO(GNSS_SupportList);
    asn_sequence_empty(&message->list);
    // TODO(ewasjon): Only send the GNSS IDs that are supported
    asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_gps));
    asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_glonass));
    asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_galileo));
    asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_bds));
    return message;
}

static A_GNSS_ProvideCapabilities* a_gnss_provide_capabilities() {
    auto ext2 = ALLOC_ZERO(A_GNSS_ProvideCapabilities::A_GNSS_ProvideCapabilities__ext2);
    ext2->periodicAssistanceData_r15 =
        helper::BitStringBuilder{}
            .set(A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_solicited)
            .set(A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_unsolicited)
            .to_bit_string(8);

    auto message = ALLOC_ZERO(A_GNSS_ProvideCapabilities);
    // TODO(ewasjon): message->assistanceDataSupportList = assistance_data_support_list();
    message->gnss_SupportList = gnss_support_list();
    message->ext2             = ext2;
    return message;
}

static void provide_capabilities_r9(ProvideCapabilities_r9_IEs& message) {
    message.commonIEsProvideCapabilities = common_ies_provide_capabilities();
    message.a_gnss_ProvideCapabilities   = a_gnss_provide_capabilities();
    // TODO(ewasjon): message.ecid_ProvideCapabilities     = ecid_provide_capabilities();
}

Message create_provide_capabilities() {
    auto body               = ALLOC_ZERO(LPP_MessageBody);
    body->present           = LPP_MessageBody_PR_c1;
    body->choice.c1.present = LPP_MessageBody__c1_PR_provideCapabilities;

    auto body_ce     = &body->choice.c1.choice.provideCapabilities.criticalExtensions;
    body_ce->present = ProvideCapabilities__criticalExtensions_PR_c1;
    body_ce->choice.c1.present =
        ProvideCapabilities__criticalExtensions__c1_PR_provideCapabilities_r9;

    auto body_ce_c1 = &body_ce->choice.c1.choice.provideCapabilities_r9;
    provide_capabilities_r9(*body_ce_c1);

    auto message             = ALLOC_ZERO(LPP_Message);
    message->lpp_MessageBody = body;
    return Message{message};
}

}  // namespace lpp

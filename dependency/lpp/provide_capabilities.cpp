#include "lpp/provide_capabilities.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <A-GNSS-ProvideCapabilities.h>
#include <AssistanceDataSupportList.h>
#include <CommonIEsProvideCapabilities.h>
#include <ECID-ProvideCapabilities.h>
#include <GLO-RTK-BiasInformationSupport-r15.h>
#include <GNSS-FrequencyID-r15.h>
#include <GNSS-GenericAssistDataSupportElement.h>
#include <GNSS-GenericAssistanceDataSupport.h>
#include <GNSS-Link-Combinations-r15.h>
#include <GNSS-Link-CombinationsList-r15.h>
#include <GNSS-RTK-ObservationsSupport-r15.h>
#include <GNSS-RTK-ReferenceStationInfoSupport-r15.h>
#include <GNSS-RTK-ResidualsSupport-r15.h>
#include <GNSS-SSR-ClockCorrectionsSupport-r15.h>
#include <GNSS-SSR-CodeBiasSupport-r15.h>
#include <GNSS-SSR-GriddedCorrectionSupport-r16.h>
#include <GNSS-SSR-OrbitCorrectionsSupport-r15.h>
#include <GNSS-SSR-PhaseBiasSupport-r16.h>
#include <GNSS-SSR-STEC-CorrectionSupport-r16.h>
#include <GNSS-SSR-URA-Support-r16.h>
#include <GNSS-SupportElement.h>
#include <GNSS-SupportList.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <PositioningModes.h>
#include <ProvideCapabilities-r9-IEs.h>
#include <ProvideCapabilities.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>

#define ALLOC_ZERO(type) reinterpret_cast<type*>(calloc(1, sizeof(type)))

namespace lpp {

static CommonIEsProvideCapabilities* common_ies_provide_capabilities(ProvideCapabilities const&) {
#if 0
    auto ext1 = ALLOC_ZERO(CommonIEsProvideCapabilities::CommonIEsProvideCapabilities__ext1);
// TODO(ewasjon): Make this configurable
    ext1->lpp_message_segmentation_r14 = helper::BitStringBuilder{}
                                             .set(CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_serverToTarget)
                                             .set(CommonIEsProvideCapabilities__ext1__lpp_message_segmentation_r14_targetToServer)
                                             .to_bit_string(2);
#endif

    auto message = ALLOC_ZERO(CommonIEsProvideCapabilities);
#if 0
    message->ext1 = ext1;
#endif
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

static GNSS_SupportList* gnss_support_list(ProvideCapabilities const& capabilities) {
    auto message = ALLOC_ZERO(GNSS_SupportList);
    asn_sequence_empty(&message->list);
    if (capabilities.gnss.gps)
        asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_gps));
    if (capabilities.gnss.glonass)
        asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_glonass));
    if (capabilities.gnss.galileo)
        asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_galileo));
    if (capabilities.gnss.beidou)
        asn_sequence_add(&message->list, gnss_support_element(GNSS_ID__gnss_id_bds));
    return message;
}

static GNSS_RTK_ReferenceStationInfoSupport_r15*
gnss_rtk_reference_station_info_support_r15(ProvideCapabilities const&) {
    return ALLOC_ZERO(GNSS_RTK_ReferenceStationInfoSupport_r15);
}

static GNSS_CommonAssistanceDataSupport
gnss_common_assistance_data_support(ProvideCapabilities const& capabilities) {
    GNSS_CommonAssistanceDataSupport message{};

    if (capabilities.assistance_data.osr) {
        if (!message.ext1)
            message.ext1 = ALLOC_ZERO(
                GNSS_CommonAssistanceDataSupport::GNSS_CommonAssistanceDataSupport__ext1);
        message.ext1->gnss_RTK_ReferenceStationInfoSupport_r15 =
            gnss_rtk_reference_station_info_support_r15(capabilities);
    }

    if (capabilities.assistance_data.ssr) {
        // ...
    }

    return message;
}

static void gnss_signal_ids_fill(GNSS_SignalIDs& signals) {
    helper::BitStringBuilder{}.integer(0, 8, 0xFF).into_bit_string(8, &signals.gnss_SignalIDs);

    if (!signals.ext1) signals.ext1 = ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
    signals.ext1->gnss_SignalIDs_Ext_r15 = ALLOC_ZERO(BIT_STRING_t);

    helper::BitStringBuilder{}
        .integer(0, 8, 0xFF)
        .into_bit_string(8, signals.ext1->gnss_SignalIDs_Ext_r15);
}

static GNSS_RTK_ObservationsSupport_r15*
gnss_rtk_observations_support_r15(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_RTK_ObservationsSupport_r15);
    gnss_signal_ids_fill(message->gnssSignalIDs_r15);
    return message;
}

static GNSS_RTK_ResidualsSupport_r15* gnss_rtk_residuals_support_r15(long,
                                                                     ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_RTK_ResidualsSupport_r15);
    asn_sequence_empty(&message->link_combinations_support_r15);

    auto link_comb                         = ALLOC_ZERO(GNSS_Link_Combinations_r15);
    link_comb->l1_r15.gnss_FrequencyID_r15 = 0;
    link_comb->l2_r15.gnss_FrequencyID_r15 = 1;
    asn_sequence_add(&message->link_combinations_support_r15, link_comb);

    return message;
}

static GLO_RTK_BiasInformationSupport_r15*
glo_rtk_bias_information_support_r15(long gnss_id, ProvideCapabilities const&) {
    if (gnss_id != GNSS_ID__gnss_id_glonass) return nullptr;

    auto message = ALLOC_ZERO(GLO_RTK_BiasInformationSupport_r15);
    return message;
}

static GNSS_SSR_ClockCorrectionsSupport_r15*
gnss_ssr_clock_corrections_support_r15(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_ClockCorrectionsSupport_r15);
    return message;
}

static GNSS_SSR_OrbitCorrectionsSupport_r15*
gnss_ssr_orbit_corrections_support_r15(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_OrbitCorrectionsSupport_r15);
    return message;
}

static GNSS_SSR_CodeBiasSupport_r15* gnss_ssr_code_bias_support_r15(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_CodeBiasSupport_r15);
    gnss_signal_ids_fill(message->signal_and_tracking_mode_ID_Sup_r15);
    return message;
}

static GNSS_SSR_PhaseBiasSupport_r16* gnss_ssr_phase_bias_support_r16(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_PhaseBiasSupport_r16);
    gnss_signal_ids_fill(message->signal_and_tracking_mode_ID_Sup_r16);
    return message;
}

static GNSS_SSR_URA_Support_r16* gnss_ssr_ura_support_r16(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_URA_Support_r16);
    return message;
}

static GNSS_SSR_STEC_CorrectionSupport_r16*
gnss_ssr_stec_correction_support_r16(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_STEC_CorrectionSupport_r16);
    return message;
}

static GNSS_SSR_GriddedCorrectionSupport_r16*
gnss_ssr_gridded_correction_support_r16(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(GNSS_SSR_GriddedCorrectionSupport_r16);
    return message;
}

static GNSS_GenericAssistDataSupportElement*
gnss_generic_assist_data_support_element(long gnss_id, ProvideCapabilities const& capabilities) {
    auto message             = ALLOC_ZERO(GNSS_GenericAssistDataSupportElement);
    message->gnss_ID.gnss_id = gnss_id;

    if (capabilities.assistance_data.osr) {
        if (!message->ext2)
            message->ext2 = ALLOC_ZERO(
                GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext2);
        message->ext2->gnss_RTK_ObservationsSupport_r15 =
            gnss_rtk_observations_support_r15(capabilities);
        message->ext2->gnss_RTK_ResidualsSupport_r15 =
            gnss_rtk_residuals_support_r15(gnss_id, capabilities);
        message->ext2->glo_RTK_BiasInformationSupport_r15 =
            glo_rtk_bias_information_support_r15(gnss_id, capabilities);
    }

    if (capabilities.assistance_data.ssr) {
        if (!message->ext2)
            message->ext2 = ALLOC_ZERO(
                GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext2);
        if (!message->ext3)
            message->ext3 = ALLOC_ZERO(
                GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext3);

        message->ext2->gnss_SSR_ClockCorrectionsSupport_r15 =
            gnss_ssr_clock_corrections_support_r15(capabilities);
        message->ext2->gnss_SSR_OrbitCorrectionsSupport_r15 =
            gnss_ssr_orbit_corrections_support_r15(capabilities);
        message->ext2->gnss_SSR_CodeBiasSupport_r15 = gnss_ssr_code_bias_support_r15(capabilities);
        message->ext3->gnss_SSR_PhaseBiasSupport_r16 =
            gnss_ssr_phase_bias_support_r16(capabilities);
        message->ext3->gnss_SSR_URA_Support_r16 = gnss_ssr_ura_support_r16(capabilities);
        message->ext3->gnss_SSR_STEC_CorrectionSupport_r16 =
            gnss_ssr_stec_correction_support_r16(capabilities);
        message->ext3->gnss_SSR_GriddedCorrectionSupport_r16 =
            gnss_ssr_gridded_correction_support_r16(capabilities);
    }

    return message;
}

static GNSS_GenericAssistanceDataSupport
gnss_generic_assistance_data_support(ProvideCapabilities const& capabilities) {
    GNSS_GenericAssistanceDataSupport message{};
    asn_sequence_empty(&message.list);

    if (capabilities.gnss.gps)
        asn_sequence_add(&message.list, gnss_generic_assist_data_support_element(
                                            GNSS_ID__gnss_id_gps, capabilities));
    if (capabilities.gnss.glonass)
        asn_sequence_add(&message.list, gnss_generic_assist_data_support_element(
                                            GNSS_ID__gnss_id_glonass, capabilities));
    if (capabilities.gnss.galileo)
        asn_sequence_add(&message.list, gnss_generic_assist_data_support_element(
                                            GNSS_ID__gnss_id_galileo, capabilities));
    if (capabilities.gnss.beidou)
        asn_sequence_add(&message.list, gnss_generic_assist_data_support_element(
                                            GNSS_ID__gnss_id_bds, capabilities));

    return message;
}

static AssistanceDataSupportList*
assistance_data_support_list(ProvideCapabilities const& capabilities) {
    auto message                               = ALLOC_ZERO(AssistanceDataSupportList);
    message->gnss_CommonAssistanceDataSupport  = gnss_common_assistance_data_support(capabilities);
    message->gnss_GenericAssistanceDataSupport = gnss_generic_assistance_data_support(capabilities);
    return message;
}

static A_GNSS_ProvideCapabilities*
a_gnss_provide_capabilities(ProvideCapabilities const& capabilities) {
    auto ext2 = ALLOC_ZERO(A_GNSS_ProvideCapabilities::A_GNSS_ProvideCapabilities__ext2);
    ext2->periodicAssistanceData_r15 =
        helper::BitStringBuilder{}
            .set(A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_solicited)
            .set(A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_unsolicited)
            .to_bit_string(8);

    auto message                       = ALLOC_ZERO(A_GNSS_ProvideCapabilities);
    message->assistanceDataSupportList = assistance_data_support_list(capabilities);
    message->gnss_SupportList          = gnss_support_list(capabilities);
    message->ext2                      = ext2;
    return message;
}

static ECID_ProvideCapabilities* ecid_provide_capabilities(ProvideCapabilities const&) {
    auto message = ALLOC_ZERO(ECID_ProvideCapabilities);

    helper::BitStringBuilder{}
        .clear(ECID_ProvideCapabilities__ecid_MeasSupported_rsrpSup)
        .clear(ECID_ProvideCapabilities__ecid_MeasSupported_rsrqSup)
        .clear(ECID_ProvideCapabilities__ecid_MeasSupported_ueRxTxSup)
        .clear(ECID_ProvideCapabilities__ecid_MeasSupported_nrsrpSup_r14)
        .clear(ECID_ProvideCapabilities__ecid_MeasSupported_nrsrqSup_r14)
        .into_bit_string(8, &message->ecid_MeasSupported);

    return message;
}

static void provide_capabilities_r9(ProvideCapabilities_r9_IEs& message,
                                    ProvideCapabilities const&  capabilities) {
    message.commonIEsProvideCapabilities = common_ies_provide_capabilities(capabilities);
    message.a_gnss_ProvideCapabilities   = a_gnss_provide_capabilities(capabilities);
    message.ecid_ProvideCapabilities     = ecid_provide_capabilities(capabilities);
}

Message create_provide_capabilities(ProvideCapabilities const& capabilities) {
    auto body               = ALLOC_ZERO(LPP_MessageBody);
    body->present           = LPP_MessageBody_PR_c1;
    body->choice.c1.present = LPP_MessageBody__c1_PR_provideCapabilities;

    auto body_ce     = &body->choice.c1.choice.provideCapabilities.criticalExtensions;
    body_ce->present = ProvideCapabilities__criticalExtensions_PR_c1;
    body_ce->choice.c1.present =
        ProvideCapabilities__criticalExtensions__c1_PR_provideCapabilities_r9;

    auto body_ce_c1 = &body_ce->choice.c1.choice.provideCapabilities_r9;
    provide_capabilities_r9(*body_ce_c1, capabilities);

    auto message             = ALLOC_ZERO(LPP_Message);
    message->lpp_MessageBody = body;
    return Message{message};
}

}  // namespace lpp

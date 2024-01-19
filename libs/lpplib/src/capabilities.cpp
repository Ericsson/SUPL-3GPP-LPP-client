#include "internal_lpp.h"
#include "lpp.h"

static GNSS_SupportElement* build_support_element(long gnss_id) {
    auto element                        = ALLOC_ZERO(GNSS_SupportElement);
    element->gnss_ID.gnss_id            = GNSS_ID__gnss_id_gps;
    element->adr_Support                = false;
    element->velocityMeasurementSupport = true;

    BitStringBuilder{}
        .set(7 - PositioningModes__posModes_ue_based)
        .set(7 - PositioningModes__posModes_ue_assisted)
        .into_bit_string(8, &element->agnss_Modes.posModes);

    auto ha_gnss_modes_r15 = ALLOC_ZERO(PositioningModes);
    BitStringBuilder{}
        .set(7 - PositioningModes__posModes_ue_based)
        .set(7 - PositioningModes__posModes_ue_assisted)
        .into_bit_string(8, &ha_gnss_modes_r15->posModes);

    auto ext               = ALLOC_ZERO(GNSS_SupportElement::GNSS_SupportElement__ext1);
    ext->ha_gnss_Modes_r15 = ha_gnss_modes_r15;
    element->ext1          = ext;

    BitStringBuilder{}
        .set_int(0, 8, 0xFF)
        .into_bit_string(8, &element->gnss_Signals.gnss_SignalIDs);

    return element;
}

static GNSS_SupportList* build_support_list() {
    auto gnss_list = ALLOC_ZERO(GNSS_SupportList);
    asn_sequence_empty(&gnss_list->list);
    asn_sequence_add(&gnss_list->list, build_support_element(GNSS_ID__gnss_id_gps));
    asn_sequence_add(&gnss_list->list, build_support_element(GNSS_ID__gnss_id_glonass));
    asn_sequence_add(&gnss_list->list, build_support_element(GNSS_ID__gnss_id_galileo));
    asn_sequence_add(&gnss_list->list, build_support_element(GNSS_ID__gnss_id_bds));
    return gnss_list;
}

static AssistanceDataSupportList* build_assistance_support() {
    auto as = ALLOC_ZERO(AssistanceDataSupportList);

    // GNSS Common AD Support
    auto as_ext1 =
        ALLOC_ZERO(GNSS_CommonAssistanceDataSupport::GNSS_CommonAssistanceDataSupport__ext1);

    auto rtk_reference_station_support = ALLOC_ZERO(GNSS_RTK_ReferenceStationInfoSupport_r15);
    auto rtk_auxiliary_station_support = ALLOC_ZERO(GNSS_RTK_AuxiliaryStationDataSupport_r15);
    as_ext1->gnss_RTK_ReferenceStationInfoSupport_r15 = rtk_reference_station_support;
    as_ext1->gnss_RTK_AuxiliaryStationDataSupport_r15 = rtk_auxiliary_station_support;
    as->gnss_CommonAssistanceDataSupport.ext1         = as_ext1;

    // GNSS Generic AD Support
    auto generic_assistance = &as->gnss_GenericAssistanceDataSupport;
    asn_sequence_empty(&generic_assistance->list);

    {
        auto gps_assistance             = ALLOC_ZERO(GNSS_GenericAssistDataSupportElement);
        gps_assistance->gnss_ID.gnss_id = GNSS_ID__gnss_id_gps;
        gps_assistance->gnss_AcquisitionAssistanceSupport =
            ALLOC_ZERO(GNSS_AcquisitionAssistanceSupport);

        auto ext = ALLOC_ZERO(
            GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext2);
        gps_assistance->ext2                  = ext;
        ext->gnss_RTK_ObservationsSupport_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsSupport_r15_t);
        auto signals                          = BitString::allocate(
            8, &ext->gnss_RTK_ObservationsSupport_r15->gnssSignalIDs_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        ext->gnss_RTK_ResidualsSupport_r15 = ALLOC_ZERO(GNSS_RTK_ResidualsSupport_r15_t);
        GNSS_Link_CombinationsList_r15_t* link_list =
            &ext->gnss_RTK_ResidualsSupport_r15->link_combinations_support_r15;
        asn_sequence_empty(&link_list->list);

        {
            GNSS_Link_Combinations_r15_t* link = ALLOC_ZERO(GNSS_Link_Combinations_r15_t);
            link->l1_r15.gnss_FrequencyID_r15  = 0;
            link->l2_r15.gnss_FrequencyID_r15  = 1;
            asn_sequence_add(&link_list->list, link);
        }

        asn_sequence_add(&generic_assistance->list, gps_assistance);
    }

    {
        auto glonass_assistance             = ALLOC_ZERO(GNSS_GenericAssistDataSupportElement_t);
        glonass_assistance->gnss_ID.gnss_id = GNSS_ID__gnss_id_glonass;
        glonass_assistance->gnss_AcquisitionAssistanceSupport =
            ALLOC_ZERO(GNSS_AcquisitionAssistanceSupport_t);

        auto ext = ALLOC_ZERO(
            GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext2);
        glonass_assistance->ext2              = ext;
        ext->gnss_RTK_ObservationsSupport_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsSupport_r15_t);
        auto signals                          = BitString::allocate(
            8, &ext->gnss_RTK_ObservationsSupport_r15->gnssSignalIDs_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        ext->glo_RTK_BiasInformationSupport_r15 = ALLOC_ZERO(GLO_RTK_BiasInformationSupport_r15_t);
        ext->gnss_RTK_ResidualsSupport_r15      = ALLOC_ZERO(GNSS_RTK_ResidualsSupport_r15_t);
        auto link_list = &ext->gnss_RTK_ResidualsSupport_r15->link_combinations_support_r15;
        asn_sequence_empty(&link_list->list);

        {
            auto link                         = ALLOC_ZERO(GNSS_Link_Combinations_r15_t);
            link->l1_r15.gnss_FrequencyID_r15 = 0;
            link->l2_r15.gnss_FrequencyID_r15 = 1;
            asn_sequence_add(&link_list->list, link);
        }

        asn_sequence_add(&generic_assistance->list, glonass_assistance);
    }

    {
        auto galileo_assistance             = ALLOC_ZERO(GNSS_GenericAssistDataSupportElement);
        galileo_assistance->gnss_ID.gnss_id = GNSS_ID__gnss_id_galileo;
        galileo_assistance->gnss_AcquisitionAssistanceSupport =
            ALLOC_ZERO(GNSS_AcquisitionAssistanceSupport);

        auto ext = ALLOC_ZERO(
            GNSS_GenericAssistDataSupportElement::GNSS_GenericAssistDataSupportElement__ext2);
        galileo_assistance->ext2              = ext;
        ext->gnss_RTK_ObservationsSupport_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsSupport_r15_t);
        auto signals                          = BitString::allocate(
            8, &ext->gnss_RTK_ObservationsSupport_r15->gnssSignalIDs_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        ext->gnss_RTK_ResidualsSupport_r15 = ALLOC_ZERO(GNSS_RTK_ResidualsSupport_r15_t);
        auto link_list = &ext->gnss_RTK_ResidualsSupport_r15->link_combinations_support_r15;
        asn_sequence_empty(&link_list->list);

        {
            GNSS_Link_Combinations_r15_t* link = ALLOC_ZERO(GNSS_Link_Combinations_r15_t);
            link->l1_r15.gnss_FrequencyID_r15  = 0;
            link->l2_r15.gnss_FrequencyID_r15  = 1;
            asn_sequence_add(&link_list->list, link);
        }

        asn_sequence_add(&generic_assistance->list, galileo_assistance);
    }

    return as;
}

LPP_Message* lpp_provide_capabilities(LPP_Transaction* transaction, bool segmentation) {
    auto message = lpp_create(transaction, LPP_MessageBody__c1_PR_provideCapabilities);
    auto body    = message->lpp_MessageBody;

    auto cext               = &body->choice.c1.choice.provideCapabilities.criticalExtensions;
    cext->present           = ProvideCapabilities__criticalExtensions_PR_c1;
    cext->choice.c1.present = ProvideCapabilities__criticalExtensions__c1_PR_provideCapabilities_r9;

    auto ext2 = ALLOC_ZERO(A_GNSS_ProvideCapabilities::A_GNSS_ProvideCapabilities__ext2);

    auto pad =
        BitStringBuilder{}
            .set(7 - A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_solicited)
            .set(7 - A_GNSS_ProvideCapabilities__ext2__periodicAssistanceData_r15_unsolicited)
            .to_bit_string(8);
    ext2->periodicAssistanceData_r15 = pad;

    auto gnss_capabilities                       = ALLOC_ZERO(A_GNSS_ProvideCapabilities);
    gnss_capabilities->assistanceDataSupportList = build_assistance_support();
    gnss_capabilities->gnss_SupportList          = build_support_list();
    gnss_capabilities->ext2                      = ext2;

    auto ecid_capabilities = ALLOC_ZERO(ECID_ProvideCapabilities);

    BitStringBuilder{}
        .set(7 - ECID_ProvideCapabilities__ecid_MeasSupported_rsrpSup)
        .set(7 - ECID_ProvideCapabilities__ecid_MeasSupported_rsrqSup)
        .set(7 - ECID_ProvideCapabilities__ecid_MeasSupported_ueRxTxSup)
        .set(7 - ECID_ProvideCapabilities__ecid_MeasSupported_nrsrpSup_r14)
        .set(7 - ECID_ProvideCapabilities__ecid_MeasSupported_nrsrqSup_r14)
        .into_bit_string(8, &ecid_capabilities->ecid_MeasSupported);

    auto common_capabilities = ALLOC_ZERO(CommonIEsProvideCapabilities);
    auto cc_ext1 = ALLOC_ZERO(CommonIEsProvideCapabilities::CommonIEsProvideCapabilities__ext1);
    auto segmentation_support = BitString::allocate(2);
    if (segmentation) {
        segmentation_support->set_bit(0);  // Support receiving segmented messages
    }
    cc_ext1->lpp_message_segmentation_r14 = segmentation_support;
    common_capabilities->ext1             = cc_ext1;
    // segmentation

    auto pcr9                          = &cext->choice.c1.choice.provideCapabilities_r9;
    pcr9->commonIEsProvideCapabilities = common_capabilities;
    pcr9->a_gnss_ProvideCapabilities   = gnss_capabilities;
    pcr9->ecid_ProvideCapabilities     = ecid_capabilities;

    return message;
}
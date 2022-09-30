#include "internal_lpp.h"
#include "lpp.h"

LPP_Message* lpp_request_assistance_data(LPP_Transaction* transaction, CellID cell,
                                         long periodic_id, long interval) {
    auto message = lpp_create(transaction, LPP_MessageBody__c1_PR_requestAssistanceData);
    auto body    = message->lpp_MessageBody;

    auto cext     = &body->choice.c1.choice.requestAssistanceData.criticalExtensions;
    cext->present = RequestAssistanceData__criticalExtensions_PR_c1;
    cext->choice.c1.present =
        RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9;

    auto pad1 = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15::
                               PeriodicAssistanceDataControlParameters_r15__ext1);

    auto pad1_update_capabilities = BitString::allocate(1);
    pad1_update_capabilities->set_bit(UpdateCapabilities_r15_primaryCellID_r15);

    pad1->updateCapabilities_r15 = pad1_update_capabilities;

    auto pad = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15);
    pad->periodicSessionID_r15.periodicSessionInitiator_r15 =
        PeriodicSessionID_r15__periodicSessionInitiator_r15_targetDevice;
    pad->periodicSessionID_r15.periodicSessionNumber_r15 = periodic_id;
    pad->ext1                                            = pad1;

    auto cad2 = ALLOC_ZERO(CommonIEsRequestAssistanceData::CommonIEsRequestAssistanceData__ext2);
    cad2->periodicAssistanceDataReq_r15 = pad;

    auto cad           = ALLOC_ZERO(CommonIEsRequestAssistanceData);
    cad->primaryCellID = ecgi_create(cell.mcc, cell.mnc, cell.cell);
    cad->ext2          = cad2;

    auto car1_aux                  = ALLOC_ZERO(GNSS_RTK_AuxiliaryStationDataReq_r15);
    auto car1_ref                  = ALLOC_ZERO(GNSS_RTK_ReferenceStationInfoReq_r15);
    car1_ref->antennaHeightReq_r15 = true;

    auto car1 = ALLOC_ZERO(GNSS_CommonAssistDataReq::GNSS_CommonAssistDataReq__ext1);
    car1->gnss_RTK_AuxiliaryStationDataReq_r15 = car1_aux;
    car1->gnss_RTK_ReferenceStationInfoReq_r15 = car1_ref;

    auto car_time = ALLOC_ZERO(GNSS_ReferenceTimeReq_t);
    asn_sequence_empty(&car_time->gnss_TimeReqPrefList.list);
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_gps;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_glonass;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_galileo;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }

    auto car                   = ALLOC_ZERO(GNSS_CommonAssistDataReq_t);
    car->gnss_ReferenceTimeReq = car_time;
    car->ext1                  = car1;

    // GENERIC
    auto gar = ALLOC_ZERO(GNSS_GenericAssistDataReq_t);
    asn_sequence_empty(&gar->list);

    // GPS
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_RTK_ResidualsReq_r15    = ALLOC_ZERO(GNSS_RTK_ResidualsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_Integer_ms_Req_r15    = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_PhaseRangeRateReq_r15 = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_CNR_Req_r15           = true;
        auto signals = BitString::allocate(
            8, &element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        auto ext_signals = BitString::allocate(16);
        for (size_t i = 0; i < 16; i++) {
            ext_signals->set_bit(i);
        }

        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1 =
            ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1
            ->gnss_SignalIDs_Ext_r15 = ext_signals;

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_gps;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        asn_sequence_add(&gar->list, element);
    }

    // GLONASS
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->glo_RTK_BiasInformationReq_r15 = ALLOC_ZERO(GLO_RTK_BiasInformationReq_r15);
        element2->gnss_RTK_ResidualsReq_r15      = ALLOC_ZERO(GNSS_RTK_ResidualsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15   = ALLOC_ZERO(GNSS_RTK_ObservationsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_Integer_ms_Req_r15    = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_PhaseRangeRateReq_r15 = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_CNR_Req_r15           = true;
        auto signals = BitString::allocate(
            8, &element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        auto ext_signals = BitString::allocate(16);
        for (size_t i = 0; i < 16; i++) {
            ext_signals->set_bit(i);
        }

        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1 =
            ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1
            ->gnss_SignalIDs_Ext_r15 = ext_signals;

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_glonass;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        asn_sequence_add(&gar->list, element);
    }

    // GALILEO
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_RTK_ObservationsReq_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_Integer_ms_Req_r15    = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_PhaseRangeRateReq_r15 = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_CNR_Req_r15           = true;
        auto signals = BitString::allocate(
            8, &element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        auto ext_signals = BitString::allocate(16);
        for (size_t i = 0; i < 16; i++) {
            ext_signals->set_bit(i);
        }

        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1 =
            ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1
            ->gnss_SignalIDs_Ext_r15 = ext_signals;

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_galileo;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        asn_sequence_add(&gar->list, element);
    }

    // BEIDOU
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_RTK_ObservationsReq_r15 = ALLOC_ZERO(GNSS_RTK_ObservationsReq_r15);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_Integer_ms_Req_r15    = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_PhaseRangeRateReq_r15 = true;
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_CNR_Req_r15           = true;
        auto signals = BitString::allocate(
            8, &element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.gnss_SignalIDs);
        signals->set_bit(0);
        signals->set_bit(1);
        signals->set_bit(2);
        signals->set_bit(3);
        signals->set_bit(4);
        signals->set_bit(5);
        signals->set_bit(6);
        signals->set_bit(7);

        auto ext_signals = BitString::allocate(16);
        for (size_t i = 0; i < 16; i++) {
            ext_signals->set_bit(i);
        }

        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1 =
            ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
        element2->gnss_RTK_ObservationsReq_r15->gnss_RTK_SignalsReq_r15.ext1
            ->gnss_SignalIDs_Ext_r15 = ext_signals;

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_bds;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        asn_sequence_add(&gar->list, element);
    }

    auto padq = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15_t);

    padq->gnss_RTK_PeriodicObservationsReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->gnss_RTK_PeriodicObservationsReq_r15->deliveryAmount_r15   = 32;
    padq->gnss_RTK_PeriodicObservationsReq_r15->deliveryInterval_r15 = interval;

    padq->gnss_RTK_PeriodicResidualsReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->gnss_RTK_PeriodicResidualsReq_r15->deliveryAmount_r15   = 32;
    padq->gnss_RTK_PeriodicResidualsReq_r15->deliveryInterval_r15 = interval;

    padq->glo_RTK_PeriodicBiasInformationReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->glo_RTK_PeriodicBiasInformationReq_r15->deliveryAmount_r15   = 32;
    padq->glo_RTK_PeriodicBiasInformationReq_r15->deliveryInterval_r15 = interval;

    auto gad1 = ALLOC_ZERO(A_GNSS_RequestAssistanceData::A_GNSS_RequestAssistanceData__ext1);
    gad1->gnss_PeriodicAssistDataReq_r15 = padq;

    auto gad                       = ALLOC_ZERO(A_GNSS_RequestAssistanceData_t);
    gad->ext1                      = gad1;
    gad->gnss_CommonAssistDataReq  = car;
    gad->gnss_GenericAssistDataReq = gar;

    auto rad9                            = &cext->choice.c1.choice.requestAssistanceData_r9;
    rad9->commonIEsRequestAssistanceData = cad;
    rad9->a_gnss_RequestAssistanceData   = gad;

    return message;
}

LPP_Message* lpp_request_assistance_data_ssr(LPP_Transaction* transaction, CellID cell,
                                             long periodic_id, long interval) {
    auto message = lpp_create(transaction, LPP_MessageBody__c1_PR_requestAssistanceData);
    auto body    = message->lpp_MessageBody;

    auto cext     = &body->choice.c1.choice.requestAssistanceData.criticalExtensions;
    cext->present = RequestAssistanceData__criticalExtensions_PR_c1;
    cext->choice.c1.present =
        RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9;

    auto pad1 = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15::
                               PeriodicAssistanceDataControlParameters_r15__ext1);

    auto pad1_update_capabilities = BitString::allocate(1);
    pad1_update_capabilities->set_bit(UpdateCapabilities_r15_primaryCellID_r15);

    pad1->updateCapabilities_r15 = pad1_update_capabilities;

    auto pad = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15);
    pad->periodicSessionID_r15.periodicSessionInitiator_r15 =
        PeriodicSessionID_r15__periodicSessionInitiator_r15_targetDevice;
    pad->periodicSessionID_r15.periodicSessionNumber_r15 = periodic_id;
    pad->ext1                                            = pad1;

    auto cad2 = ALLOC_ZERO(CommonIEsRequestAssistanceData::CommonIEsRequestAssistanceData__ext2);
    cad2->periodicAssistanceDataReq_r15 = pad;

    auto cad           = ALLOC_ZERO(CommonIEsRequestAssistanceData);
    cad->primaryCellID = ecgi_create(cell.mcc, cell.mnc, cell.cell);
    cad->ext2          = cad2;

    auto car2_ssr = ALLOC_ZERO(GNSS_SSR_CorrectionPointsReq_r16);

    auto car2 = ALLOC_ZERO(GNSS_CommonAssistDataReq::GNSS_CommonAssistDataReq__ext2);
    car2->gnss_SSR_CorrectionPointsReq_r16 = car2_ssr;

    auto car_time = ALLOC_ZERO(GNSS_ReferenceTimeReq_t);
    asn_sequence_empty(&car_time->gnss_TimeReqPrefList.list);
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_gps;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_glonass;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }
    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = GNSS_ID__gnss_id_galileo;
        asn_sequence_add(&car_time->gnss_TimeReqPrefList.list, id);
    }

    auto car                   = ALLOC_ZERO(GNSS_CommonAssistDataReq_t);
    car->gnss_ReferenceTimeReq = car_time;
    car->ext2                  = car2;

    // GENERIC
    auto gar = ALLOC_ZERO(GNSS_GenericAssistDataReq_t);
    asn_sequence_empty(&gar->list);

    // GPS
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_SSR_ClockCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_ClockCorrectionsReq_r15);
        element2->gnss_SSR_OrbitCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_OrbitCorrectionsReq_r15);
        element2->gnss_SSR_CodeBiasReq_r15         = ALLOC_ZERO(GNSS_SSR_CodeBiasReq_r15);
        auto codebias_signals =
            BitString::allocate(8, &element2->gnss_SSR_CodeBiasReq_r15
                                        ->signal_and_tracking_mode_ID_Map_r15.gnss_SignalIDs);
        codebias_signals->set_bit(0);
        codebias_signals->set_bit(1);
        codebias_signals->set_bit(2);
        codebias_signals->set_bit(3);
        codebias_signals->set_bit(4);
        codebias_signals->set_bit(5);
        codebias_signals->set_bit(6);
        codebias_signals->set_bit(7);

        auto element3 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext3);
        element3->gnss_SSR_PhaseBiasReq_r16 = ALLOC_ZERO(GNSS_SSR_PhaseBiasReq_r16);
        auto phasebias_signals =
            BitString::allocate(8, &element3->gnss_SSR_PhaseBiasReq_r16
                                        ->signal_and_tracking_mode_ID_Map_r16.gnss_SignalIDs);
        phasebias_signals->set_bit(0);
        phasebias_signals->set_bit(1);
        phasebias_signals->set_bit(2);
        phasebias_signals->set_bit(3);
        phasebias_signals->set_bit(4);
        phasebias_signals->set_bit(5);
        phasebias_signals->set_bit(6);
        phasebias_signals->set_bit(7);

        element3->gnss_SSR_GriddedCorrectionReq_r16 = ALLOC_ZERO(GNSS_SSR_GriddedCorrectionReq_r16);
        element3->gnss_SSR_STEC_CorrectionReq_r16   = ALLOC_ZERO(GNSS_SSR_STEC_CorrectionReq_r16);
        element3->gnss_SSR_URA_Req_r16              = ALLOC_ZERO(GNSS_SSR_URA_Req_r16);

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_gps;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        element->ext3                         = element3;
        asn_sequence_add(&gar->list, element);
    }

    // GLONASS
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_SSR_ClockCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_ClockCorrectionsReq_r15);
        element2->gnss_SSR_OrbitCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_OrbitCorrectionsReq_r15);
        element2->gnss_SSR_CodeBiasReq_r15         = ALLOC_ZERO(GNSS_SSR_CodeBiasReq_r15);
        auto codebias_signals =
            BitString::allocate(8, &element2->gnss_SSR_CodeBiasReq_r15
                                        ->signal_and_tracking_mode_ID_Map_r15.gnss_SignalIDs);
        codebias_signals->set_bit(0);
        codebias_signals->set_bit(1);
        codebias_signals->set_bit(2);
        codebias_signals->set_bit(3);
        codebias_signals->set_bit(4);
        codebias_signals->set_bit(5);
        codebias_signals->set_bit(6);
        codebias_signals->set_bit(7);

        auto element3 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext3);
        element3->gnss_SSR_PhaseBiasReq_r16 = ALLOC_ZERO(GNSS_SSR_PhaseBiasReq_r16);
        auto phasebias_signals =
            BitString::allocate(8, &element3->gnss_SSR_PhaseBiasReq_r16
                                        ->signal_and_tracking_mode_ID_Map_r16.gnss_SignalIDs);
        phasebias_signals->set_bit(0);
        phasebias_signals->set_bit(1);
        phasebias_signals->set_bit(2);
        phasebias_signals->set_bit(3);
        phasebias_signals->set_bit(4);
        phasebias_signals->set_bit(5);
        phasebias_signals->set_bit(6);
        phasebias_signals->set_bit(7);

        element3->gnss_SSR_GriddedCorrectionReq_r16 = ALLOC_ZERO(GNSS_SSR_GriddedCorrectionReq_r16);
        element3->gnss_SSR_STEC_CorrectionReq_r16   = ALLOC_ZERO(GNSS_SSR_STEC_CorrectionReq_r16);
        element3->gnss_SSR_URA_Req_r16              = ALLOC_ZERO(GNSS_SSR_URA_Req_r16);

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_glonass;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        element->ext3                         = element3;
        asn_sequence_add(&gar->list, element);
    }

    // GALILEO
    {
        auto element2 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
        element2->gnss_SSR_ClockCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_ClockCorrectionsReq_r15);
        element2->gnss_SSR_OrbitCorrectionsReq_r15 = ALLOC_ZERO(GNSS_SSR_OrbitCorrectionsReq_r15);
        element2->gnss_SSR_CodeBiasReq_r15         = ALLOC_ZERO(GNSS_SSR_CodeBiasReq_r15);
        auto codebias_signals =
            BitString::allocate(8, &element2->gnss_SSR_CodeBiasReq_r15
                                        ->signal_and_tracking_mode_ID_Map_r15.gnss_SignalIDs);
        codebias_signals->set_bit(0);
        codebias_signals->set_bit(1);
        codebias_signals->set_bit(2);
        codebias_signals->set_bit(3);
        codebias_signals->set_bit(4);
        codebias_signals->set_bit(5);
        codebias_signals->set_bit(6);
        codebias_signals->set_bit(7);

        auto element3 =
            ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext3);
        element3->gnss_SSR_PhaseBiasReq_r16 = ALLOC_ZERO(GNSS_SSR_PhaseBiasReq_r16);
        auto phasebias_signals =
            BitString::allocate(8, &element3->gnss_SSR_PhaseBiasReq_r16
                                        ->signal_and_tracking_mode_ID_Map_r16.gnss_SignalIDs);
        phasebias_signals->set_bit(0);
        phasebias_signals->set_bit(1);
        phasebias_signals->set_bit(2);
        phasebias_signals->set_bit(3);
        phasebias_signals->set_bit(4);
        phasebias_signals->set_bit(5);
        phasebias_signals->set_bit(6);
        phasebias_signals->set_bit(7);

        element3->gnss_SSR_GriddedCorrectionReq_r16 = ALLOC_ZERO(GNSS_SSR_GriddedCorrectionReq_r16);
        element3->gnss_SSR_STEC_CorrectionReq_r16   = ALLOC_ZERO(GNSS_SSR_STEC_CorrectionReq_r16);
        element3->gnss_SSR_URA_Req_r16              = ALLOC_ZERO(GNSS_SSR_URA_Req_r16);

        auto element                          = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
        element->gnss_ID.gnss_id              = GNSS_ID__gnss_id_galileo;
        element->gnss_AuxiliaryInformationReq = ALLOC_ZERO(GNSS_AuxiliaryInformationReq);
        element->ext2                         = element2;
        element->ext3                         = element3;
        asn_sequence_add(&gar->list, element);
    }

    auto padq  = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15_t);
    auto padq1 = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15::GNSS_PeriodicAssistDataReq_r15__ext1);
    padq->ext1 = padq1;

    padq->gnss_SSR_PeriodicClockCorrectionsReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->gnss_SSR_PeriodicClockCorrectionsReq_r15->deliveryAmount_r15   = 32;
    padq->gnss_SSR_PeriodicClockCorrectionsReq_r15->deliveryInterval_r15 = interval;

    padq->gnss_SSR_PeriodicOrbitCorrectionsReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->gnss_SSR_PeriodicOrbitCorrectionsReq_r15->deliveryAmount_r15   = 32;
    padq->gnss_SSR_PeriodicOrbitCorrectionsReq_r15->deliveryInterval_r15 = interval;

    padq->gnss_SSR_PeriodicCodeBiasReq_r15 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq->gnss_SSR_PeriodicCodeBiasReq_r15->deliveryAmount_r15   = 32;
    padq->gnss_SSR_PeriodicCodeBiasReq_r15->deliveryInterval_r15 = interval;

    padq1->gnss_SSR_PeriodicPhaseBiasReq_r16 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq1->gnss_SSR_PeriodicPhaseBiasReq_r16->deliveryAmount_r15   = 32;
    padq1->gnss_SSR_PeriodicPhaseBiasReq_r16->deliveryInterval_r15 = interval;

    padq1->gnss_SSR_PeriodicGriddedCorrectionReq_r16 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq1->gnss_SSR_PeriodicGriddedCorrectionReq_r16->deliveryAmount_r15   = 32;
    padq1->gnss_SSR_PeriodicGriddedCorrectionReq_r16->deliveryInterval_r15 = interval;

    padq1->gnss_SSR_PeriodicSTEC_CorrectionReq_r16 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq1->gnss_SSR_PeriodicSTEC_CorrectionReq_r16->deliveryAmount_r15   = 32;
    padq1->gnss_SSR_PeriodicSTEC_CorrectionReq_r16->deliveryInterval_r15 = interval;

    padq1->gnss_SSR_PeriodicURA_Req_r16 = ALLOC_ZERO(GNSS_PeriodicControlParam_r15);
    padq1->gnss_SSR_PeriodicURA_Req_r16->deliveryAmount_r15   = 32;
    padq1->gnss_SSR_PeriodicURA_Req_r16->deliveryInterval_r15 = interval;

    auto gad1 = ALLOC_ZERO(A_GNSS_RequestAssistanceData::A_GNSS_RequestAssistanceData__ext1);
    gad1->gnss_PeriodicAssistDataReq_r15 = padq;

    auto gad                       = ALLOC_ZERO(A_GNSS_RequestAssistanceData_t);
    gad->ext1                      = gad1;
    gad->gnss_CommonAssistDataReq  = car;
    gad->gnss_GenericAssistDataReq = gar;

    auto rad9                            = &cext->choice.c1.choice.requestAssistanceData_r9;
    rad9->commonIEsRequestAssistanceData = cad;
    rad9->a_gnss_RequestAssistanceData   = gad;

    return message;
}

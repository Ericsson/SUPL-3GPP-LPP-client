#include "internal_lpp.h"
#include "lpp.h"

static GNSS_ReferenceTimeReq_t* request_reference_time(long gnss_id) {
    auto element = ALLOC_ZERO(GNSS_ReferenceTimeReq_t);
    asn_sequence_empty(&element->gnss_TimeReqPrefList.list);

    {
        auto id     = ALLOC_ZERO(GNSS_ID_t);
        id->gnss_id = gnss_id;
        asn_sequence_add(&element->gnss_TimeReqPrefList.list, id);
    }

    return element;
}

static GNSS_IonosphericModelReq_t* request_ionospheric_model(long gnss_id) {
    if (gnss_id == GNSS_ID__gnss_id_gps || gnss_id == GNSS_ID__gnss_id_glonass) {
        auto element = ALLOC_ZERO(GNSS_IonosphericModelReq_t);
        // Request '00' Klobuchar model
        element->klobucharModelReq = BitStringBuilder{}.clear(0).clear(1).to_bit_string(2);
        return element;
    } else if (gnss_id == GNSS_ID__gnss_id_bds) {
        auto element = ALLOC_ZERO(GNSS_IonosphericModelReq_t);
        // Request '01' BDS Klobuchar model
        element->klobucharModelReq = BitStringBuilder{}.clear(0).set(1).to_bit_string(2);
        return element;
    } else if (gnss_id == GNSS_ID__gnss_id_galileo) {
        auto element             = ALLOC_ZERO(GNSS_IonosphericModelReq_t);
        element->neQuickModelReq = ALLOC_ZERO(NULL_t);
        return element;
    }

    return nullptr;
}

static GNSS_TimeModelListReq_t* request_time_models() {
    auto element = ALLOC_ZERO(GNSS_TimeModelListReq_t);
    asn_sequence_empty(&element->list);

    auto model            = ALLOC_ZERO(GNSS_TimeModelElementReq);
    model->gnss_TO_IDsReq = 1;
    model->deltaTreq      = false;
    asn_sequence_add(&element->list, model);

    return element;
}

static GNSS_NavigationModelReq_t* request_navigation_model() {
    auto element                                 = ALLOC_ZERO(GNSS_NavigationModelReq);
    element->present                             = GNSS_NavigationModelReq_PR_storedNavList;
    element->choice.storedNavList.gnss_WeekOrDay = 0;
    element->choice.storedNavList.gnss_Toe       = 0;
    element->choice.storedNavList.t_toeLimit     = 0;
    return element;
}

static GNSS_RTK_ReferenceStationInfoReq_r15_t* request_reference_station() {
    auto element                             = ALLOC_ZERO(GNSS_RTK_ReferenceStationInfoReq_r15_t);
    element->antennaHeightReq_r15            = true;
    element->physicalReferenceStationReq_r15 = true;
    element->antennaDescriptionReq_r15       = false;
    return element;
}

static GNSS_AuxiliaryInformationReq_t* request_auxiliary_information() {
    auto element = ALLOC_ZERO(GNSS_AuxiliaryInformationReq_t);
    return element;
}

static GNSS_RTK_ObservationsReq_r15_t* request_observations() {
    auto element                            = ALLOC_ZERO(GNSS_RTK_ObservationsReq_r15_t);
    element->gnss_RTK_Integer_ms_Req_r15    = true;
    element->gnss_RTK_PhaseRangeRateReq_r15 = true;
    element->gnss_RTK_CNR_Req_r15           = true;

    BitStringBuilder{}
        .integer(0, 8, 0xFF)
        .into_bit_string(8, &element->gnss_RTK_SignalsReq_r15.gnss_SignalIDs);

    element->gnss_RTK_SignalsReq_r15.ext1 = ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
    element->gnss_RTK_SignalsReq_r15.ext1->gnss_SignalIDs_Ext_r15 =
        BitStringBuilder{}.integer(0, 16, 0xFFFF).to_bit_string(16);
    return element;
}

static GNSS_SSR_OrbitCorrectionsReq_r15_t* request_orbits() {
    auto element = ALLOC_ZERO(GNSS_SSR_OrbitCorrectionsReq_r15_t);
    return element;
}

static GNSS_SSR_ClockCorrectionsReq_r15_t* request_clocks() {
    auto element = ALLOC_ZERO(GNSS_SSR_ClockCorrectionsReq_r15_t);
    return element;
}

static GNSS_SSR_CodeBiasReq_r15_t* request_codebias() {
    auto element = ALLOC_ZERO(GNSS_SSR_CodeBiasReq_r15_t);

    BitStringBuilder{}
        .integer(0, 8, 0xFF)
        .into_bit_string(8, &element->signal_and_tracking_mode_ID_Map_r15.gnss_SignalIDs);

    element->signal_and_tracking_mode_ID_Map_r15.ext1 =
        ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
    element->signal_and_tracking_mode_ID_Map_r15.ext1->gnss_SignalIDs_Ext_r15 =
        BitStringBuilder{}.integer(0, 16, 0xFFFF).to_bit_string(16);
    return element;
}

static GNSS_SSR_PhaseBiasReq_r16_t* request_phasebias() {
    auto element = ALLOC_ZERO(GNSS_SSR_PhaseBiasReq_r16_t);

    BitStringBuilder{}
        .integer(0, 8, 0xFF)
        .into_bit_string(8, &element->signal_and_tracking_mode_ID_Map_r16.gnss_SignalIDs);

    element->signal_and_tracking_mode_ID_Map_r16.ext1 =
        ALLOC_ZERO(GNSS_SignalIDs::GNSS_SignalIDs__ext1);
    element->signal_and_tracking_mode_ID_Map_r16.ext1->gnss_SignalIDs_Ext_r15 =
        BitStringBuilder{}.integer(0, 16, 0xFFFF).to_bit_string(16);
    return element;
}

static GNSS_SSR_GriddedCorrectionReq_r16_t* request_gridded() {
    auto element = ALLOC_ZERO(GNSS_SSR_GriddedCorrectionReq_r16_t);
    return element;
}

static GNSS_SSR_STEC_CorrectionReq_r16_t* request_stec() {
    auto element = ALLOC_ZERO(GNSS_SSR_STEC_CorrectionReq_r16_t);
    return element;
}

static GNSS_SSR_URA_Req_r16_t* request_ura() {
    auto element = ALLOC_ZERO(GNSS_SSR_URA_Req_r16_t);
    return element;
}

static GNSS_GenericAssistDataReqElement_t* request_generic_gnss(long gnss_id, bool osr, bool ssr,
                                                                bool agnss) {
    auto element             = ALLOC_ZERO(GNSS_GenericAssistDataReqElement_t);
    element->gnss_ID.gnss_id = gnss_id;
    element->ext2 =
        ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext2);
    element->ext3 =
        ALLOC_ZERO(GNSS_GenericAssistDataReqElement::GNSS_GenericAssistDataReqElement__ext3);

    if (osr) {
        element->gnss_AuxiliaryInformationReq       = request_auxiliary_information();
        element->ext2->gnss_RTK_ObservationsReq_r15 = request_observations();

        if (gnss_id == GNSS_ID__gnss_id_gps || gnss_id == GNSS_ID__gnss_id_glonass) {
            element->ext2->gnss_RTK_ResidualsReq_r15 = ALLOC_ZERO(GNSS_RTK_ResidualsReq_r15_t);
        }

        if (gnss_id == GNSS_ID__gnss_id_glonass) {
            element->ext2->glo_RTK_BiasInformationReq_r15 =
                ALLOC_ZERO(GLO_RTK_BiasInformationReq_r15_t);
        }
    }

    if (ssr) {
        element->ext2->gnss_SSR_ClockCorrectionsReq_r15  = request_clocks();
        element->ext2->gnss_SSR_OrbitCorrectionsReq_r15  = request_orbits();
        element->ext2->gnss_SSR_CodeBiasReq_r15          = request_codebias();
        element->ext3->gnss_SSR_GriddedCorrectionReq_r16 = request_gridded();
        element->ext3->gnss_SSR_PhaseBiasReq_r16         = request_phasebias();
        element->ext3->gnss_SSR_STEC_CorrectionReq_r16   = request_stec();
        element->ext3->gnss_SSR_URA_Req_r16              = request_ura();
    }

    if (agnss) {
        element->gnss_TimeModelsReq        = request_time_models();
        element->gnss_NavigationModelReq   = request_navigation_model();
        element->gnss_RealTimeIntegrityReq = ALLOC_ZERO(GNSS_RealTimeIntegrityReq_t);
        element->gnss_AlmanacReq           = ALLOC_ZERO(GNSS_AlmanacReq_t);
        element->gnss_UTCModelReq          = ALLOC_ZERO(GNSS_UTC_ModelReq_t);
    }

    return element;
}

static GNSS_PeriodicControlParam_r15_t* request_delivery(long amount, long interval) {
    auto param                  = ALLOC_ZERO(GNSS_PeriodicControlParam_r15_t);
    param->deliveryInterval_r15 = interval;
    param->deliveryAmount_r15   = amount;
    return param;
}

LPP_Message* lpp_request_assistance_data(LPP_Transaction* transaction, CellID cell,
                                         long periodic_id, long interval) {
    auto message = lpp_create(transaction, LPP_MessageBody__c1_PR_requestAssistanceData);
    auto body    = message->lpp_MessageBody;

    auto cext     = &body->choice.c1.choice.requestAssistanceData.criticalExtensions;
    cext->present = RequestAssistanceData__criticalExtensions_PR_c1;
    cext->choice.c1.present =
        RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9;

    auto pad = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15);
    pad->periodicSessionID_r15.periodicSessionInitiator_r15 =
        PeriodicSessionID_r15__periodicSessionInitiator_r15_targetDevice;
    pad->periodicSessionID_r15.periodicSessionNumber_r15 = periodic_id;

    auto cad2 = ALLOC_ZERO(CommonIEsRequestAssistanceData::CommonIEsRequestAssistanceData__ext2);
    cad2->periodicAssistanceDataReq_r15 = pad;

    if(cell.is_nr) {
        cad2->primaryCellID_r15 = ncgi_create(cell.mcc, cell.mnc, cell.cell);
    }

    auto cad           = ALLOC_ZERO(CommonIEsRequestAssistanceData);
    if(!cell.is_nr) {
        cad->primaryCellID = ecgi_create(cell.mcc, cell.mnc, cell.cell);
    }
    cad->ext2          = cad2;

    auto car1 = ALLOC_ZERO(GNSS_CommonAssistDataReq::GNSS_CommonAssistDataReq__ext1);
    car1->gnss_RTK_ReferenceStationInfoReq_r15 = request_reference_station();

    auto car  = ALLOC_ZERO(GNSS_CommonAssistDataReq_t);
    car->ext1 = car1;

    auto gar = ALLOC_ZERO(GNSS_GenericAssistDataReq_t);
    asn_sequence_empty(&gar->list);
    asn_sequence_add(&gar->list, request_generic_gnss(GNSS_ID__gnss_id_gps, true, false, false));
    asn_sequence_add(&gar->list,
                     request_generic_gnss(GNSS_ID__gnss_id_glonass, true, false, false));
    asn_sequence_add(&gar->list,
                     request_generic_gnss(GNSS_ID__gnss_id_galileo, true, false, false));
    asn_sequence_add(&gar->list, request_generic_gnss(GNSS_ID__gnss_id_bds, true, false, false));

    auto padq                                    = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15_t);
    padq->gnss_RTK_PeriodicObservationsReq_r15   = request_delivery(32, interval);
    padq->gnss_RTK_PeriodicResidualsReq_r15      = request_delivery(32, interval);
    padq->glo_RTK_PeriodicBiasInformationReq_r15 = request_delivery(32, interval);

    auto gad1 = ALLOC_ZERO(A_GNSS_RequestAssistanceData::A_GNSS_RequestAssistanceData__ext1);
    gad1->gnss_PeriodicAssistDataReq_r15 = padq;

    auto gad                       = ALLOC_ZERO(A_GNSS_RequestAssistanceData_t);
    gad->gnss_CommonAssistDataReq  = car;
    gad->gnss_GenericAssistDataReq = gar;
    gad->ext1                      = gad1;

    auto rad9                            = &cext->choice.c1.choice.requestAssistanceData_r9;
    rad9->commonIEsRequestAssistanceData = cad;
    rad9->a_gnss_RequestAssistanceData   = gad;
    return message;
}

LPP_Message* lpp_request_agnss(LPP_Transaction* transaction, CellID cell, long gnss_id) {
    auto message = lpp_create(transaction, LPP_MessageBody__c1_PR_requestAssistanceData);
    auto body    = message->lpp_MessageBody;

    auto cext     = &body->choice.c1.choice.requestAssistanceData.criticalExtensions;
    cext->present = RequestAssistanceData__criticalExtensions_PR_c1;
    cext->choice.c1.present =
        RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9;

    auto cad           = ALLOC_ZERO(CommonIEsRequestAssistanceData);
    cad->primaryCellID = ecgi_create(cell.mcc, cell.mnc, cell.cell);

    auto car                      = ALLOC_ZERO(GNSS_CommonAssistDataReq_t);
    car->gnss_ReferenceTimeReq    = request_reference_time(gnss_id);
    car->gnss_IonosphericModelReq = request_ionospheric_model(gnss_id);

    auto gar = ALLOC_ZERO(GNSS_GenericAssistDataReq_t);
    asn_sequence_empty(&gar->list);
    asn_sequence_add(&gar->list, request_generic_gnss(gnss_id, false, false, true));

    auto gad                       = ALLOC_ZERO(A_GNSS_RequestAssistanceData_t);
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

    auto pad = ALLOC_ZERO(PeriodicAssistanceDataControlParameters_r15);
    pad->periodicSessionID_r15.periodicSessionInitiator_r15 =
        PeriodicSessionID_r15__periodicSessionInitiator_r15_targetDevice;
    pad->periodicSessionID_r15.periodicSessionNumber_r15 = periodic_id;

    auto cad2 = ALLOC_ZERO(CommonIEsRequestAssistanceData::CommonIEsRequestAssistanceData__ext2);
    cad2->periodicAssistanceDataReq_r15 = pad;

    auto cad           = ALLOC_ZERO(CommonIEsRequestAssistanceData);
    cad->primaryCellID = ecgi_create(cell.mcc, cell.mnc, cell.cell);
    cad->ext2          = cad2;

    auto car2_ssr = ALLOC_ZERO(GNSS_SSR_CorrectionPointsReq_r16);
    auto car2     = ALLOC_ZERO(GNSS_CommonAssistDataReq::GNSS_CommonAssistDataReq__ext2);
    car2->gnss_SSR_CorrectionPointsReq_r16 = car2_ssr;

    auto car  = ALLOC_ZERO(GNSS_CommonAssistDataReq_t);
    car->ext2 = car2;

    // GENERIC
    auto gar = ALLOC_ZERO(GNSS_GenericAssistDataReq_t);
    asn_sequence_empty(&gar->list);
    asn_sequence_add(&gar->list, request_generic_gnss(GNSS_ID__gnss_id_gps, false, true, false));
    asn_sequence_add(&gar->list,
                     request_generic_gnss(GNSS_ID__gnss_id_glonass, false, true, false));
    asn_sequence_add(&gar->list,
                     request_generic_gnss(GNSS_ID__gnss_id_galileo, false, true, false));
    asn_sequence_add(&gar->list, request_generic_gnss(GNSS_ID__gnss_id_bds, false, true, false));

    auto padq                                      = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15_t);
    padq->gnss_SSR_PeriodicClockCorrectionsReq_r15 = request_delivery(32, interval);
    padq->gnss_SSR_PeriodicOrbitCorrectionsReq_r15 = request_delivery(32, interval);
    padq->gnss_SSR_PeriodicCodeBiasReq_r15         = request_delivery(32, interval);

    auto padq1 = ALLOC_ZERO(GNSS_PeriodicAssistDataReq_r15::GNSS_PeriodicAssistDataReq_r15__ext1);
    padq1->gnss_SSR_PeriodicPhaseBiasReq_r16         = request_delivery(32, interval);
    padq1->gnss_SSR_PeriodicGriddedCorrectionReq_r16 = request_delivery(32, interval);
    padq1->gnss_SSR_PeriodicSTEC_CorrectionReq_r16   = request_delivery(32, interval);
    padq1->gnss_SSR_PeriodicURA_Req_r16              = request_delivery(32, interval);
    padq->ext1                                       = padq1;

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

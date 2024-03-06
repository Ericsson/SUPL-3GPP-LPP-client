#pragma once

#include "asn_helper.h"
#include "asnlib.h"
#include "cell_id.h"
#include "lpp.h"

#define LPP_MSG_MAX_COUNT 10

bool lpp_harvest_transaction(LPP_Transaction* transaction, LPP_Message* lpp);

LPP_Message*  lpp_decode(OCTET_STRING* data);
OCTET_STRING* lpp_encode(LPP_Message* lpp);

LPP_Message* lpp_create(LPP_Transaction* transaction, LPP_MessageBody__c1_PR pr);
void         lpp_destroy(LPP_Message* lpp);

LPP_Message* lpp_provide_capabilities(LPP_Transaction* transaction, bool segmentation);
LPP_Message* lpp_request_assistance_data(LPP_Transaction* transaction, CellID cell,
                                         long periodic_id, long interval);
LPP_Message* lpp_request_agnss(LPP_Transaction* transaction, CellID cell, long gnss_id);
LPP_Message* lpp_request_assistance_data_ssr(LPP_Transaction* transaction, CellID cell,
                                             long periodic_id, long interval);
LPP_Message* lpp_abort(LPP_Transaction* transaction);

int lpp_is_provide_assistance_data(LPP_Message* lpp);
int lpp_is_request_capabilities(LPP_Message* lpp);
int lpp_is_request_location_information(LPP_Message* lpp);
int lpp_is_abort(LPP_Message* lpp);

long lpp_get_periodic_id(LPP_Message* lpp);

inline LocationInformationType_t lpp_get_request_location_information_type(LPP_Message* lpp) {
    if (!lpp) return -1;
    if (!lpp->lpp_MessageBody) return -1;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return -1;
    if (lpp->lpp_MessageBody->choice.c1.present !=
        LPP_MessageBody__c1_PR_requestLocationInformation)
        return false;

    auto rli = &lpp->lpp_MessageBody->choice.c1.choice.requestLocationInformation;
    if (rli->criticalExtensions.present != RequestLocationInformation__criticalExtensions_PR_c1)
        return -1;
    if (rli->criticalExtensions.choice.c1.present !=
        RequestLocationInformation__criticalExtensions__c1_PR_requestLocationInformation_r9)
        return -1;

    auto r9 = &rli->criticalExtensions.choice.c1.choice.requestLocationInformation_r9;
    if (!r9->commonIEsRequestLocationInformation) return -1;

    return r9->commonIEsRequestLocationInformation->locationInformationType;
}

inline int lpp_get_request_location_interval(LPP_Message* lpp) {
    if (!lpp) return -1;
    if (!lpp->lpp_MessageBody) return -1;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return -1;
    if (lpp->lpp_MessageBody->choice.c1.present !=
        LPP_MessageBody__c1_PR_requestLocationInformation)
        return -1;

    auto rli = &lpp->lpp_MessageBody->choice.c1.choice.requestLocationInformation;
    if (rli->criticalExtensions.present != RequestLocationInformation__criticalExtensions_PR_c1)
        return -1;
    if (rli->criticalExtensions.choice.c1.present !=
        RequestLocationInformation__criticalExtensions__c1_PR_requestLocationInformation_r9)
        return -1;

    auto r9 = &rli->criticalExtensions.choice.c1.choice.requestLocationInformation_r9;
    if (!r9->commonIEsRequestLocationInformation) return -1;
    if (!r9->commonIEsRequestLocationInformation->periodicalReporting) return -1;

    switch (r9->commonIEsRequestLocationInformation->periodicalReporting->reportingInterval) {
    case PeriodicalReportingCriteria__reportingInterval_noPeriodicalReporting: return -1;
    case PeriodicalReportingCriteria__reportingInterval_ri0_25: return 1000;
    case PeriodicalReportingCriteria__reportingInterval_ri0_5: return 2000;
    case PeriodicalReportingCriteria__reportingInterval_ri1: return 4000;
    case PeriodicalReportingCriteria__reportingInterval_ri2: return 8000;
    case PeriodicalReportingCriteria__reportingInterval_ri4: return 10000;
    case PeriodicalReportingCriteria__reportingInterval_ri8: return 16000;
    case PeriodicalReportingCriteria__reportingInterval_ri16: return 20000;
    case PeriodicalReportingCriteria__reportingInterval_ri32: return 32000;
    case PeriodicalReportingCriteria__reportingInterval_ri64: return 64000;
    default: return -1;
    }
}

LPP_Message* lpp_PLI_location_estimate(LPP_Transaction*                           transaction,
                                       location_information::LocationInformation* li,
                                       bool                                       has_information);
LPP_Message* lpp_PLI_location_measurements(LPP_Transaction*                       transaction,
                                           location_information::ECIDInformation* li,
                                           bool                                   has_information);

#include "asn_helper.h"
#include "lpp.h"

#include <math.h>
#include <time.h>

bool lpp_harvest_transaction(LPP_Transaction* transaction, LPP_Message* lpp) {
    if (!lpp) return false;
    if (!lpp->transactionID) return false;

    transaction->id        = lpp->transactionID->transactionNumber;
    transaction->end       = lpp->endTransaction;
    transaction->initiator = lpp->transactionID->initiator == Initiator_targetDevice ? 0 : 1;
    return true;
}

LPP_Message* lpp_decode(OCTET_STRING* data) {
    LPP_Message*   lpp = ALLOC_ZERO(LPP_Message);
    asn_dec_rval_t rval =
        uper_decode_complete(0, &asn_DEF_LPP_Message, (void**)&lpp, data->buf, data->size);
    if (rval.code != RC_OK) {
        return NULL;
    }

    return lpp;
}

OCTET_STRING* lpp_encode(LPP_Message* lpp) {
    char           buffer[1 << 16];
    asn_enc_rval_t ret =
        uper_encode_to_buffer(&asn_DEF_LPP_Message, NULL, lpp, buffer, sizeof(buffer));
    if (ret.encoded == -1) {
        return NULL;
    }

    int pdu_len = (ret.encoded + 7) >> 3;

    OCTET_STRING* octet_buffer = ALLOC_ZERO(OCTET_STRING);
    (void)OCTET_STRING_fromBuf(octet_buffer, buffer, pdu_len);
    return octet_buffer;
}

LPP_Message* lpp_create(LPP_Transaction* transaction, LPP_MessageBody__c1_PR pr) {
    LPP_Message* lpp    = ALLOC_ZERO(LPP_Message);
    lpp->endTransaction = 0;

    if (transaction) {
        LPP_TransactionID* tid = ALLOC_ZERO(LPP_TransactionID);
        tid->initiator =
            transaction->initiator == 0 ? Initiator_targetDevice : Initiator_locationServer;
        tid->transactionNumber = transaction->id;
        lpp->transactionID     = tid;
        lpp->endTransaction    = transaction->end;
    }

    LPP_MessageBody_t* body = ALLOC_ZERO(LPP_MessageBody_t);
    body->present           = LPP_MessageBody_PR_c1;
    body->choice.c1.present = pr;

    lpp->lpp_MessageBody = body;
    return lpp;
}

void lpp_destroy(LPP_Message* lpp) {
    ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp);
}

OCTET_STRING* lpp_abort(LPP_Transaction* transaction) {
    transaction->end                 = 1;
    LPP_Message*       lpp_abort_msg = lpp_create(transaction, LPP_MessageBody__c1_PR_abort);
    LPP_MessageBody_t* body          = lpp_abort_msg->lpp_MessageBody;
    body->choice.c1.choice.abort.criticalExtensions.present = Abort__criticalExtensions_PR_c1;
    body->choice.c1.choice.abort.criticalExtensions.choice.c1.present =
        Abort__criticalExtensions__c1_PR_abort_r9;

    Abort_r9_IEs_t* abort_message =
        &body->choice.c1.choice.abort.criticalExtensions.choice.c1.choice.abort_r9;
    CommonIEsAbort_t* common_abort_message = ALLOC_ZERO(CommonIEsAbort_t);
    common_abort_message->abortCause =
        CommonIEsAbort__abortCause_stopPeriodicAssistanceDataDelivery_v1510;
    abort_message->commonIEsAbort = common_abort_message;

    OCTET_STRING* result = lpp_encode(lpp_abort_msg);
    lpp_destroy(lpp_abort_msg);
    return result;
}

int lpp_is_provide_assistance_data(LPP_Message* lpp) {
    if (!lpp) return 0;
    if (!lpp->lpp_MessageBody) return 0;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return 0;
    if (lpp->lpp_MessageBody->choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData)
        return 0;
    return 1;
}

long lpp_get_periodic_id(LPP_Message* lpp) {
    if (!lpp) return 0;
    if (!lpp->lpp_MessageBody) return 0;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return 0;
    if (lpp->lpp_MessageBody->choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData)
        return 0;

    auto c1_pad = &lpp->lpp_MessageBody->choice.c1.choice.provideAssistanceData;
    if (c1_pad->criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1)
        return 0;
    if (c1_pad->criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return 0;

    auto pad = &c1_pad->criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    if (!pad->commonIEsProvideAssistanceData) return 0;
    if (!pad->commonIEsProvideAssistanceData->ext2) return 0;
    if (!pad->commonIEsProvideAssistanceData->ext2->periodicAssistanceData_r15) return 0;

    auto periodic_ad = pad->commonIEsProvideAssistanceData->ext2->periodicAssistanceData_r15;
    auto session     = &periodic_ad->periodicSessionID_r15;
    return session->periodicSessionNumber_r15;
}

int lpp_is_request_capabilities(LPP_Message* lpp) {
    if (!lpp) return 0;
    if (!lpp->lpp_MessageBody) return 0;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return 0;
    if (lpp->lpp_MessageBody->choice.c1.present != LPP_MessageBody__c1_PR_requestCapabilities)
        return 0;
    return 1;
}

int lpp_is_request_location_information(LPP_Message* lpp) {
    if (!lpp) return 0;
    if (!lpp->lpp_MessageBody) return 0;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return 0;
    if (lpp->lpp_MessageBody->choice.c1.present !=
        LPP_MessageBody__c1_PR_requestLocationInformation)
        return 0;
    return 1;
}

int lpp_is_abort(LPP_Message* lpp) {
    if (!lpp) return 0;
    if (!lpp->lpp_MessageBody) return 0;
    if (lpp->lpp_MessageBody->present != LPP_MessageBody_PR_c1) return 0;
    if (lpp->lpp_MessageBody->choice.c1.present != LPP_MessageBody__c1_PR_abort) return 0;
    return 1;
}

long gnss_system_time_tow(GNSS_SystemTime_t system_time) {
    switch (system_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: {
        long day      = system_time.gnss_DayNumber % 7;
        long day_time = system_time.gnss_TimeOfDay;
        long ms       = day * 24 * 3600 * 1000 + day_time * 1000;
        if (system_time.gnss_TimeOfDayFrac_msec) {
            ms += *system_time.gnss_TimeOfDayFrac_msec;
        }

        long tow = ms / 80;  // tow is "time since last week in 6 seconds",
                             // divide by 0.08 tow/sec * 1000 ms = 80

        return tow;
    } break;
    }

    return 0;
}

static long encodeLat(double lat) {
    auto value = (lat / 90.0) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return (long)value;
}

static long encodeLon(double lon) {
    auto value = (lon / 180) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return (long)value;
}

static long uncertainityCoding(double r) {
    auto value = (long)(log(r * 10 / 3 + 1) / 0.01980262729);  // log(1.02)
    if (value <= 0) value = 0;
    if (value >= 255) value = 255;
    return value;
}

static long HAAltitude(double a) {
    long res = (long)a * 128;
    if (res < -64000) return -64000;
    if (res > 1280000) return 1280000;
    return res;
}

static void
lpp_HAEPWAAUE_r15(HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_r15_t* HAEPWAAUE,
                  LocationInformation*                                                 pi) {
    HAEPWAAUE->degreesLatitude_r15      = encodeLat(pi->lat);
    HAEPWAAUE->degreesLongitude_r15     = encodeLon(pi->lon);
    HAEPWAAUE->altitude_r15             = HAAltitude(pi->altitude);
    HAEPWAAUE->uncertaintySemiMajor_r15 = uncertainityCoding(pi->hacc);
    HAEPWAAUE->uncertaintySemiMinor_r15 = uncertainityCoding(pi->vacc);
    HAEPWAAUE->orientationMajorAxis_r15 = 0;                         // (0..179)
    HAEPWAAUE->horizontalConfidence_r15 = 68;                        // (0..100) standard deviation
    HAEPWAAUE->uncertaintyAltitude_r15  = uncertainityCoding(0.68);  // (0..255)
    HAEPWAAUE->verticalConfidence_r15   = 68;                        // (0..100) standard deviation
}

static LocationCoordinates_t* lpp_LocationCoordinates(LocationInformation* li) {
    auto LC = ALLOC_ZERO(LocationCoordinates_t);
    LC->present =
        LocationCoordinates_PR_highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510;
    lpp_HAEPWAAUE_r15(
        &LC->choice.highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510, li);
    return LC;
}

static long encodeVelocity(double vel, long max) {
    long N;
    vel = abs(vel) * 3.6;
    if (vel < 0.5) {
        N = 0;
    } else if (vel >= max + 0.5) {
        N = max;
    } else {
        N = round(vel);
    }
    return N;
}

static void lpp_HWVVAU(HorizontalWithVerticalVelocityAndUncertainty_t* HWVVAU,
                       LocationInformation*                            pi) {
    auto dir = pi->tracking;
    while (dir < 0.0)
        dir += 360.0;
    while (dir > 360.0)
        dir -= 360.0;
    HWVVAU->bearing         = (long)dir;
    HWVVAU->horizontalSpeed = encodeVelocity(pi->velocity, 2047);
    HWVVAU->verticalDirection =
        HorizontalWithVerticalVelocityAndUncertainty__verticalDirection_upward;
    HWVVAU->verticalSpeed              = encodeVelocity(pi->velocity, 255);
    HWVVAU->horizontalUncertaintySpeed = 0;
    HWVVAU->verticalUncertaintySpeed   = 0;
}

static Velocity_t* lpp_Velocity(LocationInformation* li) {
    auto V     = ALLOC_ZERO(Velocity_t);
    V->present = Velocity_PR_horizontalWithVerticalVelocityAndUncertainty;
    lpp_HWVVAU(&V->choice.horizontalWithVerticalVelocityAndUncertainty, li);
    return V;
}

static CommonIEsProvideLocationInformation_t::CommonIEsProvideLocationInformation__ext2*
lpp_PLI_CIE_ext2(LocationInformation* li) {
    auto ext2 = ALLOC_ZERO(
        CommonIEsProvideLocationInformation_t::CommonIEsProvideLocationInformation__ext2);
    auto locationSource      = BitString::allocate(6);
    ext2->locationSource_r13 = locationSource;
    locationSource->set_bit(LocationSource_r13_ha_gnss_v1510);

    struct tm tm {};
    auto      current_time = li->time;
    auto      ptm          = gmtime_r(&current_time, &tm);

    ext2->locationTimestamp_r13 = asn_time2UT(NULL, ptm, 1);

    return ext2;
}

static CommonIEsProvideLocationInformation_t*
lpp_PLI_CommonIEsProvideLocationInformation(LocationInformation* li, bool has_information) {
    auto CIE_PLI = ALLOC_ZERO(CommonIEsProvideLocationInformation_t);

    if (has_information) {
        CIE_PLI->locationEstimate = lpp_LocationCoordinates(li);
        CIE_PLI->velocityEstimate = lpp_Velocity(li);
        CIE_PLI->ext2             = lpp_PLI_CIE_ext2(li);
    } else {
        auto LE                  = ALLOC_ZERO(LocationError_t);
        LE->locationfailurecause = LocationFailureCause_periodicLocationMeasurementsNotAvailable;
        CIE_PLI->locationError   = LE;
    }

    return CIE_PLI;
}

LPP_Message* lpp_PLI_location_estimate(LPP_Transaction* transaction, LocationInformation* li,
                                       bool has_information) {
    //
    // Init body
    //
    auto lpp  = lpp_create(transaction, LPP_MessageBody__c1_PR_provideLocationInformation);
    auto body = lpp->lpp_MessageBody;
    body->choice.c1.choice.provideLocationInformation.criticalExtensions.present =
        ProvideLocationInformation__criticalExtensions_PR_c1;
    body->choice.c1.choice.provideLocationInformation.criticalExtensions.choice.c1.present =
        ProvideLocationInformation__criticalExtensions__c1_PR_provideLocationInformation_r9;
    ProvideLocationInformation_r9_IEs_t* PLI =
        &body->choice.c1.choice.provideLocationInformation.criticalExtensions.choice.c1.choice
             .provideLocationInformation_r9;

    auto CIE_PLI = lpp_PLI_CommonIEsProvideLocationInformation(li, has_information);
    PLI->commonIEsProvideLocationInformation = CIE_PLI;

    return lpp;
}

ECID_ProvideLocationInformation_t*
lpp_PLI_get_ECID_ProvideLocationInformation(ECIDInformation* ecid, bool has_information) {
    auto ECID_PLI = ALLOC_ZERO(ECID_ProvideLocationInformation_t);

    if (!has_information || ecid->neighbor_count <= 1) {
        auto error     = ALLOC_ZERO(ECID_Error);
        error->present = ECID_Error_PR_targetDeviceErrorCauses;
        error->choice.targetDeviceErrorCauses.cause =
            ECID_TargetDeviceErrorCauses__cause_requestedMeasurementNotAvailable;
        ECID_PLI->ecid_Error = error;

        return ECID_PLI;
    }

    auto ECID_SMI    = ALLOC_ZERO(ECID_SignalMeasurementInformation_t);
    auto primary_MRE = ALLOC_ZERO(MeasuredResultsElement_t);

    ECID_PLI->ecid_SignalMeasurementInformation = ECID_SMI;
    ECID_SMI->primaryCellMeasuredResults        = primary_MRE;

    //
    // Set ECID values for all cells ( CURRENTLY PLACEHOLDER! ) (NOT OPTIONAL)
    //
    for (auto i = 0; i < ecid->neighbor_count; i++) {
        auto& neighbour             = ecid->neighbors[i];
        auto  sample_element        = ALLOC_ZERO(MeasuredResultsElement_t);
        sample_element->physCellId  = neighbour.id;      // NOT OPTIONAL
        sample_element->arfcnEUTRA  = neighbour.earfcn;  // NOT OPTIONAL
        sample_element->rsrp_Result = newLong(neighbour.rsrp);
        sample_element->rsrq_Result = newLong(neighbour.rsrq);
        asn_sequence_add(&ECID_SMI->measuredResultsList, sample_element);
    }

    //
    // Set ECID values for primary cell
    //
    primary_MRE->physCellId  = ecid->neighbors[0].id;      // NOT OPTIONAL
    primary_MRE->arfcnEUTRA  = ecid->neighbors[0].earfcn;  // NOT OPTIONAL
    primary_MRE->rsrp_Result = newLong(ecid->neighbors[0].rsrp);
    primary_MRE->rsrq_Result = newLong(ecid->neighbors[0].rsrq);

    // Set cellGlobalId
    CellGlobalIdEUTRA_AndUTRA_t* CGI = ALLOC_ZERO(CellGlobalIdEUTRA_AndUTRA_t);
    CGI->cellIdentity.present        = CellGlobalIdEUTRA_AndUTRA__cellIdentity_PR_eutra;

    auto eutra = BitString::allocate(28, &CGI->cellIdentity.choice.eutra);
    eutra->set_integer(0, 28, ecid->cell.cell);

    int mcc  = ecid->cell.mcc;
    int temp = (int)(mcc - mcc % 100) / 100;
    asn_sequence_add(&CGI->plmn_Identity.mcc, newLong(temp));
    temp = mcc - temp * 100;
    asn_sequence_add(&CGI->plmn_Identity.mcc, newLong((int)(temp - temp % 10) / 10));
    asn_sequence_add(&CGI->plmn_Identity.mcc, newLong(mcc % 10));
    asn_sequence_add(&CGI->plmn_Identity.mnc,
                     newLong((int)(ecid->cell.mnc - ecid->cell.mnc % 10) / 10));
    asn_sequence_add(&CGI->plmn_Identity.mnc, newLong(ecid->cell.mnc % 10));

    primary_MRE->cellGlobalId = CGI;
    return ECID_PLI;
}

LPP_Message* lpp_PLI_location_measurements(LPP_Transaction* transaction, ECIDInformation* ecid,
                                           bool has_information) {
    //
    // Init body
    //
    auto lpp  = lpp_create(transaction, LPP_MessageBody__c1_PR_provideLocationInformation);
    auto body = lpp->lpp_MessageBody;
    body->choice.c1.choice.provideLocationInformation.criticalExtensions.present =
        ProvideLocationInformation__criticalExtensions_PR_c1;
    body->choice.c1.choice.provideLocationInformation.criticalExtensions.choice.c1.present =
        ProvideLocationInformation__criticalExtensions__c1_PR_provideLocationInformation_r9;
    ProvideLocationInformation_r9_IEs_t* PLI =
        &body->choice.c1.choice.provideLocationInformation.criticalExtensions.choice.c1.choice
             .provideLocationInformation_r9;

    PLI->ecid_ProvideLocationInformation =
        lpp_PLI_get_ECID_ProvideLocationInformation(ecid, has_information);

    return lpp;
}
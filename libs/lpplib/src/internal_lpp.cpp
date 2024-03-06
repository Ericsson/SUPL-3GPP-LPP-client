#include "asn_helper.h"
#include "location_information.h"
#include "lpp.h"

#include <A-GNSS-ProvideLocationInformation.h>
#include <math.h>
#include <time.h>
#include <utility/time.h>

bool lpp_harvest_transaction(LPP_Transaction* transaction, LPP_Message* lpp) {
    if (!lpp) return false;
    if (!lpp->transactionID) return false;

    transaction->id        = lpp->transactionID->transactionNumber;
    transaction->end       = lpp->endTransaction;
    transaction->initiator = lpp->transactionID->initiator == Initiator_targetDevice ? 0 : 1;
    return true;
}

LPP_Message* lpp_decode(OCTET_STRING* data) {
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    LPP_Message*   lpp = ALLOC_ZERO(LPP_Message);
    asn_dec_rval_t rval =
        uper_decode_complete(&stack_ctx, &asn_DEF_LPP_Message, (void**)&lpp, data->buf, data->size);
    if (rval.code != RC_OK) {
        free(lpp);
        return NULL;
    }

    return lpp;
}

OCTET_STRING* lpp_encode(LPP_Message* lpp) {
    char           buffer[1 << 16];
    asn_enc_rval_t ret =
        uper_encode_to_buffer(&asn_DEF_LPP_Message, NULL, lpp, buffer, sizeof(buffer));
    if (ret.encoded == -1) {
        printf("ERROR: Encoding failed: %s\n",
               ret.failed_type ? ret.failed_type->name : "<unknown>");
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

static long encode_latitude(double lat) {
    auto value = (lat / 90.0) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return (long)value;
}

static long encode_longitude(double lon) {
    auto value = (lon / 180) * pow(2, 31);
    if (value <= -2147483648) value = -2147483648;
    if (value >= 2147483647) value = 2147483647;
    return (long)value;
}

static long encode_ha_uncertainity(double r) {
    auto C     = 0.3;
    auto x     = 0.02;
    auto k     = log((r / C) + 1) / log(1 + x);
    auto value = (long)k;
    if (value <= 0) value = 0;
    if (value >= 255) value = 255;
    return value;
}

static long encode_ha_altitude(double a) {
    long res = (long)(a * 128.0);
    if (res < -64000) return -64000;
    if (res > 1280000) return 1280000;
    return res;
}

static long encode_orientation(double orientation) {
    auto value = (long)orientation;
    if (value < 0) value = 0;
    if (value > 179) value = 179;
    return value;
}

static long encode_confidence(double confidence) {
    auto value = (long)(confidence * 100.0);
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    return value;
}

static LocationCoordinates_t* encode_haepue(const location_information::LocationShape& shape) {
    auto location_coordinates = ALLOC_ZERO(LocationCoordinates_t);
    location_coordinates->present =
        LocationCoordinates_PR_highAccuracyEllipsoidPointWithUncertaintyEllipse_v1510;

    auto& data =
        location_coordinates->choice.highAccuracyEllipsoidPointWithUncertaintyEllipse_v1510;
    data.degreesLatitude_r15  = encode_latitude(shape.data.haepue.latitude);
    data.degreesLongitude_r15 = encode_longitude(shape.data.haepue.longitude);
    data.uncertaintySemiMajor_r15 =
        encode_ha_uncertainity(shape.data.haepue.horizontal_accuracy.semi_major);
    data.uncertaintySemiMinor_r15 =
        encode_ha_uncertainity(shape.data.haepue.horizontal_accuracy.semi_minor);
    data.orientationMajorAxis_r15 =
        encode_orientation(shape.data.haepue.horizontal_accuracy.orientation);
    data.confidence_r15 = encode_confidence(shape.data.haepue.horizontal_accuracy.confidence);
    return location_coordinates;
}

static LocationCoordinates_t* encode_haepaue(const location_information::LocationShape& shape) {
    auto location_coordinates = ALLOC_ZERO(LocationCoordinates_t);
    location_coordinates->present =
        LocationCoordinates_PR_highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510;

    auto& data = location_coordinates->choice
                     .highAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid_v1510;
    data.degreesLatitude_r15  = encode_latitude(shape.data.haepaue.latitude);
    data.degreesLongitude_r15 = encode_longitude(shape.data.haepaue.longitude);
    data.altitude_r15         = encode_ha_altitude(shape.data.haepaue.altitude);
    data.uncertaintySemiMajor_r15 =
        encode_ha_uncertainity(shape.data.haepaue.horizontal_accuracy.semi_major);
    data.uncertaintySemiMinor_r15 =
        encode_ha_uncertainity(shape.data.haepaue.horizontal_accuracy.semi_minor);
    data.orientationMajorAxis_r15 =
        encode_orientation(shape.data.haepaue.horizontal_accuracy.orientation);
    data.horizontalConfidence_r15 =
        encode_confidence(shape.data.haepaue.horizontal_accuracy.confidence);
    data.uncertaintyAltitude_r15 =
        encode_ha_uncertainity(shape.data.haepaue.vertical_accuracy.uncertainty);
    data.verticalConfidence_r15 =
        encode_confidence(shape.data.haepaue.vertical_accuracy.confidence);
    return location_coordinates;
}

static LocationCoordinates_t*
lpp_LocationCoordinates(const location_information::LocationShape& shape) {
    using namespace location_information;

    switch (shape.kind) {
    case LocationShape::Kind::HighAccuracyEllipsoidPointWithUncertaintyEllipse:
        return encode_haepue(shape);
    case LocationShape::Kind::HighAccuracyEllipsoidPointWithAltitudeAndUncertaintyEllipsoid:
        return encode_haepaue(shape);
    default: return nullptr;
    }
}

static long encode_velocity(double vel, long max) {
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

static long encode_bearing(double bearing) {
    while (bearing < 0.0)
        bearing += 360.0;
    while (bearing >= 360.0)
        bearing -= 360.0;
    auto value = (long)bearing;
    if (value < 0) value = 0;
    if (value > 359) value = 359;
    return value;
}

static Velocity_t* lpp_HorizontalVelocity(const location_information::VelocityShape& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalVelocity;

    auto& data           = element->choice.horizontalVelocity;
    data.bearing         = encode_bearing(shape.data.hv.horizontal.bearing);
    data.horizontalSpeed = encode_velocity(shape.data.hv.horizontal.speed, 2047);
    return element;
}

static Velocity_t*
lpp_HorizontalVelocityWithUncertainty(const location_information::VelocityShape& shape) {
    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalVelocityWithUncertainty;

    auto& data            = element->choice.horizontalVelocityWithUncertainty;
    data.bearing          = encode_bearing(shape.data.hvu.horizontal.bearing);
    data.horizontalSpeed  = encode_velocity(shape.data.hvu.horizontal.speed, 2047);
    data.uncertaintySpeed = encode_velocity(shape.data.hvu.horizontal.uncertainty, 255);
    return element;
}

static Velocity_t*
lpp_HorizontalWithVerticalVelocity(const location_information::VelocityShape& shape) {
    using namespace location_information;

    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalWithVerticalVelocity;

    auto& data             = element->choice.horizontalWithVerticalVelocity;
    data.bearing           = encode_bearing(shape.data.hvv.horizontal.bearing);
    data.horizontalSpeed   = encode_velocity(shape.data.hvv.horizontal.speed, 2047);
    data.verticalSpeed     = encode_velocity(shape.data.hvv.vertical.speed, 255);
    data.verticalDirection = shape.data.hvv.vertical.direction == VerticalDirection::Up ?
                                 HorizontalWithVerticalVelocity__verticalDirection_upward :
                                 HorizontalWithVerticalVelocity__verticalDirection_downward;
    return element;
}

static Velocity_t*
lpp_HorizontalWithVerticalVelocityAndUncertainty(const location_information::VelocityShape& shape) {
    using namespace location_information;

    auto element     = ALLOC_ZERO(Velocity_t);
    element->present = Velocity_PR_horizontalWithVerticalVelocityAndUncertainty;

    auto& data                      = element->choice.horizontalWithVerticalVelocityAndUncertainty;
    data.bearing                    = encode_bearing(shape.data.hvvu.horizontal.bearing);
    data.horizontalSpeed            = encode_velocity(shape.data.hvvu.horizontal.speed, 2047);
    data.horizontalUncertaintySpeed = encode_velocity(shape.data.hvvu.horizontal.uncertainty, 255);
    data.verticalSpeed              = encode_velocity(shape.data.hvvu.vertical.speed, 255);
    data.verticalUncertaintySpeed   = encode_velocity(shape.data.hvvu.vertical.uncertainty, 255);
    data.verticalDirection =
        shape.data.hvvu.vertical.direction == VerticalDirection::Up ?
            HorizontalWithVerticalVelocityAndUncertainty__verticalDirection_upward :
            HorizontalWithVerticalVelocityAndUncertainty__verticalDirection_downward;
    return element;
}

static Velocity_t* lpp_Velocity(const location_information::VelocityShape& shape) {
    using namespace location_information;

    switch (shape.kind) {
    case VelocityShape::Kind::HorizontalVelocity: return lpp_HorizontalVelocity(shape);
    case VelocityShape::Kind::HorizontalVelocityWithUncertainty:
        return lpp_HorizontalVelocityWithUncertainty(shape);
    case VelocityShape::Kind::HorizontalWithVerticalVelocity:
        return lpp_HorizontalWithVerticalVelocity(shape);
    case VelocityShape::Kind::HorizontalWithVerticalVelocityAndUncertainty:
        return lpp_HorizontalWithVerticalVelocityAndUncertainty(shape);
    default: return nullptr;
    }
}

static CommonIEsProvideLocationInformation_t::CommonIEsProvideLocationInformation__ext2*
lpp_PLI_CIE_ext2(const location_information::LocationInformation& location) {
    auto ext2 = ALLOC_ZERO(
        CommonIEsProvideLocationInformation_t::CommonIEsProvideLocationInformation__ext2);
    ext2->locationSource_r13 =
        BitStringBuilder{}.set(LocationSource_r13_ha_gnss_v1510).to_bit_string(6);

    struct tm tm {};
    auto      seconds      = UTC_Time{location.time}.timestamp().seconds();
    auto      current_time = static_cast<time_t>(seconds);
    auto      ptm          = gmtime_r(&current_time, &tm);

    ext2->locationTimestamp_r13 = asn_time2UT(NULL, ptm, 1);

    return ext2;
}

static CommonIEsProvideLocationInformation_t* lpp_PLI_CommonIEsProvideLocationInformation(
    const location_information::LocationInformation& location, bool has_information) {
    auto CIE_PLI = ALLOC_ZERO(CommonIEsProvideLocationInformation_t);

    if (has_information) {
        if (location.location.has_value()) {
            CIE_PLI->locationEstimate = lpp_LocationCoordinates(location.location.const_value());
        }

        if (location.velocity.has_value()) {
            CIE_PLI->velocityEstimate = lpp_Velocity(location.velocity.const_value());
        }

        CIE_PLI->ext2 = lpp_PLI_CIE_ext2(location);
    } else {
        auto LE                  = ALLOC_ZERO(LocationError_t);
        LE->locationfailurecause = LocationFailureCause_periodicLocationMeasurementsNotAvailable;
        CIE_PLI->locationError   = LE;
    }

    return CIE_PLI;
}

static GNSS_LocationInformation*
lpp_LocationInformation(const location_information::LocationInformation& location) {
    auto  location_information = ALLOC_ZERO(GNSS_LocationInformation);
    auto& mrt                  = location_information->measurementReferenceTime;

    auto time = GPS_Time{location.time};
    auto tod  = time.time_of_day();
    // time of day in milliseconds
    auto msec = static_cast<long>(tod.full_seconds() * 1000);
    // time of day in 250 nanoseconds
    auto nfrac = tod.full_seconds() * 1000.0 - static_cast<double>(msec);
    nfrac *= 1000.0 * 4.0;

    mrt.gnss_TimeID.gnss_id = GNSS_ID__gnss_id_gps;
    // only take the first 3600 * 1000 milliseconds of the day
    mrt.gnss_TOD_msec = msec % (3600 * 1000);
    mrt.gnss_TOD_frac = newLong((long)nfrac);

    BitStringBuilder{}
        .set(GNSS_ID_Bitmap__gnss_ids_gps)
        .set(GNSS_ID_Bitmap__gnss_ids_glonass)
        .set(GNSS_ID_Bitmap__gnss_ids_galileo)
        .into_bit_string(6, &location_information->agnss_List.gnss_ids);

    return location_information;
}

static A_GNSS_ProvideLocationInformation*
lpp_PLI_A_GNSS_ProvideLocationInformation(const location_information::LocationInformation& location,
                                          bool has_information) {
    if (!has_information) return nullptr;

    auto provide_location_information = ALLOC_ZERO(A_GNSS_ProvideLocationInformation);
    provide_location_information->gnss_LocationInformation = lpp_LocationInformation(location);

    return provide_location_information;
}

LPP_Message* lpp_PLI_location_estimate(LPP_Transaction*                           transaction,
                                       location_information::LocationInformation* li,
                                       bool                                       has_information) {
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

    auto CIE_PLI = lpp_PLI_CommonIEsProvideLocationInformation(*li, has_information);
    PLI->commonIEsProvideLocationInformation = CIE_PLI;
    PLI->a_gnss_ProvideLocationInformation =
        lpp_PLI_A_GNSS_ProvideLocationInformation(*li, has_information);

    return lpp;
}

ECID_ProvideLocationInformation_t*
lpp_PLI_get_ECID_ProvideLocationInformation(location_information::ECIDInformation* ecid,
                                            bool has_information) {
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

    BitStringBuilder{}
        .integer(0, 28, ecid->cell.cell)
        .into_bit_string(28, &CGI->cellIdentity.choice.eutra);

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

LPP_Message* lpp_PLI_location_measurements(LPP_Transaction*                       transaction,
                                           location_information::ECIDInformation* ecid,
                                           bool                                   has_information) {
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
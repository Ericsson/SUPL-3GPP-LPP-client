#include "lpp/message.hpp"
#include "lpp/periodic_session.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <A-GNSS-ProvideAssistanceData.h>
#include <CommonIEsProvideAssistanceData.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <PeriodicAssistanceDataControlParameters-r15.h>
#include <PeriodicSessionID-r15.h>
#pragma GCC diagnostic pop

namespace lpp {
namespace custom {

template <>
void Deleter<LPP_Message>::operator()(LPP_Message* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_LPP_Message, ptr);
    }
}

}  // namespace custom

void print(Message const& message) {
    if (!message) return;
    xer_fprint(stdout, &asn_DEF_LPP_Message, message.get());
}

void print(A_GNSS_ProvideAssistanceData* message) {
    if (!message) return;
    xer_fprint(stdout, &asn_DEF_A_GNSS_ProvideAssistanceData, message);
}

void destroy(A_GNSS_ProvideAssistanceData* message) {
    if (message) {
        ASN_STRUCT_FREE(asn_DEF_A_GNSS_ProvideAssistanceData, message);
    }
}

bool is_request_capabilities(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_requestCapabilities) return false;

    auto& outer = body.choice.c1.choice.requestCapabilities.criticalExtensions;
    if (outer.present != RequestCapabilities__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        RequestCapabilities__criticalExtensions__c1_PR_requestCapabilities_r9)
        return false;

    return true;
}

bool is_request_location_information(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_requestLocationInformation) return false;

    auto& outer = body.choice.c1.choice.requestLocationInformation.criticalExtensions;
    if (outer.present != RequestLocationInformation__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        RequestLocationInformation__criticalExtensions__c1_PR_requestLocationInformation_r9)
        return false;

    return true;
}

bool is_request_assistance_data(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_requestAssistanceData) return false;

    auto& outer = body.choice.c1.choice.requestAssistanceData.criticalExtensions;
    if (outer.present != RequestAssistanceData__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        RequestAssistanceData__criticalExtensions__c1_PR_requestAssistanceData_r9)
        return false;

    return true;
}

bool is_provide_capabilities(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideCapabilities) return false;

    auto& outer = body.choice.c1.choice.provideCapabilities.criticalExtensions;
    if (outer.present != ProvideCapabilities__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        ProvideCapabilities__criticalExtensions__c1_PR_provideCapabilities_r9)
        return false;

    return true;
}

bool is_provide_location_information(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideLocationInformation) return false;

    auto& outer = body.choice.c1.choice.provideLocationInformation.criticalExtensions;
    if (outer.present != ProvideLocationInformation__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        ProvideLocationInformation__criticalExtensions__c1_PR_provideLocationInformation_r9)
        return false;

    return true;
}

bool is_provide_assistance_data(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return false;

    auto& outer = body.choice.c1.choice.provideAssistanceData.criticalExtensions;
    if (outer.present != ProvideAssistanceData__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return false;

    return true;
}

bool is_abort(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_abort) return false;

    auto& outer = body.choice.c1.choice.abort.criticalExtensions;
    if (outer.present != Abort__criticalExtensions_PR_c1) return false;
    if (outer.choice.c1.present != Abort__criticalExtensions__c1_PR_abort_r9) return false;

    return true;
}

bool is_error(Message const& message) {
    if (!message) return false;
    if (!message->lpp_MessageBody) return false;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_error) return false;

    auto& outer = body.choice.c1.choice.error;
    if (outer.present != Error_PR_error_r9) return false;

    return true;
}

ProvideAssistanceData_r9_IEs* get_provide_assistance_data(Message const& message) {
    if (!message) return nullptr;
    if (!message->lpp_MessageBody) return nullptr;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return nullptr;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return nullptr;

    auto& outer = body.choice.c1.choice.provideAssistanceData.criticalExtensions;
    if (outer.present != ProvideAssistanceData__criticalExtensions_PR_c1) return nullptr;
    if (outer.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return nullptr;

    return &outer.choice.c1.choice.provideAssistanceData_r9;
}

bool get_periodic_session(ProvideAssistanceData_r9_IEs const& inner,
                          PeriodicSessionHandle*              session) {
    if (!session) return false;
    if (!inner.commonIEsProvideAssistanceData) return false;
    if (!inner.commonIEsProvideAssistanceData->ext2) return false;

    auto& ext2 = *inner.commonIEsProvideAssistanceData->ext2;
    if (!ext2.periodicAssistanceData_r15) return false;

    auto& psid = ext2.periodicAssistanceData_r15->periodicSessionID_r15;
    *session   = PeriodicSessionHandle{
        psid.periodicSessionNumber_r15,
        psid.periodicSessionInitiator_r15 ==
                PeriodicSessionID_r15__periodicSessionInitiator_r15_locationServer ?
              Initiator::LocationServer :
              Initiator::TargetDevice};
    return true;
}

RequestLocationInformation_r9_IEs* get_request_location_information(Message const& message) {
    if (!message) return nullptr;
    if (!message->lpp_MessageBody) return nullptr;

    auto& body = *message->lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return nullptr;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_requestLocationInformation) return nullptr;

    auto& outer = body.choice.c1.choice.requestLocationInformation.criticalExtensions;
    if (outer.present != RequestLocationInformation__criticalExtensions_PR_c1) return nullptr;
    if (outer.choice.c1.present !=
        RequestLocationInformation__criticalExtensions__c1_PR_requestLocationInformation_r9)
        return nullptr;

    return &outer.choice.c1.choice.requestLocationInformation_r9;
}

}  // namespace lpp

#include "lpp/abort.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <Abort-r9-IEs.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>

#define ALLOC_ZERO(type) reinterpret_cast<type*>(calloc(1, sizeof(type)))

namespace lpp {

static void abort_r9(Abort_r9_IEs&) {}

Message create_abort() {
    auto body               = ALLOC_ZERO(LPP_MessageBody);
    body->present           = LPP_MessageBody_PR_c1;
    body->choice.c1.present = LPP_MessageBody__c1_PR_abort;

    auto body_ce               = &body->choice.c1.choice.abort.criticalExtensions;
    body_ce->present           = Abort__criticalExtensions_PR_c1;
    body_ce->choice.c1.present = Abort__criticalExtensions__c1_PR_abort_r9;

    auto body_ce_c1 = &body_ce->choice.c1.choice.abort_r9;
    abort_r9(*body_ce_c1);

    auto message             = ALLOC_ZERO(LPP_Message);
    message->lpp_MessageBody = body;
    return Message{message};
}

}  // namespace lpp

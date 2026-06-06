#include "lpp_static_repeat.hpp"

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
EXTERNAL_WARNINGS_POP

#include <loglet/loglet.hpp>
#include <lpp/message.hpp>
#include <lpp/session.hpp>

LOGLET_MODULE2(p, lpp_static_repeat);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, lpp_static_repeat)

void LppStaticDataRepeat::inspect(streamline::System&, DataType const& message, uint64_t) NOEXCEPT {
    auto* pad = lpp::get_provide_assistance_data(message);
    if (!pad || !pad->a_gnss_ProvideAssistanceData) return;

    auto* common = pad->a_gnss_ProvideAssistanceData->gnss_CommonAssistData;
    if (!common) return;

    bool has_rtk = common->ext1 && common->ext1->gnss_RTK_ReferenceStationInfo_r15;
    bool has_cps = common->ext2 && common->ext2->gnss_SSR_CorrectionPoints_r16;
    if (!has_rtk && !has_cps) return;

    // Shallow copy with only the two static fields
    GNSS_CommonAssistData       tmp{};
    GNSS_CommonAssistData__ext1 ext1{};
    GNSS_CommonAssistData__ext2 ext2{};
    if (has_rtk) {
        ext1.gnss_RTK_ReferenceStationInfo_r15 = common->ext1->gnss_RTK_ReferenceStationInfo_r15;
        tmp.ext1                               = &ext1;
    }
    if (has_cps) {
        ext2.gnss_SSR_CorrectionPoints_r16 = common->ext2->gnss_SSR_CorrectionPoints_r16;
        tmp.ext2                           = &ext2;
    }

    // Encode into mCachedUper, reusing existing capacity
    mCachedUper.resize(4096);
    asn_enc_rval_t rv = uper_encode_to_buffer(&asn_DEF_GNSS_CommonAssistData, nullptr, &tmp,
                                              mCachedUper.data(), mCachedUper.size());
    if (rv.encoded < 0) {
        WARNF("failed to encode static GNSS_CommonAssistData");
        mCachedUper.clear();
        return;
    }

    size_t bytes = (static_cast<size_t>(rv.encoded) + 7) / 8;
    mCachedUper.resize(bytes);
    INFOF("cached static LPP data (%zu bytes, rtk=%d cps=%d)", bytes, has_rtk, has_cps);
}

bool LppStaticDataRepeat::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    mTimer = std::make_unique<scheduler::PeriodicTask>(mInterval);
    mTimer->set_event_name("lpp-static-repeat");
    mTimer->callback = [this]() {
        if (mCachedUper.empty()) return;

        GNSS_CommonAssistData* common{};
        asn_codec_ctx_t        ctx{};
        ctx.max_stack_size = 1024 * 1024;
        auto rv            = uper_decode_complete(&ctx, &asn_DEF_GNSS_CommonAssistData,
                                                  reinterpret_cast<void**>(&common), mCachedUper.data(),
                                                  mCachedUper.size());
        if (rv.code != RC_OK || !common) {
            WARNF("failed to decode cached GNSS_CommonAssistData");
            return;
        }

        auto* lpp_msg = static_cast<LPP_Message*>(calloc(1, sizeof(LPP_Message)));
        auto* body    = static_cast<LPP_MessageBody*>(calloc(1, sizeof(LPP_MessageBody)));
        auto* a_gnss  = static_cast<A_GNSS_ProvideAssistanceData*>(
            calloc(1, sizeof(A_GNSS_ProvideAssistanceData)));

        a_gnss->gnss_CommonAssistData = common;
        body->present                 = LPP_MessageBody_PR_c1;
        auto& c1                      = body->choice.c1;
        c1.present                    = LPP_MessageBody__c1_PR_provideAssistanceData;
        auto& outer                   = c1.choice.provideAssistanceData.criticalExtensions;
        outer.present                 = ProvideAssistanceData__criticalExtensions_PR_c1;
        auto& c1b                     = outer.choice.c1;
        c1b.present = ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9;
        c1b.choice.provideAssistanceData_r9.a_gnss_ProvideAssistanceData = a_gnss;
        lpp_msg->lpp_MessageBody                                         = body;

        DEBUGF("re-pushing cached static LPP data (%zu bytes)", mCachedUper.size());
        mSystem.push(lpp::Message{lpp_msg}, mTag);
    };
    return mTimer->schedule(scheduler);
}

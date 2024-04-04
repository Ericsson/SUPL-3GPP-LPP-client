#pragma once
#include "options.hpp"

namespace ssr_example {

enum class Format {
    XER,
    ASN1_UPER,
#ifdef INCLUDE_GENERATOR_SPARTN
    SPARTN_NEW,
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
    SPARTN_OLD,
#endif
#ifdef INCLUDE_GENERATOR_RTCM
    LRF_UPER,
#endif
};

class SsrCommand final : public Command {
public:
    SsrCommand()
        : Command("ssr", "Request State-space Representation (SSR) data from the location server"),
          mFormatArg(nullptr), mUraOverrideArg(nullptr), mUbloxClockCorrectionArg(nullptr),
          mForceContinuityArg(nullptr), mAverageZenithDelayArg(nullptr), mEnableIodeShift(nullptr),
          mSf055Override(nullptr), mIncreasingSiou(nullptr), mPrintRTCMArg(nullptr) {}

    ~SsrCommand() override {
        delete mFormatArg;
        delete mUraOverrideArg;
        delete mUbloxClockCorrectionArg;
        delete mForceContinuityArg;
        delete mAverageZenithDelayArg;
        delete mEnableIodeShift;
        delete mSf055Override;
        delete mIncreasingSiou;
        delete mPrintRTCMArg;
    }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
    args::ValueFlag<int>*         mUraOverrideArg;
    args::Flag*                   mUbloxClockCorrectionArg;
    args::Flag*                   mForceContinuityArg;
    args::Flag*                   mAverageZenithDelayArg;
    args::Flag*                   mEnableIodeShift;
    args::ValueFlag<int>*         mSf055Override;
    args::Flag*                   mIncreasingSiou;
    args::Flag*                   mPrintRTCMArg;
};

}  // namespace ssr_example

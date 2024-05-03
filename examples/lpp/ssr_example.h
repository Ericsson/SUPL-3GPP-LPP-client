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
          mSf055Override(nullptr), mSf055Default(nullptr), mSf042Override(nullptr),
          mSf042Default(nullptr), mIncreasingSiou(nullptr), mFilterByOcb(nullptr),
          mIgnoreL2L(nullptr), mPrintRTCMArg(nullptr) {}

    ~SsrCommand() override {
        delete mFormatArg;
        delete mUraOverrideArg;
        delete mUbloxClockCorrectionArg;
        delete mForceContinuityArg;
        delete mAverageZenithDelayArg;
        delete mEnableIodeShift;
        delete mSf055Override;
        delete mSf055Default;
        delete mSf042Override;
        delete mSf042Default;
        delete mIncreasingSiou;
        delete mFilterByOcb;
        delete mIgnoreL2L;
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
    args::ValueFlag<int>*         mSf055Default;
    args::ValueFlag<int>*         mSf042Override;
    args::ValueFlag<int>*         mSf042Default;
    args::Flag*                   mIncreasingSiou;
    args::Flag*                   mFilterByOcb;
    args::Flag*                   mIgnoreL2L;
    args::Flag*                   mPrintRTCMArg;
};

}  // namespace ssr_example

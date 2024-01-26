#pragma once
#include "options.hpp"

namespace ssr_example {

enum class Format {
    XER,
    SPARTN_NEW,
    SPARTN_OLD,
    ASN1_UPER,
};

class SsrCommand final : public Command {
public:
    SsrCommand()
        : Command("ssr", "Request State-space Representation (SSR) data from the location server"),
          mFormatArg(nullptr), mUraOverrideArg(nullptr), mUbloxClockCorrectionArg(nullptr),
          mForceContinuityArg(nullptr), mAverageZenithDelayArg(nullptr), mEnableIodeShift(nullptr) {
    }

    ~SsrCommand() override {
        delete mFormatArg;
        delete mUraOverrideArg;
        delete mUbloxClockCorrectionArg;
        delete mForceContinuityArg;
        delete mAverageZenithDelayArg;
        delete mEnableIodeShift;
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
};

}  // namespace ssr_example

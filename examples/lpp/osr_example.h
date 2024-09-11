#pragma once
#include "options.hpp"

namespace osr_example {

enum class Format {
#ifdef INCLUDE_GENERATOR_RTCM
    RTCM,
    LRF_UPER,
#endif
    XER,
    ASN1_UPER,
    NONE,
};

enum class MsmType {
    ANY,
    MSM4,
    MSM5,
    MSM6,
    MSM7,
};

class OsrCommand final : public Command {
public:
    OsrCommand()
        : Command("osr", "Request observation data from a location server"), mFormatArg(nullptr),
          mLRFMessageIdArg(nullptr), mMsmTypeArg(nullptr), mPrintRTCMArg(nullptr), mNoGPS(nullptr),
          mNoGLONASS(nullptr), mNoGalileo(nullptr), mNoBeiDou(nullptr) {}

    ~OsrCommand() override {
        delete mFormatArg;
        delete mLRFMessageIdArg;
        delete mMsmTypeArg;
        delete mPrintRTCMArg;
        delete mNoGPS;
        delete mNoGLONASS;
        delete mNoGalileo;
        delete mNoBeiDou;
    }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
    args::ValueFlag<int>*         mLRFMessageIdArg;
    args::ValueFlag<std::string>* mMsmTypeArg;
    args::Flag*                   mPrintRTCMArg;
    args::Flag*                   mNoGPS;
    args::Flag*                   mNoGLONASS;
    args::Flag*                   mNoGalileo;
    args::Flag*                   mNoBeiDou;
};

}  // namespace osr_example

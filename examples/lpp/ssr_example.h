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
          mFormatArg(nullptr), mLRFMessageIdArg(nullptr), mUraOverrideArg(nullptr),
          mUraDefaultArg(nullptr), mUbloxClockCorrectionArg(nullptr),
          mNoUbloxClockCorrectionArg(nullptr), mForceContinuityArg(nullptr),
          mNoForceContinuityArg(nullptr), mAverageZenithDelayArg(nullptr),
          mNoAverageZenithDelayArg(nullptr), mSf055Override(nullptr), mSf055Default(nullptr),
          mSf042Override(nullptr), mSf042Default(nullptr), mIncreasingSiou(nullptr),
          mFilterByResiduals(nullptr), mFilterByOcb(nullptr), mIgnoreL2L(nullptr),
          mPrintRTCMArg(nullptr), mCodeBiasNoTranslateArg(nullptr),
          mCodeBiasNoCorrectionShiftArg(nullptr), mPhaseBiasNoTranslateArg(nullptr),
          mPhaseBiasNoCorrectionShiftArg(nullptr), mHydrostaticInZenithArg(nullptr),
          mStecMethod(nullptr), mNoStecTransform(nullptr), mStecInvalidToZero(nullptr),
          mSignFlipC00(nullptr), mSignFlipC01(nullptr), mSignFlipC10(nullptr),
          mSignFlipC11(nullptr), mSignFlipStecResiduals(nullptr), mNoGPS(nullptr),
          mNoGLONASS(nullptr), mNoGalileo(nullptr), mBeiDou(nullptr), mFlipGridBitmask(nullptr),
          mNoGenerateGAD(nullptr), mNoGenerateOCB(nullptr), mNoGenerateHPAC(nullptr),
          mFlipOrbitCorrection(nullptr) {}

    ~SsrCommand() override { cleanup(); }

    void cleanup() {
        delete mFormatArg;
        delete mLRFMessageIdArg;
        delete mUraOverrideArg;
        delete mUraDefaultArg;
        delete mUbloxClockCorrectionArg;
        delete mNoUbloxClockCorrectionArg;
        delete mForceContinuityArg;
        delete mNoForceContinuityArg;
        delete mAverageZenithDelayArg;
        delete mNoAverageZenithDelayArg;
        delete mSf055Override;
        delete mSf055Default;
        delete mSf042Override;
        delete mSf042Default;
        delete mIncreasingSiou;
        delete mFilterByResiduals;
        delete mFilterByOcb;
        delete mIgnoreL2L;
        delete mPrintRTCMArg;
        delete mCodeBiasNoTranslateArg;
        delete mCodeBiasNoCorrectionShiftArg;
        delete mPhaseBiasNoTranslateArg;
        delete mPhaseBiasNoCorrectionShiftArg;
        delete mHydrostaticInZenithArg;
        delete mStecMethod;
        delete mNoStecTransform;
        delete mStecInvalidToZero;
        delete mSignFlipC00;
        delete mSignFlipC01;
        delete mSignFlipC10;
        delete mSignFlipC11;
        delete mSignFlipStecResiduals;
        delete mNoGPS;
        delete mNoGLONASS;
        delete mNoGalileo;
        delete mBeiDou;
        delete mFlipGridBitmask;
        delete mNoGenerateGAD;
        delete mNoGenerateOCB;
        delete mNoGenerateHPAC;
        delete mFlipOrbitCorrection;
    }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
    args::ValueFlag<int>*         mLRFMessageIdArg;
    args::ValueFlag<int>*         mUraOverrideArg;
    args::ValueFlag<int>*         mUraDefaultArg;
    args::Flag*                   mUbloxClockCorrectionArg;
    args::Flag*                   mNoUbloxClockCorrectionArg;
    args::Flag*                   mForceContinuityArg;
    args::Flag*                   mNoForceContinuityArg;
    args::Flag*                   mAverageZenithDelayArg;
    args::Flag*                   mNoAverageZenithDelayArg;
    args::ValueFlag<int>*         mSf055Override;
    args::ValueFlag<int>*         mSf055Default;
    args::ValueFlag<int>*         mSf042Override;
    args::ValueFlag<int>*         mSf042Default;
    args::Flag*                   mIncreasingSiou;
    args::Flag*                   mFilterByResiduals;
    args::Flag*                   mFilterByOcb;
    args::Flag*                   mIgnoreL2L;
    args::Flag*                   mPrintRTCMArg;
    args::Flag*                   mCodeBiasNoTranslateArg;
    args::Flag*                   mCodeBiasNoCorrectionShiftArg;
    args::Flag*                   mPhaseBiasNoTranslateArg;
    args::Flag*                   mPhaseBiasNoCorrectionShiftArg;
    args::Flag*                   mHydrostaticInZenithArg;
    args::ValueFlag<std::string>* mStecMethod;
    args::Flag*                   mNoStecTransform;
    args::Flag*                   mStecInvalidToZero;
    args::Flag*                   mSignFlipC00;
    args::Flag*                   mSignFlipC01;
    args::Flag*                   mSignFlipC10;
    args::Flag*                   mSignFlipC11;
    args::Flag*                   mSignFlipStecResiduals;
    args::Flag*                   mNoGPS;
    args::Flag*                   mNoGLONASS;
    args::Flag*                   mNoGalileo;
    args::Flag*                   mBeiDou;
    args::Flag*                   mFlipGridBitmask;
    args::Flag*                   mNoGenerateGAD;
    args::Flag*                   mNoGenerateOCB;
    args::Flag*                   mNoGenerateHPAC;
    args::Flag*                   mFlipOrbitCorrection;
};

}  // namespace ssr_example

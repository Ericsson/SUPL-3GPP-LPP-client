#pragma once
#include <lpp/cell_id.h>
#include "options.hpp"

#ifdef INCLUDE_GENERATOR_SPARTN
#include <generator/spartn2/generator.hpp>
#endif

namespace ssr_example {

enum class Format {
    XER,
    ASN1_UPER,
#ifdef INCLUDE_GENERATOR_SPARTN
    SPARTN_NEW,
#endif
#ifdef INCLUDE_GENERATOR_RTCM
    LRF_UPER,
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    TOKORO,
#endif
};

struct SsrGlobals {
    Options options;
    CellID  cell;
    Format  format;
    int     lrf_rtcm_id;

#ifdef INCLUDE_GENERATOR_SPARTN
    int                           ura_override;
    int                           ura_default;
    bool                          ublox_clock_correction;
    bool                          force_continuity;
    bool                          average_zenith_delay;
    bool                          iode_shift;
    int                           sf055_override;
    int                           sf055_default;
    int                           sf042_override;
    int                           sf042_default;
    bool                          increasing_siou;
    bool                          filter_by_residuals;
    bool                          filter_by_ocb;
    bool                          ignore_l2l;
    bool                          print_rtcm;
    bool                          hydrostatic_in_zenith;
    generator::spartn::StecMethod stec_method;
    bool                          stec_transform;
    bool                          stec_invalid_to_zero;
    bool                          sign_flip_c00;
    bool                          sign_flip_c01;
    bool                          sign_flip_c10;
    bool                          sign_flip_c11;
    bool                          sign_flip_stec_residuals;
    bool                          code_bias_translate;
    bool                          code_bias_correction_shift;
    bool                          phase_bias_translate;
    bool                          phase_bias_correction_shift;
    bool                          generate_gad;
    bool                          generate_ocb;
    bool                          generate_hpac;
    bool                          flip_grid_bitmask;
    bool                          flip_orbit_correction;
#endif

#ifdef INCLUDE_GENERATOR_TOKORO
    bool shapiro_correction;
    bool phase_windup_correction;
    bool earth_solid_tides_correction;
    bool antenna_phase_variation_correction;
    bool tropospheric_height_correction;
    bool iod_consistency_check;
    bool rtoc;
    bool ocit;
    bool negative_phase_windup;
#endif

    bool generate_gps;
    bool generate_glonass;
    bool generate_galileo;
    bool generate_beidou;
};

class SsrCommand final : public Command {
public:
    SsrCommand()
        : Command("ssr", "Request State-space Representation (SSR) data from the location server") {
        mFormatArg = nullptr;
        mNoGPS     = nullptr;
        mNoGLONASS = nullptr;
        mNoGalileo = nullptr;
        mNoBeiDou  = nullptr;
#ifdef INCLUDE_GENERATOR_RTCM
        mLRFMessageIdArg = nullptr;
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
        mUraOverrideArg                = nullptr;
        mUraDefaultArg                 = nullptr;
        mUbloxClockCorrectionArg       = nullptr;
        mNoUbloxClockCorrectionArg     = nullptr;
        mForceContinuityArg            = nullptr;
        mNoForceContinuityArg          = nullptr;
        mAverageZenithDelayArg         = nullptr;
        mNoAverageZenithDelayArg       = nullptr;
        mSf055Override                 = nullptr;
        mSf055Default                  = nullptr;
        mSf042Override                 = nullptr;
        mSf042Default                  = nullptr;
        mIncreasingSiou                = nullptr;
        mFilterByResiduals             = nullptr;
        mFilterByOcb                   = nullptr;
        mIgnoreL2L                     = nullptr;
        mPrintRTCMArg                  = nullptr;
        mCodeBiasNoTranslateArg        = nullptr;
        mCodeBiasNoCorrectionShiftArg  = nullptr;
        mPhaseBiasNoTranslateArg       = nullptr;
        mPhaseBiasNoCorrectionShiftArg = nullptr;
        mHydrostaticInZenithArg        = nullptr;
        mStecMethod                    = nullptr;
        mNoStecTransform               = nullptr;
        mStecInvalidToZero             = nullptr;
        mSignFlipC00                   = nullptr;
        mSignFlipC01                   = nullptr;
        mSignFlipC10                   = nullptr;
        mSignFlipC11                   = nullptr;
        mSignFlipStecResiduals         = nullptr;
        mFlipGridBitmask               = nullptr;
        mNoGenerateGAD                 = nullptr;
        mNoGenerateOCB                 = nullptr;
        mNoGenerateHPAC                = nullptr;
        mFlipOrbitCorrection           = nullptr;
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        mShapiroCorrection               = nullptr;
        mPhaseWindupCorrection           = nullptr;
        mEarthSolidTidesCorrection       = nullptr;
        mAntennaPhaseVariationCorrection = nullptr;
        mTroposphericHeightCorrection    = nullptr;
        mIodConsistencyCheck             = nullptr;
        mRtOC                            = nullptr;
        mOcit                            = nullptr;
        mNegativePhaseWindup             = nullptr;
#endif
    }

    ~SsrCommand() override { cleanup(); }

    void cleanup() {
        delete mFormatArg;
        delete mNoGPS;
        delete mNoGLONASS;
        delete mNoGalileo;
        delete mNoBeiDou;
#ifdef INCLUDE_GENERATOR_RTCM
        delete mLRFMessageIdArg;
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
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
        delete mFlipGridBitmask;
        delete mNoGenerateGAD;
        delete mNoGenerateOCB;
        delete mNoGenerateHPAC;
        delete mFlipOrbitCorrection;
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
        delete mShapiroCorrection;
        delete mPhaseWindupCorrection;
        delete mEarthSolidTidesCorrection;
        delete mAntennaPhaseVariationCorrection;
        delete mTroposphericHeightCorrection;
        delete mIodConsistencyCheck;
        delete mRtOC;
        delete mOcit;
        delete mNegativePhaseWindup;
#endif
    }

    void parse(args::Subparser& parser) override;
    void execute(Options options) override;

private:
    args::ValueFlag<std::string>* mFormatArg;
    args::Flag*                   mNoGPS;
    args::Flag*                   mNoGLONASS;
    args::Flag*                   mNoGalileo;
    args::Flag*                   mNoBeiDou;
#ifdef INCLUDE_GENERATOR_RTCM
    args::ValueFlag<int>* mLRFMessageIdArg;
#endif
#ifdef INCLUDE_GENERATOR_SPARTN
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
    args::Flag*                   mFlipGridBitmask;
    args::Flag*                   mNoGenerateGAD;
    args::Flag*                   mNoGenerateOCB;
    args::Flag*                   mNoGenerateHPAC;
    args::Flag*                   mFlipOrbitCorrection;
#endif
#ifdef INCLUDE_GENERATOR_TOKORO
    args::Flag* mShapiroCorrection;
    args::Flag* mPhaseWindupCorrection;
    args::Flag* mEarthSolidTidesCorrection;
    args::Flag* mAntennaPhaseVariationCorrection;
    args::Flag* mTroposphericHeightCorrection;
    args::Flag* mIodConsistencyCheck;
    args::Flag* mRtOC;
    args::Flag* mOcit;
    args::Flag* mNegativePhaseWindup;
#endif
};

}  // namespace ssr_example

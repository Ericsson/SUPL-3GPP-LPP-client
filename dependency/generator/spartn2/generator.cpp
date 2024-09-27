#include "generator.hpp"
#include "data.hpp"
#include "decode.hpp"
#include "message.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-SSR-CorrectionPoints-r16.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#pragma GCC diagnostic pop

#include <map>
#include <unordered_map>

namespace generator {
namespace spartn {

Generator::Generator()
    : mGenerationIndex(0), mNextAreaId(1), mUraOverride(-1),
      mUraDefault(0 /* SF024(0) = unknown */), mContinuityIndicator(-1),
      mUBloxClockCorrection(false), mSf055Override(-1), mSf055Default(0 /* SF055(0) = invalid */),
      mSf042Override(-1), mSf042Default(0 /* SF042(0) = invalid */),
      mComputeAverageZenithDelay(false), mGroupByEpochTime(false), mIodeShift(true),
      mIncreasingSiou(false), mSiouIndex(1), mCodeBiasTranslate(true),
      mCodeBiasCorrectionShift(true), mPhaseBiasTranslate(true), mPhaseBiasCorrectionShift(true),
      mHydrostaticResidualInZenith(false), mStecMethod(StecMethod::Default), mStecTranform(true),
      mFlipGridBitmask(false), mFilterByResiduals(false), mFilterByOcb(false), mIgnoreL2L(false),
      mStecInvalidToZero(false), mSignFlipC00(false), mSignFlipC01(false), mSignFlipC10(false),
      mSignFlipC11(false), mSignFlipStecResiduals(false), mFlipOrbitCorrection(false),
      mGenerateGad(true), mGenerateOcb(true), mGenerateHpac(true), mGpsSupported(true),
      mGlonassSupported(true), mGalileoSupported(true), mBeidouSupported(false) {}

Generator::~Generator() = default;

std::vector<Message> Generator::generate(LPP_Message const* lpp_message) {
    // Clear previous messages
    mMessages.clear();
    if (!lpp_message) return mMessages;

    auto body = lpp_message->lpp_MessageBody;
    if (body->present != LPP_MessageBody_PR_c1) return mMessages;
    if (body->choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return mMessages;

    auto& pad = body->choice.c1.choice.provideAssistanceData;
    if (pad.criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1)
        return mMessages;
    if (pad.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return mMessages;

    // Initialze (and clear previous) correction data
    mCorrectionData = std::unique_ptr<CorrectionData>(new CorrectionData(mGroupByEpochTime));

    auto message = &pad.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    find_correction_point_set(message);
    find_ocb_corrections(message);
    find_hpac_corrections(message);

    auto iods = mCorrectionData->iods();
    for (auto iod : iods) {
#ifdef SPARTN_DEBUG_PRINT
        printf("-- iod=%hu\n", iod);
#endif

        auto ocb  = mCorrectionData->ocb(iod);
        auto hpac = mCorrectionData->hpac(iod);

        if (mIncreasingSiou) {
            // Only update SIOU when a new GAD/HPAC message is generated
            if (hpac && mGenerateHpac) {
                mSiouIndex++;
                mSiouIndex %= 512;
            }
        }

        if (hpac && mGenerateGad) {
            std::vector<uint16_t> set_ids;
            hpac->set_ids(set_ids);

            SpartnTime gad_epoch_time{};
            if (mCorrectionData->find_gad_epoch_time(iod, &gad_epoch_time)) {
                for (auto set_id : set_ids) {
#ifdef SPARTN_DEBUG_PRINT
                    printf("GAD: set=%hu, iod=%hu, time=%u\n", set_id, iod,
                           gad_epoch_time.rounded_seconds);
#endif

                    generate_gad(iod, gad_epoch_time.rounded_seconds, set_id);
                }
            } else {
#ifdef SPARTN_DEBUG_PRINT
                printf("GAD: no epoch time for iod=%hu\n", iod);
#endif
            }
        }

        if (hpac && mGenerateHpac) {
            generate_hpac(iod);
        }

        if (ocb && mGenerateOcb) {
            generate_ocb(iod);
        }
    }

    // Increment generation index
    mGenerationIndex++;
    return mMessages;
}

void Generator::find_correction_point_set(ProvideAssistanceData_r9_IEs const* message) {
    if (!message->a_gnss_ProvideAssistanceData) return;
    if (!message->a_gnss_ProvideAssistanceData->gnss_CommonAssistData) return;

    auto& cad = *message->a_gnss_ProvideAssistanceData->gnss_CommonAssistData;
    if (!cad.ext2) return;
    if (!cad.ext2->gnss_SSR_CorrectionPoints_r16) return;

    auto& ssr = *cad.ext2->gnss_SSR_CorrectionPoints_r16;
    if (ssr.correctionPoints_r16.present ==
        GNSS_SSR_CorrectionPoints_r16__correctionPoints_r16_PR_arrayOfCorrectionPoints_r16) {
        auto& array                   = ssr.correctionPoints_r16.choice.arrayOfCorrectionPoints_r16;
        auto  correction_point_set_id = static_cast<uint16_t>(ssr.correctionPointSetID_r16);
        if (mCorrectionPointSets.find(correction_point_set_id) != mCorrectionPointSets.end()) {
            // NOTE(ewasjon): We assume that the correction point set cannot be changed. From an
            // location server point-of-view, this should never happen. The correction point set is
            // only every sent on the first non-periodic message or when the UE changes cell.
            return;
        } else {
            CorrectionPointSet correction_point_set{};
            correction_point_set.set_id  = correction_point_set_id;
            correction_point_set.area_id = next_area_id();
            correction_point_set.grid_point_count =
                (array.numberOfStepsLatitude_r16 + 1) * (array.numberOfStepsLongitude_r16 + 1);
            correction_point_set.referencePointLatitude_r16  = array.referencePointLatitude_r16;
            correction_point_set.referencePointLongitude_r16 = array.referencePointLongitude_r16;
            correction_point_set.numberOfStepsLatitude_r16   = array.numberOfStepsLatitude_r16;
            correction_point_set.numberOfStepsLongitude_r16  = array.numberOfStepsLongitude_r16;
            correction_point_set.stepOfLatitude_r16          = array.stepOfLatitude_r16;
            correction_point_set.stepOfLongitude_r16         = array.stepOfLongitude_r16;

            correction_point_set.reference_point_latitude =
                decode::referencePointLatitude_r16(correction_point_set.referencePointLatitude_r16);
            correction_point_set.reference_point_longitude = decode::referencePointLongitude_r16(
                correction_point_set.referencePointLongitude_r16);
            correction_point_set.latitude_delta =
                decode::stepOfLatitude_r16(correction_point_set.stepOfLatitude_r16);
            correction_point_set.longitude_delta =
                decode::stepOfLongitude_r16(correction_point_set.stepOfLongitude_r16);

            uint64_t bitmask = 0;
            if (array.bitmaskOfGrids_r16) {
                for (size_t i = 0; i < array.bitmaskOfGrids_r16->size; i++) {
                    bitmask <<= 8;
                    bitmask |= static_cast<uint64_t>(array.bitmaskOfGrids_r16->buf[i]);
                }
                bitmask >>= array.bitmaskOfGrids_r16->bits_unused;
#ifdef SPARTN_DEBUG_PRINT
                printf(" bitmask: %ld bytes, %d bits, 0x%016lX\n", array.bitmaskOfGrids_r16->size,
                       array.bitmaskOfGrids_r16->bits_unused, bitmask);
#endif
            } else {
                bitmask = 0xFFFFFFFFFFFFFFFF;
            }

            if (mFlipGridBitmask) {
                uint64_t new_bitmask = 0;
                for (int i = 0; i < 64; i++) {
                    new_bitmask <<= 1;
                    new_bitmask |= bitmask & 1;
                    bitmask >>= 1;
                }
                bitmask = new_bitmask;
            }

            correction_point_set.bitmask = bitmask;
            correction_point_set.calculate_grid_points();

            auto correction_point_set_ptr = std::unique_ptr<CorrectionPointSet>(
                new CorrectionPointSet(std::move(correction_point_set)));
            mCorrectionPointSets.insert(
                std::make_pair(ssr.correctionPointSetID_r16, std::move(correction_point_set_ptr)));
        }
    } else {
        // TODO(ewasjon): [low-priority] Support list of correction points
    }
}

void Generator::find_ocb_corrections(ProvideAssistanceData_r9_IEs const* message) {
    if (!message->a_gnss_ProvideAssistanceData) return;
    if (!message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    auto& gad = *message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (int i = 0; i < gad.list.count; i++) {
        auto element = gad.list.array[i];
        if (!element) continue;

        auto gnss_id = element->gnss_ID.gnss_id;
        if (element->ext2) {
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_OrbitCorrections_r15);
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_ClockCorrections_r15);
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_CodeBias_r15);
        }

        if (element->ext3) {
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_PhaseBias_r16);
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_URA_r16);
        }
    }
}

void Generator::find_hpac_corrections(ProvideAssistanceData_r9_IEs const* message) {
    if (!message->a_gnss_ProvideAssistanceData) return;
    if (!message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    auto& gad = *message->a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (int i = 0; i < gad.list.count; i++) {
        auto element = gad.list.array[i];
        if (!element) continue;

        auto gnss_id = element->gnss_ID.gnss_id;
        if (element->ext3) {
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_STEC_Correction_r16);
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_GriddedCorrection_r16);
        }
    }
}

}  // namespace spartn
}  // namespace generator

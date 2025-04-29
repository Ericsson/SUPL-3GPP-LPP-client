#include "possib_logger.hpp"

#include <loglet/loglet.hpp>
#include <time/gps.hpp>

#include <filesystem>

#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <ProvideAssistanceData.h>

#include <GNSS-EarthOrientationParameters.h>
#include <GNSS-Integrity-ServiceAlert-r17.h>
#include <GNSS-Integrity-ServiceParameters-r17.h>
#include <GNSS-IonosphericModel.h>
#include <GNSS-RTK-AuxiliaryStationData-r15.h>
#include <GNSS-RTK-CommonObservationInfo-r15.h>
#include <GNSS-RTK-ReferenceStationInfo-r15.h>
#include <GNSS-ReferenceLocation.h>
#include <GNSS-ReferenceTime.h>
#include <GNSS-SSR-CorrectionPoints-r16.h>

LOGLET_MODULE2(p, possib);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, possib)

LppPossibLogger::LppPossibLogger(std::string path) : mPath(path) {
    VSCOPE_FUNCTION();
    mEncodeBufferSize = 1024 * 1024 * 16;
    mEncodeBuffer     = malloc(mEncodeBufferSize);
}

LppPossibLogger::~LppPossibLogger() {
    VSCOPE_FUNCTION();
    free(mEncodeBuffer);
}

std::ostream& LppPossibLogger::get_stream(std::string const& path) {
    auto now       = ts::GPS_Time::now();
    auto timestamp = (now.timestamp().seconds() / 3600) * 3600;

    auto it = mFiles.find(path);
    if (it == mFiles.end()) {
        if (it->second.last_timestamp != timestamp) {
            it->second.stream.flush();
            it->second.stream.close();
            // Continue and open the new file
        } else {
            return it->second.stream;
        }
    }

    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }

    auto& file  = mFiles[path];
    file.stream = std::ofstream{std::filesystem::path{path} / (std::to_string(timestamp) + ".json"),
                                std::ios::app};
    file.last_timestamp = timestamp;
    return file.stream;
}

void LppPossibLogger::basic_log(char const* type, asn_TYPE_descriptor_s* def, void const* ptr) {
    memset(mEncodeBuffer, 0, mEncodeBufferSize);
    auto encode_result =
        uper_encode_to_buffer(descriptor, nullptr, element, mEncodeBuffer, mEncodeBufferSize);
    if (encode_result.encoded > 0) {
        auto  length = static_cast<size_t>((encode_result.encoded + 7) / 8);
        auto  path   = std::filesystem::path{mPath} / "common" / type;
        auto& stream = get_stream(path);

        stream << "{";
        stream << "\"length\":" << length << ",";
        stream << "\"data\":\"";
        for (size_t i = 0; i < length; i++) {
            stream << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(static_cast<unsigned char*>(mEncodeBuffer)[i]);
        }
        stream << "\",";
        stream << "\"type\":\"" << type << "\"";
        stream << "}" << std::endl;
    }
}

static char const* gnss_name_from_id(long id) {
    switch (id) {
    case GNSS_ID__gnss_id_gps: return "gps";
    case GNSS_ID__gnss_id_glonass: return "glo";
    case GNSS_ID__gnss_id_galileo: return "gal";
    case GNSS_ID__gnss_id_bds: return "bds";
    default: return "unk";
    }
}

void LppPossibLogger::basic_log(long id, char const* type, asn_TYPE_descriptor_s* def,
                                void const* ptr) {
    memset(mEncodeBuffer, 0, mEncodeBufferSize);
    auto encode_result =
        uper_encode_to_buffer(descriptor, nullptr, element, mEncodeBuffer, mEncodeBufferSize);
    if (encode_result.encoded > 0) {
        auto  length = static_cast<size_t>((encode_result.encoded + 7) / 8);
        auto  path   = std::filesystem::path{mPath} / gnss_name_from_id(id) / type;
        auto& stream = get_stream(path);

        stream << "{";
        stream << "\"length\":" << length << ",";
        stream << "\"data\":\"";
        for (size_t i = 0; i < length; i++) {
            stream << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(static_cast<unsigned char*>(mEncodeBuffer)[i]);
        }
        stream << "\",";
        stream << "\"type\":\"" << type << "\",";
        stream << "\"gnss\":\"" << gnss_name_from_id(id) << "\"";
        stream << "}" << std::endl;
    }
}

void LppPossibLogger::process(GNSS_ReferenceTime const& x) {
    basic_log("gnss-reference-time", &asn_DEF_GNSS_ReferenceTime, &x);
}

void LppPossibLogger::process(GNSS_ReferenceLocation const& x) {
    basic_log("gnss-reference-location", &asn_DEF_GNSS_ReferenceLocation, &x);
}

void LppPossibLogger::process(GNSS_IonosphericModel const& x) {
    basic_log("gnss-ionospheric-model", &asn_DEF_GNSS_IonosphericModel, &x);
}

void LppPossibLogger::process(GNSS_EarthOrientationParameters const& x) {
    basic_log("gnss-earth-orientation-parameters", &asn_DEF_GNSS_EarthOrientationParameters, &x);
}

void LppPossibLogger::process(GNSS_RTK_ReferenceStationInfo_r15 const& x) {
    basic_log("gnss-rtk-reference-station-info", &asn_DEF_GNSS_RTK_ReferenceStationInfo_r15, &x);
}

void LppPossibLogger::process(GNSS_RTK_CommonObservationInfo_r15 const& x) {
    basic_log("gnss-rtk-common-observation-info", &asn_DEF_GNSS_RTK_CommonObservationInfo_r15, &x);
}

void LppPossibLogger::process(GNSS_RTK_AuxiliaryStationData_r15 const& x) {
    basic_log("gnss-rtk-auxiliary-station-data", &asn_DEF_GNSS_RTK_AuxiliaryStationData_r15, &x);
}

void LppPossibLogger::process(GNSS_SSR_CorrectionPoints_r16 const& x) {
    basic_log("gnss-ssr-corretion-points", &asn_DEF_GNSS_SSR_CorrectionPoints_r16, &x);
}

void LppPossibLogger::process(GNSS_Integrity_ServiceParameters_r17 const& x) {
    basic_log("gnss-integrity-service-parameters", &asn_DEF_GNSS_Integrity_ServiceParameters_r17,
              &x);
}

void LppPossibLogger::process(GNSS_Integrity_ServiceAlert_r17 const& x) {
    basic_log("gnss-integrity-service-alert", &asn_DEF_GNSS_Integrity_ServiceAlert_r17, &x);
}

void LppPossibLogger::process(GNSS_LOS_NLOS_GridPoints_r18 const& x) {
    basic_log("gnss-los-nlos-grid-points", &asn_DEF_GNSS_LOS_NLOS_GridPoints_r18, &x);
}

void LppPossibLogger::process(GNSS_SSR_IOD_Update_r18 const& x) {
    basic_log("gnss-ssr-iod-update", &asn_DEF_GNSS_SSR_IOD_Update_r18, &x);
}

void LppPossibLogger::process(GNSS_CommonAssistData const& x) {
    if (x.gnss_ReferenceTime) process(*x.gnss_ReferenceTime);
    if (x.gnss_ReferenceLocation) process(*x.gnss_ReferenceLocation);
    if (x.gnss_IonosphericModel) process(*x.gnss_IonosphericModel);
    if (x.gnss_EarthOrientationParameters) process(*x.gnss_EarthOrientationParameters);

    if (x.ext1) {
        if (x.ext1->gnss_RTK_ReferenceStationInfo_r15)
            process(*x.ext1->gnss_RTK_ReferenceStationInfo_r15);
        if (x.ext1->gnss_RTK_CommonObservationInfo_r15)
            process(*x.ext1->gnss_RTK_CommonObservationInfo_r15);
        if (x.ext1->gnss_RTK_AuxiliaryStationData_r15)
            process(*x.ext1->gnss_RTK_AuxiliaryStationData_r15);
    }

    if (x.ext2) {
        if (x.ext2->gnss_SSR_CorrectionPoints_r16) process(*x.ext2->gnss_SSR_CorrectionPoints_r16);
    }

    if (x.ext3) {
        if (x.ext3->gnss_Integrity_ServiceParameters_r17)
            process(*x.ext3->gnss_Integrity_ServiceParameters_r17);
        if (x.ext3->gnss_Integrity_ServiceAlert_r17)
            process(*x.ext3->gnss_Integrity_ServiceAlert_r17);
    }

    if (x.ext4) {
        if (x.ext4->gnss_los_nlos_GridPoints_r18) process(*x.ext4->gnss_los_nlos_GridPoints_r18);
        if (x.ext4->gnss_SSR_IOD_Update_r18) process(*x.ext4->gnss_SSR_IOD_Update_r18);
    }
}

void LppPossibLogger::process(Long id, GNSS_TimeModelList const& x) {
    basic_log(id, "gnss-time-model-list", &asn_DEF_GNSS_TimeModelList, &x);
}

void LppPossibLogger::process(Long id, GNSS_DifferentialCorrections const& x) {
    basic_log(id, "gnss-differential-corrections", &asn_DEF_GNSS_DifferentialCorrections, &x);
}

void LppPossibLogger::process(Long id, GNSS_NavigationModel const& x) {
    basic_log(id, "gnss-navigation-model", &asn_DEF_GNSS_NavigationModel, &x);
}

void LppPossibLogger::process(Long id, GNSS_RealTimeIntegrity const& x) {
    basic_log(id, "gnss-real-time-integrity", &asn_DEF_GNSS_RealTimeIntegrity, &x);
}

void LppPossibLogger::process(Long id, GNSS_DataBitAssistance const& x) {
    basic_log(id, "gnss-data-bit-assistance", &asn_DEF_GNSS_DataBitAssistance, &x);
}

void LppPossibLogger::process(Long id, GNSS_AcquisitionAssistance const& x) {
    basic_log(id, "gnss-acquisition-assistance", &asn_DEF_GNSS_AcquisitionAssistance, &x);
}

void LppPossibLogger::process(Long id, GNSS_Almanac const& x) {
    basic_log(id, "gnss-almanac", &asn_DEF_GNSS_Almanac, &x);
}

void LppPossibLogger::process(Long id, GNSS_UTC_Model const& x) {
    basic_log(id, "gnss-utc-model", &asn_DEF_GNSS_UTC_Model, &x);
}

void LppPossibLogger::process(Long id, GNSS_AuxiliaryInformation const& x) {
    basic_log(id, "gnss-axuiliary-information", &asn_DEF_GNSS_AuxiliaryInformation, &x);
}

void LppPossibLogger::process(Long id, BDS_DifferentialCorrections_r12 const& x) {
    basic_log(id, "bds-differential-corrections", &asn_DEF_BDS_DifferentialCorrections_r12, &x);
}

void LppPossibLogger::process(Long id, BDS_GridModelParameter_r12 const& x) {
    basic_log(id, "bds-grid-model-parameter", &asn_DEF_BDS_GridModelParameter_r12, &x);
}

void LppPossibLogger::process(Long id, GNSS_RTK_Observations_r15 const& x) {
    basic_log(id, "gnss-rtk-observations", &asn_DEF_GNSS_RTK_Observations_r15, &x);
}

void LppPossibLogger::process(Long id, GLO_RTK_BiasInformation_r15 const& x) {
    basic_log(id, "glo-rtk-bias-information", &asn_DEF_GLO_RTK_BiasInformation_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_RTK_MAC_CorrectionDifferences_r15 const& x) {
    basic_log(id, "gnss-rtk-max-correction-differences",
              &asn_DEF_GNSS_RTK_MAC_CorrectionDifferences_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_RTK_Residuals_r15 const& x) {
    basic_log(id, "gnss-rtk-residuals", &asn_DEF_GNSS_RTK_Residuals_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_RTK_FKP_Gradients_r15 const& x) {
    basic_log(id, "gnss-rtk-fkp-gradients", &asn_DEF_GNSS_RTK_FKP_Gradients_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_OrbitCorrections_r15 const& x) {
    basic_log(id, "gnss-ssr-orbit-corrections", &asn_DEF_GNSS_SSR_OrbitCorrections_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_ClockCorrections_r15 const& x) {
    basic_log(id, "gnss-ssr-clock-corrections", &asn_DEF_GNSS_SSR_ClockCorrections_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_CodeBias_r15 const& x) {
    basic_log(id, "gnss-ssr-code-bias", &asn_DEF_GNSS_SSR_CodeBias_r15, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_URA_r16 const& x) {
    basic_log(id, "gnss-ssr-ura", &asn_DEF_GNSS_SSR_URA_r16, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_PhaseBias_r16 const& x) {
    basic_log(id, "gnss-ssr-phase-bias", &asn_DEF_GNSS_SSR_PhaseBias_r16, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_STEC_Correction_r16 const& x) {
    basic_log(id, "gnss-ssr-stec-corrections", &asn_DEF_GNSS_SSR_STEC_Correction_r16, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_GriddedCorrection_r16 const& x) {
    basic_log(id, "gnss-ssr-gridded-correction", &asn_DEF_GNSS_SSR_GriddedCorrection_r16, &x);
}

void LppPossibLogger::process(Long id, NavIC_DifferentialCorrections_r16 const& x) {
    basic_log(id, "navic-differential-corrections", &asn_DEF_NavIC_DifferentialCorrections_r16, &x);
}

void LppPossibLogger::process(Long id, NavIC_GridModelParameter_r16 const& x) {
    basic_log(id, "navic-grid-model-parameter", &asn_DEF_NavIC_GridModelParameter_r16, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_OrbitCorrectionsSet2_r17 const& x) {
    basic_log(id, "gnss-ssr-orbit-corrections-set2", &asn_DEF_GNSS_SSR_OrbitCorrectionsSet2_r17,
              &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_ClockCorrectionsSet2_r17 const& x) {
    basic_log(id, "gnss-ssr-clock-corrections-set2", &asn_DEF_GNSS_SSR_ClockCorrectionsSet2_r17,
              &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_URA_Set2_r17 const& x) {
    basic_log(id, "gnss-ssr-ura-set2", &asn_DEF_GNSS_SSR_URA_Set2_r17, &x);
}

void LppPossibLogger::process(Long id, GNSS_LOS_NLOS_GriddedIndications_r18 const& x) {
    basic_log(id, "gnss-los-nlos-gridded-indications",
              &asn_DEF_GNSS_LOS_NLOS_GriddedIndications_r18, &x);
}

void LppPossibLogger::process(Long id, GNSS_SSR_SatellitePCVResiduals_r18 const& x) {
    basic_log(id, "gnss-ssr-satellite-pcv-residuals", &asn_DEF_GNSS_SSR_SatellitePCVResiduals_r18,
              &x);
}

void LppPossibLogger::process(GNSS_GenericAssistDataElement const& x) {
    auto id = x.gnss_ID.gnss_id;

    if (x.gnss_TimeModels) process(id, *x.gnss_TimeModels);
    if (x.gnss_DifferentialCorrections) process(id, *x.gnss_DifferentialCorrections);
    if (x.gnss_NavigationModel) process(id, *x.gnss_NavigationModel);
    if (x.gnss_RealTimeIntegrity) process(id, *x.gnss_RealTimeIntegrity);
    if (x.gnss_DataBitAssistance) process(id, *x.gnss_DataBitAssistance);
    if (x.gnss_AcquisitionAssistance) process(id, *x.gnss_AcquisitionAssistance);
    if (x.gnss_Almanac) process(id, *x.gnss_Almanac);
    if (x.gnss_UTC_Model) process(id, *x.gnss_UTC_Model);
    if (x.gnss_AuxiliaryInformation) process(id, *x.gnss_AuxiliaryInformation);

    if (x.ext1) {
        if (x.ext1->bds_DifferentialCorrections_r12)
            process(id, *x.ext1->bds_DifferentialCorrections_r12);
        if (x.ext1->bds_GridModel_r12) process(id, *x.ext1->bds_GridModel_r12);
    }

    if (x.ext2) {
        if (x.ext2->gnss_RTK_Observations_r15) process(id, *x.ext2->gnss_RTK_Observations_r15);
        if (x.ext2->glo_RTK_BiasInformation_r15) process(id, *x.ext2->glo_RTK_BiasInformation_r15);
        if (x.ext2->gnss_RTK_MAC_CorrectionDifferences_r15)
            process(id, *x.ext2->gnss_RTK_MAC_CorrectionDifferences_r15);
        if (x.ext2->gnss_RTK_Residuals_r15) process(id, *x.ext2->gnss_RTK_Residuals_r15);
        if (x.ext2->gnss_RTK_FKP_Gradients_r15) process(id, *x.ext2->gnss_RTK_FKP_Gradients_r15);
        if (x.ext2->gnss_SSR_OrbitCorrections_r15)
            process(id, *x.ext2->gnss_SSR_OrbitCorrections_r15);
        if (x.ext2->gnss_SSR_ClockCorrections_r15)
            process(id, *x.ext2->gnss_SSR_ClockCorrections_r15);
        if (x.ext2->gnss_SSR_CodeBias_r15) process(id, *x.ext2->gnss_SSR_CodeBias_r15);
    }

    if (x.ext3) {
        if (x.ext3->gnss_SSR_URA_r16) process(id, *x.ext3->gnss_SSR_URA_r16);
        if (x.ext3->gnss_SSR_PhaseBias_r16) process(id, *x.ext3->gnss_SSR_PhaseBias_r16);
        if (x.ext3->gnss_SSR_STEC_Correction_r16)
            process(id, *x.ext3->gnss_SSR_STEC_Correction_r16);
        if (x.ext3->gnss_SSR_GriddedCorrection_r16)
            process(id, *x.ext3->gnss_SSR_GriddedCorrection_r16);
        if (x.ext3->navic_DifferentialCorrections_r16)
            process(id, *x.ext3->navic_DifferentialCorrections_r16);
        if (x.ext3->navic_GridModel_r16) process(id, *x.ext3->navic_GridModel_r16);
    }

    if (x.ext4) {
        if (x.ext4->gnss_SSR_OrbitCorrectionsSet2_r17)
            process(id, *x.ext4->gnss_SSR_OrbitCorrectionsSet2_r17);
        if (x.ext4->gnss_SSR_ClockCorrectionsSet2_r17)
            process(id, *x.ext4->gnss_SSR_ClockCorrectionsSet2_r17);
        if (x.ext4->gnss_SSR_URA_Set2_r17) process(id, *x.ext4->gnss_SSR_URA_Set2_r17);
    }

    if (x.ext5) {
        if (x.ext5->gnss_LOS_NLOS_GriddedIndications_r18)
            process(id, *x.ext5->gnss_LOS_NLOS_GriddedIndications_r18);
        if (x.ext5->gnss_SSR_SatellitePCVResiduals_r18)
            process(id, *x.ext5->gnss_SSR_SatellitePCVResiduals_r18);
    }
}

void LppPossibLogger::process(GNSS_GenericAssistData const& message) {
    for (int i = 0; i < message.list.count; i++) {
        auto element = message.list.array[i];
        if (element) process(*element);
    }
}

void LppPossibLogger::process(A_GNSS_ProvideAssistanceData const& message) {
    if (message.gnss_CommonAssistData) process(*message.gnss_CommonAssistData);
    if (message.gnss_GenericAssistData) process(*message.gnss_GenericAssistData);
}

void LppPossibLogger::process(ProvideAssistanceData_r9_IEs const& message) {
    if (message.commonIEsProvideAssistanceData) process(*message.commonIEsProvideAssistanceData);
    if (message.a_gnss_ProvideAssistanceData) process(*message.a_gnss_ProvideAssistanceData);
}

void LppPossibLogger::process(ProvideAssistanceData const& message) {
    switch (message.criticalExtensions.present) {
    case ProvideAssistanceData__criticalExtensions_PR::
        ProvideAssistanceData__criticalExtensions_PR_c1: {
        switch (message.criticalExtensions.choice.c1.present) {
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9: {
            return process(message.criticalExtensions.choice.c1.choice.provideAssistanceData_r9);
        }
        }
    }
    }
}

void LppPossibLogger::inspect(streamline::System&, DataType const& message) {
    VSCOPE_FUNCTION();

    if (!message) return;

    auto body = message->lpp_MessageBody;
    if (!body) return;

    switch (body->present) {
    case LPP_MessageBody_PR::LPP_MessageBody_PR_c1: {
        switch (body->choice.c1.present) {
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_provideAssistanceData: {
            return process(body->choice.c1.choice.provideAssistanceData);
        }
        }
    }
    }
}

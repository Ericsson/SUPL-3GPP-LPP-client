#include "possib_logger.hpp"

#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>

#include <filesystem>
#include <iomanip>
#include <sstream>

#include <A-GNSS-ProvideAssistanceData.h>
#include <AssistanceDataSIBelement-r15.h>
#include <BDS-DifferentialCorrections-r12.h>
#include <BDS-GridModelParameter-r12.h>
#include <GLO-RTK-BiasInformation-r15.h>
#include <GNSS-AcquisitionAssistance.h>
#include <GNSS-Almanac.h>
#include <GNSS-AuxiliaryInformation.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-DataBitAssistance.h>
#include <GNSS-DifferentialCorrections.h>
#include <GNSS-EarthOrientationParameters.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-Integrity-ServiceAlert-r17.h>
#include <GNSS-Integrity-ServiceParameters-r17.h>
#include <GNSS-IonosphericModel.h>
#include <GNSS-LOS-NLOS-GridPoints-r18.h>
#include <GNSS-LOS-NLOS-GriddedIndications-r18.h>
#include <GNSS-NavigationModel.h>
#include <GNSS-RTK-AuxiliaryStationData-r15.h>
#include <GNSS-RTK-CommonObservationInfo-r15.h>
#include <GNSS-RTK-FKP-Gradients-r15.h>
#include <GNSS-RTK-MAC-CorrectionDifferences-r15.h>
#include <GNSS-RTK-Observations-r15.h>
#include <GNSS-RTK-ReferenceStationInfo-r15.h>
#include <GNSS-RTK-Residuals-r15.h>
#include <GNSS-RealTimeIntegrity.h>
#include <GNSS-ReferenceLocation.h>
#include <GNSS-ReferenceTime.h>
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-ClockCorrectionsSet2-r17.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-CorrectionPoints-r16.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-IOD-Update-r18.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-OrbitCorrectionsSet2-r17.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GNSS-SSR-SatellitePCVResiduals-r18.h>
#include <GNSS-SSR-URA-Set2-r17.h>
#include <GNSS-SSR-URA-r16.h>
#include <GNSS-TimeModelList.h>
#include <GNSS-UTC-Model.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <NavIC-DifferentialCorrections-r16.h>
#include <NavIC-GridModelParameter-r16.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <ProvideAssistanceData.h>
#include <SSR-ClockCorrectionSatelliteElement-r15.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSatList-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-CodeBiasSignalList-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSatList-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#include <SSR-PhaseBiasSignalList-r16.h>
#include <SSR-URA-SatElement-r16.h>
#include <SSR-URA-SatList-r16.h>

LOGLET_MODULE2(p, possib);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, possib)

void PossibOutput::inspect(streamline::System&, DataType const& message) NOEXCEPT {
    VSCOPE_FUNCTION();
    auto sentence = message->json();
    auto data     = reinterpret_cast<uint8_t const*>(sentence.data());
    auto size     = sentence.size();
    for (auto const& output : mOutput.outputs) {
        if (!output.possib_support()) continue;
        if (output.print) {
            XINFOF(OUTPUT_PRINT_MODULE, "possib: %zd bytes", size);
        }

        output.interface->write(data, size);
    }
}

LppPossibBuilder::LppPossibBuilder(bool wrap) : mIncludeWrap(wrap) {
    VSCOPE_FUNCTION();
    mEncodeBufferSize = 1024 * 1024 * 16;
    mEncodeBuffer     = malloc(mEncodeBufferSize);
}

LppPossibBuilder::~LppPossibBuilder() {
    VSCOPE_FUNCTION();
    free(mEncodeBuffer);
}

std::vector<uint8_t> LppPossibBuilder::encode_to_buffer(asn_TYPE_descriptor_s* descriptor,
                                                        void const*            struct_ptr) {
    memset(mEncodeBuffer, 0, mEncodeBufferSize);
    auto encode_result =
        uper_encode_to_buffer(descriptor, nullptr, struct_ptr, mEncodeBuffer, mEncodeBufferSize);
    if (encode_result.encoded > 0) {
        auto length = static_cast<size_t>((encode_result.encoded + 7) / 8);
        auto begin  = reinterpret_cast<uint8_t*>(mEncodeBuffer);
        auto end    = begin + length;
        return std::vector<uint8_t>{begin, end};
    } else {
        return {};
    }
}

void LppPossibBuilder::log(char const* type, asn_TYPE_descriptor_s* def, void const* ptr,
                           std::unordered_map<std::string, std::string> const& params) {
    VSCOPE_FUNCTIONF("\"%s\"", type);
    auto assistance_data_buffer = encode_to_buffer(def, ptr);

    std::vector<uint8_t> assistance_sib_buffer{};
    if (mIncludeWrap) {
        auto assistance_sib_element =
            (AssistanceDataSIBelement_r15*)calloc(1, sizeof(AssistanceDataSIBelement_r15));
        assistance_sib_element->assistanceDataElement_r15.buf  = assistance_data_buffer.data();
        assistance_sib_element->assistanceDataElement_r15.size = assistance_data_buffer.size();

        assistance_sib_buffer =
            encode_to_buffer(&asn_DEF_AssistanceDataSIBelement_r15, assistance_sib_element);
        free(assistance_sib_element);
    }

    auto wrap = mIncludeWrap && assistance_sib_buffer.size() > 0;
    if (!assistance_data_buffer.empty()) {
        auto time  = ts::Gps::now();
        auto epoch = time.timestamp().seconds();

        std::stringstream stream;
        stream << "{";
        stream << "\"type\":\"" << type << "\"";
        stream << ",\"gps_epoch\":" << epoch;
        stream << ",\"time\":\"" << ts::Utc{time}.rfc3339() << "\"";
        stream << ",\"data_length\":" << assistance_data_buffer.size();
        if (wrap) {
            stream << ",\"sib_length\":" << assistance_sib_buffer.size();
        }
        stream << ",\"data\":\"";
        for (size_t i = 0; i < assistance_data_buffer.size(); i++) {
            stream << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(assistance_data_buffer[i]);
        }
        stream << "\"";
        if (wrap) {
            stream << ",\"sib\":\"";
            for (size_t i = 0; i < assistance_sib_buffer.size(); i++) {
                stream << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<int>(assistance_sib_buffer[i]);
            }
            stream << "\"";
        }
        if (!params.empty()) {
            for (auto const& [key, value] : params) {
                stream << ",\"" << key << "\":\"" << value << "\"";
            }
        }
        stream << "}" << std::endl;

        auto str     = stream.str();
        auto message = std::unique_ptr<PossibMessage>(new PossibMessage(str));
        if (mSystem) {
            mSystem->push(std::move(message));
        }
    }
}

void LppPossibBuilder::basic_log(char const* type, asn_TYPE_descriptor_s* def, void const* ptr) {
    log(type, def, ptr, {});
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

void LppPossibBuilder::basic_log(long id, char const* type, asn_TYPE_descriptor_s* def,
                                 void const* ptr) {
    std::unordered_map<std::string, std::string> params;
    params["gnss_id"] = gnss_name_from_id(id);
    log(type, def, ptr, params);
}

void LppPossibBuilder::process(GNSS_ReferenceTime const& x) {
    basic_log("gnss-reference-time", &asn_DEF_GNSS_ReferenceTime, &x);
}

void LppPossibBuilder::process(GNSS_ReferenceLocation const& x) {
    basic_log("gnss-reference-location", &asn_DEF_GNSS_ReferenceLocation, &x);
}

void LppPossibBuilder::process(GNSS_IonosphericModel const& x) {
    basic_log("gnss-ionospheric-model", &asn_DEF_GNSS_IonosphericModel, &x);
}

void LppPossibBuilder::process(GNSS_EarthOrientationParameters const& x) {
    basic_log("gnss-earth-orientation-parameters", &asn_DEF_GNSS_EarthOrientationParameters, &x);
}

void LppPossibBuilder::process(GNSS_RTK_ReferenceStationInfo_r15 const& x) {
    basic_log("gnss-rtk-reference-station-info", &asn_DEF_GNSS_RTK_ReferenceStationInfo_r15, &x);
}

void LppPossibBuilder::process(GNSS_RTK_CommonObservationInfo_r15 const& x) {
    basic_log("gnss-rtk-common-observation-info", &asn_DEF_GNSS_RTK_CommonObservationInfo_r15, &x);
}

void LppPossibBuilder::process(GNSS_RTK_AuxiliaryStationData_r15 const& x) {
    basic_log("gnss-rtk-auxiliary-station-data", &asn_DEF_GNSS_RTK_AuxiliaryStationData_r15, &x);
}

void LppPossibBuilder::process(GNSS_SSR_CorrectionPoints_r16 const& x) {
    basic_log("gnss-ssr-corretion-points", &asn_DEF_GNSS_SSR_CorrectionPoints_r16, &x);
}

void LppPossibBuilder::process(GNSS_Integrity_ServiceParameters_r17 const& x) {
    basic_log("gnss-integrity-service-parameters", &asn_DEF_GNSS_Integrity_ServiceParameters_r17,
              &x);
}

void LppPossibBuilder::process(GNSS_Integrity_ServiceAlert_r17 const& x) {
    basic_log("gnss-integrity-service-alert", &asn_DEF_GNSS_Integrity_ServiceAlert_r17, &x);
}

void LppPossibBuilder::process(GNSS_LOS_NLOS_GridPoints_r18 const& x) {
    basic_log("gnss-los-nlos-grid-points", &asn_DEF_GNSS_LOS_NLOS_GridPoints_r18, &x);
}

void LppPossibBuilder::process(GNSS_SSR_IOD_Update_r18 const& x) {
    basic_log("gnss-ssr-iod-update", &asn_DEF_GNSS_SSR_IOD_Update_r18, &x);
}

void LppPossibBuilder::process(GNSS_CommonAssistData const& x) {
    VSCOPE_FUNCTIONF("GNSS_CommonAssistData");
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

void LppPossibBuilder::process(long id, GNSS_TimeModelList const& x) {
    basic_log(id, "gnss-time-model-list", &asn_DEF_GNSS_TimeModelList, &x);
}

void LppPossibBuilder::process(long id, GNSS_DifferentialCorrections const& x) {
    basic_log(id, "gnss-differential-corrections", &asn_DEF_GNSS_DifferentialCorrections, &x);
}

void LppPossibBuilder::process(long id, GNSS_NavigationModel const& x) {
    basic_log(id, "gnss-navigation-model", &asn_DEF_GNSS_NavigationModel, &x);
}

void LppPossibBuilder::process(long id, GNSS_RealTimeIntegrity const& x) {
    basic_log(id, "gnss-real-time-integrity", &asn_DEF_GNSS_RealTimeIntegrity, &x);
}

void LppPossibBuilder::process(long id, GNSS_DataBitAssistance const& x) {
    basic_log(id, "gnss-data-bit-assistance", &asn_DEF_GNSS_DataBitAssistance, &x);
}

void LppPossibBuilder::process(long id, GNSS_AcquisitionAssistance const& x) {
    basic_log(id, "gnss-acquisition-assistance", &asn_DEF_GNSS_AcquisitionAssistance, &x);
}

void LppPossibBuilder::process(long id, GNSS_Almanac const& x) {
    basic_log(id, "gnss-almanac", &asn_DEF_GNSS_Almanac, &x);
}

void LppPossibBuilder::process(long id, GNSS_UTC_Model const& x) {
    basic_log(id, "gnss-utc-model", &asn_DEF_GNSS_UTC_Model, &x);
}

void LppPossibBuilder::process(long id, GNSS_AuxiliaryInformation const& x) {
    basic_log(id, "gnss-axuiliary-information", &asn_DEF_GNSS_AuxiliaryInformation, &x);
}

void LppPossibBuilder::process(long id, BDS_DifferentialCorrections_r12 const& x) {
    basic_log(id, "bds-differential-corrections", &asn_DEF_BDS_DifferentialCorrections_r12, &x);
}

void LppPossibBuilder::process(long id, BDS_GridModelParameter_r12 const& x) {
    basic_log(id, "bds-grid-model-parameter", &asn_DEF_BDS_GridModelParameter_r12, &x);
}

void LppPossibBuilder::process(long id, GNSS_RTK_Observations_r15 const& x) {
    basic_log(id, "gnss-rtk-observations", &asn_DEF_GNSS_RTK_Observations_r15, &x);
}

void LppPossibBuilder::process(long id, GLO_RTK_BiasInformation_r15 const& x) {
    basic_log(id, "glo-rtk-bias-information", &asn_DEF_GLO_RTK_BiasInformation_r15, &x);
}

void LppPossibBuilder::process(long id, GNSS_RTK_MAC_CorrectionDifferences_r15 const& x) {
    basic_log(id, "gnss-rtk-max-correction-differences",
              &asn_DEF_GNSS_RTK_MAC_CorrectionDifferences_r15, &x);
}

void LppPossibBuilder::process(long id, GNSS_RTK_Residuals_r15 const& x) {
    basic_log(id, "gnss-rtk-residuals", &asn_DEF_GNSS_RTK_Residuals_r15, &x);
}

void LppPossibBuilder::process(long id, GNSS_RTK_FKP_Gradients_r15 const& x) {
    basic_log(id, "gnss-rtk-fkp-gradients", &asn_DEF_GNSS_RTK_FKP_Gradients_r15, &x);
}

std::vector<size_t> LppPossibBuilder::encode_list_to_sizes(asn_TYPE_descriptor_s* descriptor,
                                                           void const** struct_ptr, int n) {
    std::vector<size_t> result;
    for (int i = 0; i < n; ++i) {
        result.push_back(encode_to_buffer(descriptor, struct_ptr[i]).size());
    }
    return result;
}

static std::string to_json_array(std::vector<size_t> const& sizes) {
    std::stringstream result;
    result << "[";
    bool first = true;
    for (auto size : sizes) {
        if (!first) {
            result << ",";
        }
        result << size;
        first = false;
    }
    result << "]";
    return result.str();
}

void LppPossibBuilder::process(long id, GNSS_SSR_OrbitCorrections_r15 const& x) {
    auto& list            = x.ssr_OrbitCorrectionList_r15.list;
    auto  satellite_count = list.count;
    auto  satellite_sizes = encode_list_to_sizes(&asn_DEF_SSR_OrbitCorrectionSatelliteElement_r15,
                                                 (void const**)list.array, list.count);

    std::unordered_map<std::string, std::string> params;
    params["gnss_id"]         = gnss_name_from_id(id);
    params["satellite_count"] = std::to_string(satellite_count);
    params["satellite_sizes"] = to_json_array(satellite_sizes);
    log("gnss-ssr-orbit-corrections", &asn_DEF_GNSS_SSR_OrbitCorrections_r15, &x, params);
}

void LppPossibBuilder::process(long id, GNSS_SSR_ClockCorrections_r15 const& x) {
    auto& list            = x.ssr_ClockCorrectionList_r15.list;
    auto  satellite_count = list.count;
    auto  satellite_sizes = encode_list_to_sizes(&asn_DEF_SSR_ClockCorrectionSatelliteElement_r15,
                                                 (void const**)list.array, list.count);

    std::unordered_map<std::string, std::string> params;
    params["gnss_id"]         = gnss_name_from_id(id);
    params["satellite_count"] = std::to_string(satellite_count);
    params["satellite_sizes"] = to_json_array(satellite_sizes);
    log("gnss-ssr-clock-corrections", &asn_DEF_GNSS_SSR_ClockCorrections_r15, &x, params);
}

void LppPossibBuilder::process(long id, GNSS_SSR_CodeBias_r15 const& x) {
    auto& list            = x.ssr_CodeBiasSatList_r15.list;
    auto  satellite_count = list.count;
    auto  satellite_sizes = encode_list_to_sizes(&asn_DEF_SSR_CodeBiasSatElement_r15,
                                                 (void const**)list.array, list.count);

    auto signal_count = 0;
    auto signal_sizes = std::vector<size_t>();
    for (int i = 0; i < satellite_count; i++) {
        auto& element = list.array[i];
        if (!element) continue;

        signal_count += element->ssr_CodeBiasSignalList_r15.list.count;
        for (auto size :
             encode_list_to_sizes(&asn_DEF_SSR_CodeBiasSignalElement_r15,
                                  (void const**)element->ssr_CodeBiasSignalList_r15.list.array,
                                  element->ssr_CodeBiasSignalList_r15.list.count)) {
            signal_sizes.push_back(size);
        }
    }

    std::unordered_map<std::string, std::string> params;
    params["gnss_id"]         = gnss_name_from_id(id);
    params["satellite_count"] = std::to_string(satellite_count);
    params["satellite_sizes"] = to_json_array(satellite_sizes);
    params["signal_count"]    = std::to_string(signal_count);
    params["signal_sizes"]    = to_json_array(signal_sizes);
    log("gnss-ssr-code-bias", &asn_DEF_GNSS_SSR_CodeBias_r15, &x, params);
}

void LppPossibBuilder::process(long id, GNSS_SSR_URA_r16 const& x) {
    auto& list            = x.ssr_URA_SatList_r16.list;
    auto  satellite_count = list.count;
    auto  satellite_sizes =
        encode_list_to_sizes(&asn_DEF_SSR_URA_SatElement_r16, (void const**)list.array, list.count);

    std::unordered_map<std::string, std::string> params;
    params["gnss_id"]         = gnss_name_from_id(id);
    params["satellite_count"] = std::to_string(satellite_count);
    params["satellite_sizes"] = to_json_array(satellite_sizes);
    log("gnss-ssr-ura", &asn_DEF_GNSS_SSR_URA_r16, &x, params);
}

void LppPossibBuilder::process(long id, GNSS_SSR_PhaseBias_r16 const& x) {
    auto& list            = x.ssr_PhaseBiasSatList_r16.list;
    auto  satellite_count = list.count;
    auto  satellite_sizes = encode_list_to_sizes(&asn_DEF_SSR_PhaseBiasSatElement_r16,
                                                 (void const**)list.array, list.count);

    auto signal_count = 0;
    auto signal_sizes = std::vector<size_t>();
    for (int i = 0; i < satellite_count; i++) {
        auto& element = list.array[i];
        if (!element) continue;

        signal_count += element->ssr_PhaseBiasSignalList_r16.list.count;
        for (auto size :
             encode_list_to_sizes(&asn_DEF_SSR_PhaseBiasSignalElement_r16,
                                  (void const**)element->ssr_PhaseBiasSignalList_r16.list.array,
                                  element->ssr_PhaseBiasSignalList_r16.list.count)) {
            signal_sizes.push_back(size);
        }
    }

    std::unordered_map<std::string, std::string> params;
    params["gnss_id"]         = gnss_name_from_id(id);
    params["satellite_count"] = std::to_string(satellite_count);
    params["satellite_sizes"] = to_json_array(satellite_sizes);
    params["signal_count"]    = std::to_string(signal_count);
    params["signal_sizes"]    = to_json_array(signal_sizes);
    log("gnss-ssr-phase-bias", &asn_DEF_GNSS_SSR_PhaseBias_r16, &x, params);
}

void LppPossibBuilder::process(long id, GNSS_SSR_STEC_Correction_r16 const& x) {
    basic_log(id, "gnss-ssr-stec-corrections", &asn_DEF_GNSS_SSR_STEC_Correction_r16, &x);
}

void LppPossibBuilder::process(long id, GNSS_SSR_GriddedCorrection_r16 const& x) {
    basic_log(id, "gnss-ssr-gridded-correction", &asn_DEF_GNSS_SSR_GriddedCorrection_r16, &x);
}

void LppPossibBuilder::process(long id, NavIC_DifferentialCorrections_r16 const& x) {
    basic_log(id, "navic-differential-corrections", &asn_DEF_NavIC_DifferentialCorrections_r16, &x);
}

void LppPossibBuilder::process(long id, NavIC_GridModelParameter_r16 const& x) {
    basic_log(id, "navic-grid-model-parameter", &asn_DEF_NavIC_GridModelParameter_r16, &x);
}

void LppPossibBuilder::process(long id, GNSS_SSR_OrbitCorrectionsSet2_r17 const& x) {
    basic_log(id, "gnss-ssr-orbit-corrections-set2", &asn_DEF_GNSS_SSR_OrbitCorrectionsSet2_r17,
              &x);
}

void LppPossibBuilder::process(long id, GNSS_SSR_ClockCorrectionsSet2_r17 const& x) {
    basic_log(id, "gnss-ssr-clock-corrections-set2", &asn_DEF_GNSS_SSR_ClockCorrectionsSet2_r17,
              &x);
}

void LppPossibBuilder::process(long id, GNSS_SSR_URA_Set2_r17 const& x) {
    basic_log(id, "gnss-ssr-ura-set2", &asn_DEF_GNSS_SSR_URA_Set2_r17, &x);
}

void LppPossibBuilder::process(long id, GNSS_LOS_NLOS_GriddedIndications_r18 const& x) {
    basic_log(id, "gnss-los-nlos-gridded-indications",
              &asn_DEF_GNSS_LOS_NLOS_GriddedIndications_r18, &x);
}

void LppPossibBuilder::process(long id, GNSS_SSR_SatellitePCVResiduals_r18 const& x) {
    basic_log(id, "gnss-ssr-satellite-pcv-residuals", &asn_DEF_GNSS_SSR_SatellitePCVResiduals_r18,
              &x);
}

void LppPossibBuilder::process(GNSS_GenericAssistDataElement const& x) {
    auto id = x.gnss_ID.gnss_id;
    VSCOPE_FUNCTIONF("GNSS_GenericAssistDataElement,id=%d", id);

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

void LppPossibBuilder::process(GNSS_GenericAssistData const& message) {
    VSCOPE_FUNCTIONF("GNSS_GenericAssistData");
    for (int i = 0; i < message.list.count; i++) {
        auto element = message.list.array[i];
        if (element) process(*element);
    }
}

void LppPossibBuilder::process(A_GNSS_ProvideAssistanceData const& message) {
    VSCOPE_FUNCTIONF("A_GNSS_ProvideAssistanceData");
    if (message.gnss_CommonAssistData) process(*message.gnss_CommonAssistData);
    if (message.gnss_GenericAssistData) process(*message.gnss_GenericAssistData);
}

void LppPossibBuilder::process(CommonIEsProvideAssistanceData const&) {
    VSCOPE_FUNCTIONF("CommonIEsProvideAssistanceData");
}

void LppPossibBuilder::process(ProvideAssistanceData_r9_IEs const& message) {
    VSCOPE_FUNCTIONF("ProvideAssistanceData_r9_IEs");
    if (message.commonIEsProvideAssistanceData) process(*message.commonIEsProvideAssistanceData);
    if (message.a_gnss_ProvideAssistanceData) process(*message.a_gnss_ProvideAssistanceData);
}

void LppPossibBuilder::process(ProvideAssistanceData const& message) {
    VSCOPE_FUNCTIONF("ProvideAssistanceData");

    switch (message.criticalExtensions.present) {
    case ProvideAssistanceData__criticalExtensions_PR::
        ProvideAssistanceData__criticalExtensions_PR_c1: {
        switch (message.criticalExtensions.choice.c1.present) {
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9:
            return process(message.criticalExtensions.choice.c1.choice.provideAssistanceData_r9);
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_NOTHING:
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_spare3:
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_spare2:
        case ProvideAssistanceData__criticalExtensions__c1_PR::
            ProvideAssistanceData__criticalExtensions__c1_PR_spare1:
            break;
        }
    }
    case ProvideAssistanceData__criticalExtensions_PR::
        ProvideAssistanceData__criticalExtensions_PR_NOTHING:
    case ProvideAssistanceData__criticalExtensions_PR::
        ProvideAssistanceData__criticalExtensions_PR_criticalExtensionsFuture:
        break;
    }
}

void LppPossibBuilder::inspect(streamline::System& system, DataType const& message) noexcept {
    VSCOPE_FUNCTION();

    if (!message) return;

    auto body = message->lpp_MessageBody;
    if (!body) return;

    mSystem = &system;

    VERBOSEF("posSIB logger: processing %s", typeid(*message).name());

    switch (body->present) {
    case LPP_MessageBody_PR::LPP_MessageBody_PR_c1: {
        switch (body->choice.c1.present) {
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_provideAssistanceData: {
            process(body->choice.c1.choice.provideAssistanceData);
        } break;

        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_NOTHING:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_requestCapabilities:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_provideCapabilities:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_requestAssistanceData:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_requestLocationInformation:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_provideLocationInformation:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_abort:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_error:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare7:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare6:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare5:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare4:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare3:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare2:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare1:
        case LPP_MessageBody__c1_PR::LPP_MessageBody__c1_PR_spare0: break;
        }
    }
    case LPP_MessageBody_PR::LPP_MessageBody_PR_NOTHING:
    case LPP_MessageBody_PR::LPP_MessageBody_PR_messageClassExtension: break;
    }

    mSystem = nullptr;
}

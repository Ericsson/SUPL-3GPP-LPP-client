#pragma once

#ifdef DATA_TRACING
#include <fstream>
#include <unordered_map>

#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include "lpp.hpp"

struct GNSS_ReferenceTime;
struct GNSS_ReferenceLocation;
struct GNSS_IonosphericModel;
struct GNSS_EarthOrientationParameters;
struct GNSS_RTK_ReferenceStationInfo_r15;
struct GNSS_RTK_CommonObservationInfo_r15;
struct GNSS_RTK_AuxiliaryStationData_r15;
struct GNSS_SSR_CorrectionPoints_r16;
struct GNSS_Integrity_ServiceParameters_r17;
struct GNSS_Integrity_ServiceAlert_r17;
struct GNSS_TimeModelList;
struct GNSS_DifferentialCorrections;
struct GNSS_NavigationModel;
struct GNSS_RealTimeIntegrity;
struct GNSS_DataBitAssistance;
struct GNSS_AcquisitionAssistance;
struct GNSS_Almanac;
struct GNSS_UTC_Model;
struct GNSS_AuxiliaryInformation;
struct BDS_DifferentialCorrections_r12;
struct BDS_GridModelParameter_r12;
struct GNSS_RTK_Observations_r15;
struct GLO_RTK_BiasInformation_r15;
struct GNSS_RTK_MAC_CorrectionDifferences_r15;
struct GNSS_RTK_Residuals_r15;
struct GNSS_RTK_FKP_Gradients_r15;
struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_URA_r16;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;
struct NavIC_DifferentialCorrections_r16;
struct NavIC_GridModelParameter_r16;
struct GNSS_SSR_OrbitCorrectionsSet2_r17;
struct GNSS_SSR_ClockCorrectionsSet2_r17;
struct GNSS_SSR_URA_Set2_r17;
struct GNSS_LOS_NLOS_GriddedIndications_r18;
struct GNSS_SSR_SatellitePCVResiduals_r18;
struct asn_TYPE_descriptor_s;
struct GNSS_SSR_IOD_Update_r18;
struct GNSS_CommonAssistData;
struct GNSS_LOS_NLOS_GridPoints_r18;
struct GNSS_GenericAssistDataElement;
struct GNSS_GenericAssistData;
struct A_GNSS_ProvideAssistanceData;
struct ProvideAssistanceData_r9_IEs;
struct ProvideAssistanceData;
struct CommonIEsProvideAssistanceData;

class PossibMessage {
public:
    PossibMessage(std::string json) : mJson(std::move(json)) {}

    std::string const& json() const { return mJson; }

private:
    std::string mJson;
};

namespace streamline {
template <>
struct Clone<std::unique_ptr<PossibMessage>> {
    std::unique_ptr<PossibMessage> operator()(std::unique_ptr<PossibMessage> const&) {
        __builtin_unreachable();
    }
};
}  // namespace streamline

class PossibOutput : public streamline::Inspector<std::unique_ptr<PossibMessage>> {
public:
    PossibOutput(OutputConfig const& output) : mOutput(output) {}

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

private:
    OutputConfig const& mOutput;
};

class LppPossibBuilder : public streamline::Inspector<lpp::Message> {
public:
    LppPossibBuilder(bool wrap);
    ~LppPossibBuilder() override;

    void inspect(streamline::System&, DataType const& message) NOEXCEPT override;

protected:
    std::vector<uint8_t> encode_to_buffer(asn_TYPE_descriptor_s* descriptor,
                                          void const*            struct_ptr);
    std::vector<size_t>  encode_list_to_sizes(asn_TYPE_descriptor_s* descriptor,
                                              void const** struct_ptr, int n);

    void log(char const* type, asn_TYPE_descriptor_s* def, void const* ptr,
             std::unordered_map<std::string, std::string> const& params);
    void basic_log(char const* type, asn_TYPE_descriptor_s* def, void const* ptr);
    void basic_log(long id, char const* type, asn_TYPE_descriptor_s* def, void const* ptr);

    void process(GNSS_ReferenceTime const& x);
    void process(GNSS_ReferenceLocation const& x);
    void process(GNSS_IonosphericModel const& x);
    void process(GNSS_EarthOrientationParameters const& x);
    void process(GNSS_RTK_ReferenceStationInfo_r15 const& x);
    void process(GNSS_RTK_CommonObservationInfo_r15 const& x);
    void process(GNSS_RTK_AuxiliaryStationData_r15 const& x);
    void process(GNSS_SSR_CorrectionPoints_r16 const& x);
    void process(GNSS_Integrity_ServiceParameters_r17 const& x);
    void process(GNSS_Integrity_ServiceAlert_r17 const& x);
    void process(GNSS_LOS_NLOS_GridPoints_r18 const& x);
    void process(GNSS_SSR_IOD_Update_r18 const& x);
    void process(GNSS_CommonAssistData const& x);

    void process(long id, GNSS_TimeModelList const& x);
    void process(long id, GNSS_DifferentialCorrections const& x);
    void process(long id, GNSS_NavigationModel const& x);
    void process(long id, GNSS_RealTimeIntegrity const& x);
    void process(long id, GNSS_DataBitAssistance const& x);
    void process(long id, GNSS_AcquisitionAssistance const& x);
    void process(long id, GNSS_Almanac const& x);
    void process(long id, GNSS_UTC_Model const& x);
    void process(long id, GNSS_AuxiliaryInformation const& x);
    void process(long id, BDS_DifferentialCorrections_r12 const& x);
    void process(long id, BDS_GridModelParameter_r12 const& x);
    void process(long id, GNSS_RTK_Observations_r15 const& x);
    void process(long id, GLO_RTK_BiasInformation_r15 const& x);
    void process(long id, GNSS_RTK_MAC_CorrectionDifferences_r15 const& x);
    void process(long id, GNSS_RTK_Residuals_r15 const& x);
    void process(long id, GNSS_RTK_FKP_Gradients_r15 const& x);
    void process(long id, GNSS_SSR_OrbitCorrections_r15 const& x);
    void process(long id, GNSS_SSR_ClockCorrections_r15 const& x);
    void process(long id, GNSS_SSR_CodeBias_r15 const& x);
    void process(long id, GNSS_SSR_URA_r16 const& x);
    void process(long id, GNSS_SSR_PhaseBias_r16 const& x);
    void process(long id, GNSS_SSR_STEC_Correction_r16 const& x);
    void process(long id, GNSS_SSR_GriddedCorrection_r16 const& x);
    void process(long id, NavIC_DifferentialCorrections_r16 const& x);
    void process(long id, NavIC_GridModelParameter_r16 const& x);
    void process(long id, GNSS_SSR_OrbitCorrectionsSet2_r17 const& x);
    void process(long id, GNSS_SSR_ClockCorrectionsSet2_r17 const& x);
    void process(long id, GNSS_SSR_URA_Set2_r17 const& x);
    void process(long id, GNSS_LOS_NLOS_GriddedIndications_r18 const& x);
    void process(long id, GNSS_SSR_SatellitePCVResiduals_r18 const& x);
    void process(GNSS_GenericAssistDataElement const& x);

    void process(GNSS_GenericAssistData const& message);
    void process(A_GNSS_ProvideAssistanceData const& message);
    void process(CommonIEsProvideAssistanceData const& message);
    void process(ProvideAssistanceData_r9_IEs const& message);
    void process(ProvideAssistanceData const& message);

private:
    void*               mEncodeBuffer;
    size_t              mEncodeBufferSize;
    bool                mIncludeWrap;
    streamline::System* mSystem;
};

#endif

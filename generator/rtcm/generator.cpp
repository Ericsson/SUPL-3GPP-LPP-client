#include "generator.hpp"
#include "extract.hpp"
#include "maybe.hpp"
#include "rtk_data.hpp"
#include "satellite_id.hpp"

#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>

namespace decode {


}  // namespace decode



static void extract_common_assist_data(RtkData& data, const GNSS_CommonAssistData& src_common) {
    if (src_common.ext1) {
        auto& ext1 = *src_common.ext1;
        if (ext1.gnss_RTK_CommonObservationInfo_r15) {
            extract_common_observation_info(data, *ext1.gnss_RTK_CommonObservationInfo_r15);
        }

        if (ext1.gnss_RTK_ReferenceStationInfo_r15) {
            extract_reference_station_info(data, *ext1.gnss_RTK_ReferenceStationInfo_r15);
        }
    }
}

static void extract_generic_element_data(RtkData&                             data,
                                         const GNSS_GenericAssistDataElement& src_generic) {
    auto gnss_id = GenericGnssId::GPS;
    switch (src_generic.gnss_ID.gnss_id) {
    case GNSS_ID__gnss_id_gps: gnss_id = GenericGnssId::GPS; break;
    case GNSS_ID__gnss_id_glonass: gnss_id = GenericGnssId::GLONASS; break;
    case GNSS_ID__gnss_id_galileo: gnss_id = GenericGnssId::GALILEO; break;
    case GNSS_ID__gnss_id_bds: gnss_id = GenericGnssId::BEIDOU; break;
    case GNSS_ID__gnss_id_sbas:
    case GNSS_ID__gnss_id_qzss:
    case GNSS_ID__gnss_id_navic_v16xy: return;
    }

    if(src_generic.ext2) {
        auto& ext2 = *src_generic.ext2;

        if(ext2.gnss_RTK_Observations_r15) {
            extract_observations(data, gnss_id, *ext2.gnss_RTK_Observations_r15);
        }
    }
}

static void extract_generic_assist_data(RtkData& data, const GNSS_GenericAssistData& src_generic) {
    auto& list = src_generic.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        extract_generic_element_data(data, *list.array[i]);
    }
}

static std::unique_ptr<RtkData> extract_rtk_data(const LPP_Message& lpp_message) {
    if (!lpp_message.lpp_MessageBody) return nullptr;

    auto& body = *lpp_message.lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return nullptr;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return nullptr;

    auto& assistance_data     = body.choice.c1.choice.provideAssistanceData;
    auto& critical_extensions = assistance_data.criticalExtensions;
    if (critical_extensions.present != ProvideAssistanceData__criticalExtensions_PR_c1)
        return nullptr;
    if (critical_extensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return nullptr;

    auto& assistance_data_r15 = critical_extensions.choice.c1.choice.provideAssistanceData_r9;
    if (!assistance_data_r15.a_gnss_ProvideAssistanceData) return nullptr;

    auto data = std::unique_ptr<RtkData>(new RtkData());

    auto& a_gnss_provide_assistance_data = *assistance_data_r15.a_gnss_ProvideAssistanceData;
    if (a_gnss_provide_assistance_data.gnss_CommonAssistData) {
        extract_common_assist_data(*data.get(),
                                   *a_gnss_provide_assistance_data.gnss_CommonAssistData);
    }

    if (a_gnss_provide_assistance_data.gnss_GenericAssistData) {
        extract_generic_assist_data(*data.get(),
                                    *a_gnss_provide_assistance_data.gnss_GenericAssistData);
    }

    return data;
}

namespace generator {
namespace rtcm {

Generator::Generator() : mObservationData(nullptr) {}

Generator::~Generator() = default;

std::vector<Message> Generator::generate(const LPP_Message*   lpp_message,
                                         const MessageFilter& filter) {
    if (lpp_message) {
        auto rtk_data = extract_rtk_data(*lpp_message);

        if (rtk_data) {
            printf("-- RTK DATA\n");
            if (rtk_data->common_observation_info) printf("---- COMMON OBSERVATION INFO\n");
            if (rtk_data->reference_station_info) printf("---- REFERENCE STATION INFO\n");
            if (rtk_data->physical_reference_station_info)
                printf("---- PHYSICAL REFERENCE STATION INFO\n");

            for(auto& kvp : rtk_data->observations) {
                printf("---- OBSERVATIONS: ");
                switch(kvp.first) {
                case GenericGnssId::GPS: printf("GPS\n"); break;
                case GenericGnssId::GLONASS: printf("GLONASS\n"); break;
                case GenericGnssId::GALILEO: printf("GALILEO\n"); break;
                case GenericGnssId::BEIDOU: printf("BEIDOU\n"); break;
                }
                printf(" satellites: %2zu, signals: %2zu\n", kvp.second->satellites.size(), kvp.second->signals.size());
            }
        }
    }

    return {};
}

}  // namespace rtcm
}  // namespace generator

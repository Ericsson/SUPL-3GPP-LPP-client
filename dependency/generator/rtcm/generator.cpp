#include "generator.hpp"
#include "encoder.hpp"
#include "extract/extract.hpp"
#include "maybe.hpp"
#include "messages/1230.hpp"
#include "messages/msm.hpp"
#include "messages/reference_station.hpp"
#include "messages/residuals.hpp"
#include "rtk_data.hpp"
#include "satellite_id.hpp"

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#pragma GCC diagnostic pop

using namespace generator::rtcm;

static void extract_common_assist_data(RtkData& data, GNSS_CommonAssistData const& src_common) {
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
                                         GNSS_GenericAssistDataElement const& src_generic) {
    auto gnss_id = GenericGnssId::GPS;
    switch (src_generic.gnss_ID.gnss_id) {
    case GNSS_ID__gnss_id_gps: gnss_id = GenericGnssId::GPS; break;
    case GNSS_ID__gnss_id_glonass: gnss_id = GenericGnssId::GLONASS; break;
    case GNSS_ID__gnss_id_galileo: gnss_id = GenericGnssId::GALILEO; break;
    case GNSS_ID__gnss_id_bds: gnss_id = GenericGnssId::BEIDOU; break;
    case GNSS_ID__gnss_id_sbas:
    case GNSS_ID__gnss_id_qzss:
    case GNSS_ID__gnss_id_navic_v1610: return;
    }

    if (src_generic.gnss_AuxiliaryInformation) {
        extract_auxiliary_information(data, *src_generic.gnss_AuxiliaryInformation);
    }

    if (src_generic.ext2) {
        auto& ext2 = *src_generic.ext2;

        if (ext2.gnss_RTK_Observations_r15) {
            extract_observations(data, gnss_id, *ext2.gnss_RTK_Observations_r15);
        }

        if (ext2.glo_RTK_BiasInformation_r15 && gnss_id == GenericGnssId::GLONASS) {
            extract_bias_information(data, *ext2.glo_RTK_BiasInformation_r15);
        }

        if (ext2.gnss_RTK_Residuals_r15) {
            extract_residuals(data, gnss_id, *ext2.gnss_RTK_Residuals_r15);
        }
    }
}

static void extract_generic_assist_data(RtkData& data, GNSS_GenericAssistData const& src_generic) {
    auto& list = src_generic.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        extract_generic_element_data(data, *list.array[i]);
    }
}

static std::unique_ptr<RtkData> extract_rtk_data(LPP_Message const& lpp_message) {
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

static GenericGnssId MSM_GNSS_MESSAGE_ORDER[] = {
    GenericGnssId::GPS,
    GenericGnssId::GLONASS,
    GenericGnssId::GALILEO,
    GenericGnssId::BEIDOU,
};

namespace generator {
namespace rtcm {

Generator::Generator()
    : mGenerationIndex(0), mReferenceStation(nullptr), mPhysicalReferenceStation(nullptr) {}

Generator::~Generator() = default;

std::vector<Message> Generator::generate(LPP_Message const*   lpp_message,
                                         MessageFilter const& filter) {
    std::vector<Message> messages;
    if (!lpp_message) return messages;

    auto rtk_data = extract_rtk_data(*lpp_message);
    if (!rtk_data) return messages;

#ifdef DATA_TRACING
    for (auto& kvp : rtk_data->observations) {
        auto& observation = *kvp.second;
        for (auto& satellite : observation.satellites) {
            for (auto& signal : observation.signals) {
                if (satellite.id != signal.satellite) continue;
                if (!satellite.integer_ms.valid) continue;
                if (!satellite.rough_range.valid) continue;

                auto code_range = 0.0;
                code_range += static_cast<double>(satellite.integer_ms.value);
                code_range += satellite.rough_range.value;
                if (signal.fine_pseudo_range.valid) {
                    code_range += signal.fine_pseudo_range.value;
                }

                auto phase_range = 0.0;
                phase_range += static_cast<double>(satellite.integer_ms.value);
                phase_range += satellite.rough_range.value;
                if (signal.fine_phase_range.valid) {
                    phase_range += signal.fine_phase_range.value;
                }

                code_range *= 2.99792458e8 / 1.0e3;
                phase_range *= 2.99792458e8 / 1.0e3;

                datatrace::Observation dtobs{};
                dtobs.code_range  = code_range;
                dtobs.phase_range = phase_range;

                if (satellite.rough_phase_range_rate.valid) {
                    if (signal.fine_phase_range_rate.valid) {
                        dtobs.phase_range_rate = satellite.rough_phase_range_rate.value +
                                                 signal.fine_phase_range_rate.value;
                    }
                }

                if (signal.lock_time.valid) dtobs.phase_lock_time = signal.lock_time.value;
                if (signal.carrier_to_noise_ratio.valid)
                    dtobs.carrier_to_noise_ratio = signal.carrier_to_noise_ratio.value;
                datatrace::report_observation(observation.time, satellite.id.name(),
                                              signal.id.name(), dtobs);
            }
        }
    }
#endif

    // Get frequency channel etc. from auxiliary information
    if (rtk_data->auxiliary_information) {
        auto& aux = *rtk_data->auxiliary_information.get();
        for (auto& kvp : rtk_data->observations) {
            for (auto& satellite : kvp.second->satellites) {
                auto it = aux.satellites.find(satellite.id);
                if (it != aux.satellites.end()) {
                    satellite.frequency_channel = it->second.frequency_channel;
                }
            }
        }
    }

    // Purge data that should not be used
    if (!filter.systems.gps) rtk_data->observations.erase(GenericGnssId::GPS);
    if (!filter.systems.glonass) rtk_data->observations.erase(GenericGnssId::GLONASS);
    if (!filter.systems.galileo) rtk_data->observations.erase(GenericGnssId::GALILEO);
    if (!filter.systems.beidou) rtk_data->observations.erase(GenericGnssId::BEIDOU);

    auto any_msm       = filter.msm.msm4 || filter.msm.msm5 || filter.msm.msm6 || filter.msm.msm7;
    auto any_force_msm = filter.msm.force_msm4 || filter.msm.force_msm5 || filter.msm.force_msm6 ||
                         filter.msm.force_msm7;
    if (!any_msm && !any_force_msm) {
        rtk_data->observations.clear();
    }

    // Calculate lowest MSM version that can be used.
    uint32_t lowest_msm = 0;
    for (auto& kvp : rtk_data->observations) {
        auto gnss_msm = kvp.second->lowest_msm_version();
        if (gnss_msm > lowest_msm) lowest_msm = gnss_msm;
    }

    if (lowest_msm == 4 && !filter.msm.msm4) lowest_msm = 5;
    if (lowest_msm == 5 && !filter.msm.msm5) lowest_msm = 6;
    if (lowest_msm == 6 && !filter.msm.msm6) lowest_msm = 7;
    if (lowest_msm == 7 && !filter.msm.msm7) lowest_msm = 0;

    if (filter.msm.force_msm4) lowest_msm = 4;
    if (filter.msm.force_msm5) lowest_msm = 5;
    if (filter.msm.force_msm6) lowest_msm = 6;
    if (filter.msm.force_msm7) lowest_msm = 7;

    if (lowest_msm == 0) {
        lowest_msm = 4;  // TODO: What should we do here?
    }

    auto should_include_reference_station = false;
    if (rtk_data->reference_station) {
        should_include_reference_station = true;
    }
    if (filter.reference_station.include_every_nth_generation > 0) {
        if ((mGenerationIndex % filter.reference_station.include_every_nth_generation) == 0) {
            should_include_reference_station = true;
        }
    }

    auto gps_indicator = rtk_data->observations.count(GenericGnssId::GPS) > 0;
    auto glo_indicator = rtk_data->observations.count(GenericGnssId::GLONASS) > 0;
    auto gal_indicator = rtk_data->observations.count(GenericGnssId::GALILEO) > 0;

    if (should_include_reference_station) {
        auto reference_station_ptr          = mReferenceStation.get();
        auto physical_reference_station_ptr = mPhysicalReferenceStation.get();
        if (rtk_data->reference_station) reference_station_ptr = rtk_data->reference_station.get();
        if (rtk_data->physical_reference_station)
            physical_reference_station_ptr = rtk_data->physical_reference_station.get();

        // Generate Reference Station message
        if (reference_station_ptr) {
            auto& reference_station = *reference_station_ptr;
            if (filter.reference_station.mt1006 && reference_station.antenna_height.valid) {
                auto message =
                    generate_1006(reference_station, gps_indicator, glo_indicator, gal_indicator);
                messages.emplace_back(std::move(message));
            } else if (filter.reference_station.mt1005) {
                auto message =
                    generate_1005(reference_station, gps_indicator, glo_indicator, gal_indicator);
                messages.emplace_back(std::move(message));
            }

            // Generate Physical Reference Station message
            if (physical_reference_station_ptr) {
                auto& physical_reference_station = *physical_reference_station_ptr;
                if (filter.reference_station.mt1032) {
                    auto message = generate_1032(reference_station, physical_reference_station);
                    messages.emplace_back(std::move(message));
                }
            }
        }
    }

    // Generate MSM messages
    if (rtk_data->common_observation_info && rtk_data->observations.size() > 0) {
        auto& common = *rtk_data->common_observation_info.get();

        GenericGnssId       gnss[4];
        Observations const* observations[4] = {nullptr, nullptr, nullptr, nullptr};
        auto                count           = 0;
        for (auto i = 0; i < 4; i++) {
            auto it = rtk_data->observations.find(MSM_GNSS_MESSAGE_ORDER[i]);
            if (it == rtk_data->observations.end()) continue;
            gnss[count]         = it->first;
            observations[count] = it->second.get();
            count++;
        }

        for (auto i = 0; i < count; i++) {
            auto message =
                generate_msm(lowest_msm, i + 1 == count, gnss[i], common, *observations[i]);
            messages.emplace_back(std::move(message));
        }
    }

    // Generate GLONASS bias information message
    if (rtk_data->bias_information) {
        auto message = generate_1230(*rtk_data->bias_information.get());
        messages.emplace_back(std::move(message));
    }

    // Generate residuals messages
    if (rtk_data->gps_residuals) {
        auto message = generate_1030(*rtk_data->gps_residuals.get());
        messages.emplace_back(std::move(message));
    }

    if (rtk_data->glonass_residuals) {
        auto message = generate_1031(*rtk_data->glonass_residuals.get());
        messages.emplace_back(std::move(message));
    }

    // Update reference station
    if (rtk_data->reference_station) {
        mReferenceStation = std::move(rtk_data->reference_station);
    }
    if (rtk_data->physical_reference_station) {
        mPhysicalReferenceStation = std::move(rtk_data->physical_reference_station);
    }

    // Increment generation index
    mGenerationIndex++;
    return messages;
}

// We are using a undocument RTCM message id (100-1000). Taking 355 as that is the ending of the
// 3GPP LPP specification 37.355.
static CONSTEXPR uint16_t LRF_MESSAGE_ID = 355;

static Message generate_framing_message(int message_id, bool multiple_message_bit,
                                        uint8_t const* lpp_data, size_t lpp_data_size) {
    if (message_id < 0 || message_id > 4095) {
        message_id = LRF_MESSAGE_ID;
    }

    auto encoder = Encoder();
    encoder.u16(12, static_cast<uint16_t>(message_id));
    encoder.u8(4, 0 /* subtype */);
    encoder.u8(1, multiple_message_bit ? 1 : 0);
    encoder.u8(7, 0 /* reserved */);
    encoder.copy(lpp_data, lpp_data_size);

    auto length = static_cast<uint16_t>(encoder.byte_count());

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, length);
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(LRF_MESSAGE_ID, frame_encoder.buffer());
}

std::vector<Message> Generator::generate_framing(int message_id, void const* lpp_data,
                                                 size_t lpp_data_size) NOEXCEPT {
    std::vector<Message> messages;

    auto ptr  = reinterpret_cast<uint8_t const*>(lpp_data);
    auto size = lpp_data_size;
    while (size > 0) {
        auto length = std::min(size, static_cast<size_t>(1020 /* 1023 - 3 byte (header) */));
        auto multiple_message_bit = size > length;
        auto message = generate_framing_message(message_id, multiple_message_bit, ptr, length);
        messages.emplace_back(std::move(message));
        ptr += length;
        size -= length;
    }

    return messages;
}

}  // namespace rtcm
}  // namespace generator

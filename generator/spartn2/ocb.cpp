#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"
#include "time.hpp"

#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-URA-r16.h>

#include <SSR-OrbitCorrectionList-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>

#include <algorithm>
#include <map>

namespace generator {
namespace spartn {

uint32_t OcbSatellite::prn() const {
    // NOTE(ewasjon): 3GPP LPP defines PRN starting at 0 instead of 1.
    return id + 1;
}

void OcbSatellite::add_correction(SSR_OrbitCorrectionSatelliteElement_r15* orbit) {
    if (!orbit) return;
    this->orbit = orbit;
}

void OcbSatellite::add_correction(SSR_ClockCorrectionSatelliteElement_r15* clock) {
    if (!clock) return;
    this->clock = clock;
}

void OcbSatellite::add_correction(SSR_CodeBiasSatElement_r15* code_bias) {
    if (!code_bias) return;
    this->code_bias = code_bias;
}

void OcbSatellite::add_correction(SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) return;
    this->phase_bias = phase_bias;
}

void OcbSatellite::add_correction(SSR_URA_SatElement_r16* ura) {
    if (!ura) return;
    this->ura = ura;
}

std::vector<OcbSatellite> OcbCorrections::satellites() const {
    std::unordered_map<long, OcbSatellite> satellites;

    // Orbit
    if (orbit) {
        auto& list = orbit->ssr_OrbitCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Clock
    if (clock) {
        auto& list = clock->ssr_ClockCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Code bias
    if (code_bias) {
        auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r15.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Phase bias
    if (phase_bias) {
        auto& list = phase_bias->ssr_PhaseBiasSatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r16.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // URA
    if (ura) {
        auto& list = ura->ssr_URA_SatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  satellite_id = element->svID_r16.satellite_id;
            auto& satellite    = satellites[satellite_id];
            satellite.id       = satellite_id;
            satellite.iod      = iod;
            satellite.add_correction(element);
        }
    }

    // Convert to vector
    std::vector<OcbSatellite> result;
    for (auto& kv : satellites) {
        result.push_back(kv.second);
    }

    // Sort by satellite id
    std::sort(result.begin(), result.end(), [](const OcbSatellite& a, const OcbSatellite& b) {
        return a.id < b.id;
    });

    return result;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit) {
    if (!orbit) return;
    auto  iod = orbit->iod_ssr_r15;
    auto& ocb = mOcbData[iod];

    // TODO(ewasjon): [low-priority] Filter based on satellite reference datum.
    auto epoch_time = spartn_time_from(orbit->epochTime_r15);
    auto key        = OcbKey{gnss_id, group_by_epoch_time ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.orbit      = orbit;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock) {
    if (!clock) return;
    auto  iod = clock->iod_ssr_r15;
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(clock->epochTime_r15);
    auto key        = OcbKey{gnss_id, group_by_epoch_time ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.clock      = clock;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias) {
    if (!code_bias) return;
    auto  iod = code_bias->iod_ssr_r15;
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(code_bias->epochTime_r15);
    auto key        = OcbKey{gnss_id, group_by_epoch_time ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.code_bias  = code_bias;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias) {
    if (!phase_bias) return;
    auto  iod = phase_bias->iod_ssr_r16;
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(phase_bias->epochTime_r16);
    auto key        = OcbKey{gnss_id, group_by_epoch_time ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.phase_bias = phase_bias;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_URA_r16* ura) {
    if (!ura) return;
    auto  iod = ura->iod_ssr_r16;
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(ura->epochTime_r16);
    auto key        = OcbKey{gnss_id, group_by_epoch_time ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.ura        = ura;
}

struct Bias {
    long    signal_id;
    double  correction;
    double  continuity_indicator;
    bool    fix_flag;
    uint8_t type;
    bool    mapped;

    static Bias invalid() { return Bias{-1, 0.0, 0.0, false, 0, false}; }
};

#define X 255

static SPARTN_CONSTEXPR uint8_t GPS_TO_SPARTN[24] = {
    0,  // L1 C/A -> L1C
    X,  // L1C
    X,  // L2C
    X,  // L5
    X,  // L1 P
    X,  // L1 Z-tracking
    X,  // L2 C/A
    X,  // L2 P
    1,  // L2 Z-tracking -> L2W
    X,  // L2 L2C(M)
    2,  // L2 L2C(L) -> L2L
    X,  // L2 L2C(M+L)
    X,  // L5 I
    3,  // L5 Q -> L5Q
    X,  // L5 I+Q
    X,  // L1 L1C(D)
    X,  // L1 L1C(P)
    X,  // L1 L1C(D+P)
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
};

static SPARTN_CONSTEXPR uint8_t GPS_MAPPING[32] = {
    X,   // L1 C/A
    X,   // L1C
    X,   // L2C
    X,   // L5
    X,   // L1 P
    X,   // L1 Z-tracking
    X,   // L2 C/A
    X,   // L2 P
    X,   // L2 Z-tracking
    X,   // L2 L2C(M)
    X,   // L2 L2C(L)
    10,  // L2 L2C(M+L) -> L2 L2C(L)
    X,   // L5 I
    X,   // L5 Q
    X,   // L5 I+Q
    X,   // L1 L1C(D)
    X,   // L1 L1C(P)
    X,   // L1 L1C(D+P)
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
};

#define GPS_L1 1575.42
#define GPS_L2 1227.60
#define GPS_L5 1176.45
static SPARTN_CONSTEXPR double GPS_FREQ[24] = {
    GPS_L1,  // L1 C/A
    GPS_L1,  // L1C
    GPS_L2,  // L2C
    GPS_L5,  // L5
    GPS_L1,  // L1 P
    GPS_L1,  // L1 Z-tracking
    GPS_L2,  // L2 C/A
    GPS_L2,  // L2 P
    GPS_L2,  // L2 Z-tracking
    GPS_L2,  // L2 L2C(M)
    GPS_L2,  // L2 L2C(L)
    GPS_L2,  // L2 L2C(M+L)
    GPS_L5,  // L5 I
    GPS_L5,  // L5 Q
    GPS_L5,  // L5 I+Q
    GPS_L1,  // L1 L1C(D)
    GPS_L1,  // L1 L1C(P)
    GPS_L1,  // L1 L1C(D+P)
    // Reserved
    0,
    0,
    0,
    0,
    0,
    0,
};

static Bias gps_bias_from_signal(long signal_id, double correction, double continuity_indicator,
                                 bool fix_flag) {
    if (signal_id >= 24) {
        return Bias::invalid();
    }

    auto type = GPS_TO_SPARTN[signal_id];
    if (type == X) {
        auto from_id = signal_id;
        auto to_id   = GPS_MAPPING[from_id];
        if (to_id == X) {
            return Bias::invalid();
        }

        auto from_freq          = GPS_FREQ[from_id];
        auto to_freq            = GPS_FREQ[to_id];
        auto scale              = from_freq / to_freq;
        auto shifted_correction = correction * scale;
#ifdef SPARTN_DEBUG_PRINT
        printf("  GPS: mapping signal id %2ld to %2u (%.2f / %.2f = %.4f)\n", from_id, to_id,
               from_freq, to_freq, scale);
#endif

        return gps_bias_from_signal(to_id, shifted_correction, continuity_indicator, fix_flag);
    }

    return Bias{signal_id, correction, continuity_indicator, fix_flag, type, false};
}

static Bias glo_bias_from_signal(long signal_id, double correction, double continuity_indicator,
                                 bool fix_flag) {
    if (signal_id >= 32) {
        return Bias::invalid();
    }

    static SPARTN_CONSTEXPR uint8_t MAPPABLE[32] = {
        1, 2, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0,
    };

    auto type = MAPPABLE[signal_id];
    if (type > 0) {
        return Bias{
            signal_id, correction, continuity_indicator, fix_flag, static_cast<uint8_t>(type - 1),
            false};
    } else {
        return Bias::invalid();
    }
}

static SPARTN_CONSTEXPR uint8_t GAL_TO_SPARTN[24] = {
    X,  // E1
    X,  // E5A
    X,  // E5B
    X,  // E6
    X,  // E5A+E5B
    0,  // E1 C No data -> L1C
    X,  // E1 A
    X,  // E1 B I/NAV OS/CS/SoL
    X,  // E1 B+C
    X,  // E1 A+B+C
    X,  // E6 C
    X,  // E6 A
    X,  // E6 B
    X,  // E6 B+C
    X,  // E6 A+B+C
    X,  // E5B I
    2,  // E5B Q -> L7Q
    X,  // E5B I+Q
    X,  // E5(A+B) I
    X,  // E5(A+B) Q
    X,  // E5(A+B) I+Q
    X,  // E5A I
    1,  // E5A Q -> L5Q
    X,  // E5A I+Q
};

static SPARTN_CONSTEXPR uint8_t GAL_MAPPING[32] = {
    X,   // E1
    X,   // E5A
    X,   // E5B
    X,   // E6
    X,   // E5A+E5B
    X,   // E1 C No data
    X,   // E1 A
    X,   // E1 B I/NAV OS/CS/SoL
    5,   // E1 B+C
    X,   // E1 A+B+C
    X,   // E6 C
    X,   // E6 A
    X,   // E6 B
    X,   // E6 B+C
    X,   // E6 A+B+C
    X,   // E5B I
    X,   // E5B Q
    X,   // E5B I+Q
    X,   // E5(A+B) I
    X,   // E5(A+B) Q
    16,  // E5(A+B) I+Q
    X,   // E5A I
    X,   // E5A Q
    X,   // E5A I+Q
};

#define GAL_E1 1575.420
#define GAL_E6 1278.750
#define GAL_E5 1191.795
#define GAL_E5a 1176.450
#define GAL_E5b 1207.140
static SPARTN_CONSTEXPR double GAL_FREQ[24] = {
    GAL_E1,   // E1
    GAL_E5a,  // E5A
    GAL_E5b,  // E5B
    GAL_E6,   // E6
    0.0,      // E5A+E5B
    GAL_E1,   // E1 C No data
    GAL_E1,   // E1 A
    GAL_E1,   // E1 B I/NAV OS/CS/SoL
    GAL_E1,   // E1 B+C
    GAL_E1,   // E1 A+B+C
    GAL_E6,   // E6 C
    GAL_E6,   // E6 A
    GAL_E6,   // E6 B
    GAL_E6,   // E6 B+C
    GAL_E6,   // E6 A+B+C
    GAL_E5b,  // E5B I
    GAL_E5b,  // E5B Q
    GAL_E5b,  // E5B I+Q
    GAL_E5,   // E5(A+B) I
    GAL_E5,   // E5(A+B) Q
    GAL_E5,   // E5(A+B) I+Q
    GAL_E5a,  // E5A I
    GAL_E5a,  // E5A Q
    GAL_E5a,  // E5A I+Q
};

static Bias gal_bias_from_signal(long signal_id, double correction, double continuity_indicator,
                                 bool fix_flag) {
    if (signal_id >= 24) {
        return Bias::invalid();
    }

    auto type = GAL_TO_SPARTN[signal_id];
    if (type == X) {
        auto from_id = signal_id;
        auto to_id   = GAL_MAPPING[from_id];
        if (to_id == X) {
            return Bias::invalid();
        }

        auto from_freq          = GAL_FREQ[from_id];
        auto to_freq            = GAL_FREQ[to_id];
        auto scale              = from_freq / to_freq;
        auto shifted_correction = correction * scale;
#ifdef SPARTN_DEBUG_PRINT
        printf("  GAL: mapping signal id %2ld to %2u (%.2f / %.2f = %.4f)\n", from_id, to_id,
               from_freq, to_freq, scale);
#endif

        return gal_bias_from_signal(to_id, shifted_correction, continuity_indicator, fix_flag);
    }

    return Bias{signal_id, correction, continuity_indicator, fix_flag, type, false};
}

#undef X

static bool phase_bias_fix_flag(const SSR_PhaseBiasSignalElement_r16& signal) {
    if (!signal.phaseBiasIntegerIndicator_r16) {
        // TODO(ewasjon): What should we do here if the fix flag information is missing? The
        // original SPARTN implementation used a default value of 'true'.
        return true;
    }

    switch (*signal.phaseBiasIntegerIndicator_r16) {
    case 0: return true;
    case 1: return true;
    case 2: return false;
    default: return false;
    }
}

template <typename BiasToSignal>
static std::map<uint8_t, Bias> phase_biases(const SSR_PhaseBiasSatElement_r16& satellite,
                                            BiasToSignal*                      bias_to_signal) {
    std::map<uint8_t, Bias> biases_by_type;

    auto& list = satellite.ssr_PhaseBiasSignalList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto signal_id  = decode::signal_id(element->signal_and_tracking_mode_ID_r16);
        auto correction = decode::phaseBias_r16(element->phaseBias_r16);
        auto continuity_indicator =
            320.0;  // TODO(ewasjon): [low-priority] Compute the continuity indicator.
        auto fix_flag = phase_bias_fix_flag(*element);

        auto bias = (*bias_to_signal)(signal_id, correction, continuity_indicator, fix_flag);
        if (bias.signal_id == -1) {
#ifdef SPARTN_DEBUG_PRINT
            printf("  unsupported bias for signal id %2ld\n", signal_id);
#endif
            continue;
        }

        if (biases_by_type.count(bias.type) == 0) {
#ifdef SPARTN_DEBUG_PRINT
            printf("  adding bias type %u (id=%ld)\n", bias.type, bias.signal_id);
#endif
            biases_by_type[bias.type] = bias;
        } else if (!bias.mapped && biases_by_type[bias.type].mapped) {
#ifdef SPARTN_DEBUG_PRINT
            // If the bias is mapped and the new bias is not mapped, then we want to prioritize
            // the "original" bias.
            printf("  replacing bias type %u (id=%ld) with %u (id=%ld)\n",
                   biases_by_type[bias.type].type, biases_by_type[bias.type].signal_id, bias.type,
                   bias.signal_id);
#endif
            biases_by_type[bias.type] = bias;
        } else {
#ifdef SPARTN_DEBUG_PRINT
            printf("  duplicate bias type %u\n", bias.type);
#endif
        }
    }

    return biases_by_type;
}

template <typename BiasToSignal>
static std::map<uint8_t, Bias> code_biases(const SSR_CodeBiasSatElement_r15& satellite,
                                           BiasToSignal*                     bias_to_signal) {
    std::map<uint8_t, Bias> biases_by_type;

    auto& list = satellite.ssr_CodeBiasSignalList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto signal_id  = decode::signal_id(element->signal_and_tracking_mode_ID_r15);
        auto correction = decode::codeBias_r15(element->codeBias_r15);

        auto bias = (*bias_to_signal)(signal_id, correction, 0.0, false);
        if (bias.signal_id == -1) {
#ifdef SPARTN_DEBUG_PRINT
            printf("  unsupported bias for signal id %2ld\n", signal_id);
#endif
            continue;
        }

        if (biases_by_type.count(bias.type) == 0) {
#ifdef SPARTN_DEBUG_PRINT
            printf("  adding bias type %u (id=%ld)\n", bias.type, bias.signal_id);
#endif
            biases_by_type[bias.type] = bias;
        } else if (!bias.mapped && biases_by_type[bias.type].mapped) {
#ifdef SPARTN_DEBUG_PRINT
            // If the bias is mapped and the new bias is not mapped, then we want to prioritize
            // the "original" bias.
            printf("  replacing bias type %u (id=%ld) with %u (id=%ld)\n",
                   biases_by_type[bias.type].type, biases_by_type[bias.type].signal_id, bias.type,
                   bias.signal_id);
#endif
            biases_by_type[bias.type] = bias;
        } else {
#ifdef SPARTN_DEBUG_PRINT
            printf("  duplicate bias type %u\n", bias.type);
#endif
        }
    }

    return biases_by_type;
}

static void generate_gps_bias_block(MessageBuilder&                    builder,
                                    const SSR_CodeBiasSatElement_r15*  code_bias,
                                    const SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) {
        builder.sf025_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(*phase_bias, &gps_bias_from_signal);

        builder.sf025(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf023(kvp.second.fix_flag);
            builder.sf015(kvp.second.continuity_indicator);
            builder.sf020(kvp.second.correction);
        }
    }

    if (!code_bias) {
        builder.sf027_raw(false, 0);
    } else {
        auto types_of_biases = code_biases(*code_bias, &gps_bias_from_signal);

        builder.sf027(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

static void generate_glo_bias_block(MessageBuilder&                    builder,
                                    const SSR_CodeBiasSatElement_r15*  code_bias,
                                    const SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) {
        builder.sf026_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(*phase_bias, &glo_bias_from_signal);

        builder.sf026(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf023(kvp.second.fix_flag);
            builder.sf015(kvp.second.continuity_indicator);
            builder.sf020(kvp.second.correction);
        }
    }

    if (!code_bias) {
        builder.sf028_raw(false, 0);
    } else {
        auto types_of_biases = code_biases(*code_bias, &glo_bias_from_signal);

        builder.sf028(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

static void generate_gal_bias_block(MessageBuilder&                    builder,
                                    const SSR_CodeBiasSatElement_r15*  code_bias,
                                    const SSR_PhaseBiasSatElement_r16* phase_bias) {
    if (!phase_bias) {
        builder.sf102_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(*phase_bias, &gal_bias_from_signal);

        builder.sf102(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf023(kvp.second.fix_flag);
            builder.sf015(kvp.second.continuity_indicator);
            builder.sf020(kvp.second.correction);
        }
    }

    if (!code_bias) {
        builder.sf105_raw(false, 0);
    } else {
        auto types_of_biases = code_biases(*code_bias, &gal_bias_from_signal);

        builder.sf105(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

void Generator::generate_ocb(long iod) {
    auto ocb_data = mCorrectionData->ocb(iod);
    if (!ocb_data) return;

    std::vector<OcbCorrections*> messages;
    for (auto& kvp : ocb_data->mKeyedCorrections) {
        if (!mGpsSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_gps) continue;
        if (!mGlonassSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_glonass) continue;
        if (!mGalileoSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_galileo) continue;
        if (!mBeidouSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_bds) continue;

        messages.push_back(&kvp.second);
    }

    std::sort(messages.begin(), messages.end(),
              [](const OcbCorrections* a, const OcbCorrections* b) {
                  return subtype_from_gnss_id(a->gnss_id) < subtype_from_gnss_id(b->gnss_id);
              });

    for (size_t message_id = 0; message_id < messages.size(); message_id++) {
        auto& corrections = *messages[message_id];
        auto  epoch_time  = corrections.epoch_time.rounded_seconds;
        auto  gnss_id     = corrections.gnss_id;

        auto satellites = corrections.satellites();

#ifdef SPARTN_DEBUG_PRINT
        printf("OCB: time=%u, gnss=%ld, iod=%ld\n", epoch_time, gnss_id, iod);
        for (auto& satellite : satellites) {
            printf("  satellite: %4ld  ", satellite.id);
            if (satellite.orbit) {
                printf("O");
            } else {
                printf("-");
            }
            if (satellite.clock) {
                printf("C");
            } else {
                printf("-");
            }
            if (satellite.code_bias) {
                printf("B");
            } else {
                printf("-");
            }
            if (satellite.phase_bias) {
                printf("P");
            } else {
                printf("-");
            }
            if (satellite.ura) {
                printf("U");
            } else {
                printf("-");
            }
            printf("\n");
        }
#endif

        auto eos               = (message_id + 1) == messages.size();
        auto yaw_angle_present = false;
        auto subtype           = subtype_from_gnss_id(gnss_id);

        MessageBuilder builder{0 /* OCB */, subtype, epoch_time};
        builder.sf005(iod);
        builder.sf010(eos);
        builder.sf069();
        builder.sf008(yaw_angle_present);
        builder.sf009(0 /* ITRF */);  // we assume that satellite reference datum is ITRF
        builder.ephemeris_type(gnss_id);
        builder.satellite_mask(gnss_id, satellites);

        for (auto& satellite : satellites) {
#ifdef SPARTN_DEBUG_PRINT
            printf("  SATELLITE: %4ld\n", satellite.id);
#endif

            builder.sf013(false /* false = do use this satellite */);
            builder.sf014(satellite.orbit != nullptr, satellite.clock != nullptr,
                          satellite.code_bias != nullptr || satellite.phase_bias != nullptr);
            if (mContinuityIndicator >= 0.0) {
                builder.sf022(mContinuityIndicator);
            } else {
                builder.sf015(320.0);  // TODO(ewasjon): compute the continuity indicator
            }

            if (satellite.orbit) {
                auto& orbit = *satellite.orbit;
                builder.orbit_iode(gnss_id, orbit.iod_r15, mIodeShift);

                auto radial = decode::delta_radial_r15(orbit.delta_radial_r15);
                auto along  = decode::delta_AlongTrack_r15(orbit.delta_AlongTrack_r15);
                auto cross  = decode::delta_CrossTrack_r15(orbit.delta_CrossTrack_r15);
                builder.sf020(radial);
                builder.sf020(along);
                builder.sf020(cross);
                if (yaw_angle_present) {
                    // NOTE(ewasjon): Yaw angle is assumed to be zero, 3GPP LPP inherits this
                    // assumption from CLAS specification.
                    builder.sf021(0.0 /* 0 degrees */);
                }
            }

            if (satellite.clock) {
                auto& clock = *satellite.clock;
                if (mContinuityIndicator >= 0.0) {
                    builder.sf022(mContinuityIndicator);
                } else {
                    // TODO(ewasjon): [low-priority] Compute the continuity indicator.
                    builder.sf022(320.0);
                }

                // NOTE(ewasjon): SPARTN has a single value of the clock correction term. 3GPP
                // LPP has model this as an polynomial around the middle of the ssr update rate.
                // Thus, the single value at epoch time must be computed.

                auto c0 = decode::delta_Clock_C0_r15(clock.delta_Clock_C0_r15);
                auto c1 = decode::delta_Clock_C2_r15(clock.delta_Clock_C1_r15);
                auto c2 = decode::delta_Clock_C1_r15(clock.delta_Clock_C2_r15);

                // t_0 = epochTime + (0.5 * ssrUpdateInterval)
                // TODO(ewasjon): [low-priority] Include SSR update interval.This is fine not to
                // include while we are using t=t0.
                auto t0 = corrections.epoch_time.seconds;
                // TODO(ewasjon): [low-priority] We don't have an actual time available, is it
                // possible for us to have access to that? If not, we will continue to use t=t0.
                auto t = t0;

                // delta_c = c_0 + c_1 (t - t_0) + [c_2 (t - t_0)]^2
                auto dt = t - t0;
                auto dc = c0 + c1 * dt + c2 * dt * dt;

                // NOTE(ewasjon): [REDACTED] observed that changing the sign of the clock
                // corrections (in their correction feed) improved the result. They assumed that
                // u-Blox implemented it with a flipped sign. Thus, we also need to flip the sign to
                // conform to the u-Blox implementation.
                if (mUBloxClockCorrection) {
                    dc *= -1;
                }

                builder.sf020(dc);

                if (mUraOverride >= 0) {
                    builder.sf024_raw(mUraOverride > 7 ? 7 : mUraOverride);
                } else if (satellite.ura) {
                    auto& ura   = *satellite.ura;
                    auto  value = decode::ssr_URA_r16(ura.ssr_URA_r16);
                    builder.sf024(value);
                } else {
                    builder.sf024_raw(0);
                }
            }

            if (satellite.code_bias || satellite.phase_bias) {
                switch (gnss_id) {
                case GNSS_ID__gnss_id_gps:
                    generate_gps_bias_block(builder, satellite.code_bias, satellite.phase_bias);
                    break;
                case GNSS_ID__gnss_id_glonass:
                    generate_glo_bias_block(builder, satellite.code_bias, satellite.phase_bias);
                    break;
                case GNSS_ID__gnss_id_galileo:
                    generate_gal_bias_block(builder, satellite.code_bias, satellite.phase_bias);
                    break;
                default: SPARTN_UNREACHABLE();
                }
            }
        }

        mMessages.push_back(builder.build());
    }
}

}  // namespace spartn
}  // namespace generator

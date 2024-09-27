#include "constant.hpp"
#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"
#include "time.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-URA-r16.h>
#include <SSR-ClockCorrectionList-r15.h>
#include <SSR-ClockCorrectionSatelliteElement-r15.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSatList-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-CodeBiasSignalList-r15.h>
#include <SSR-OrbitCorrectionList-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSatList-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#include <SSR-PhaseBiasSignalList-r16.h>
#include <SSR-URA-SatElement-r16.h>
#include <SSR-URA-SatList-r16.h>
#pragma GCC diagnostic pop

#include <algorithm>
#include <map>

namespace generator {
namespace spartn {

uint32_t OcbSatellite::prn() const {
    // NOTE(ewasjon): 3GPP LPP defines PRN starting at 0 instead of 1.
    return static_cast<uint32_t>(id + 1);
}

void OcbSatellite::add_correction(SSR_OrbitCorrectionSatelliteElement_r15* new_orbit) {
    if (!new_orbit) return;
    orbit = new_orbit;
}

void OcbSatellite::add_correction(SSR_ClockCorrectionSatelliteElement_r15* new_clock) {
    if (!new_clock) return;
    clock = new_clock;
}

void OcbSatellite::add_correction(SSR_CodeBiasSatElement_r15* new_code_bias) {
    if (!new_code_bias) return;
    code_bias = new_code_bias;
}

void OcbSatellite::add_correction(SSR_PhaseBiasSatElement_r16* new_phase_bias) {
    if (!new_phase_bias) return;
    phase_bias = new_phase_bias;
}

void OcbSatellite::add_correction(SSR_URA_SatElement_r16* new_ura) {
    if (!new_ura) return;
    ura = new_ura;
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
    std::sort(result.begin(), result.end(), [](OcbSatellite const& a, OcbSatellite const& b) {
        return a.id < b.id;
    });

    return result;
}

bool OcbCorrections::has_satellite(long id) const {
    if (orbit) {
        auto& list = orbit->ssr_OrbitCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;
            if (element->svID_r15.satellite_id == id) return true;
        }
    }

    if (clock) {
        auto& list = clock->ssr_ClockCorrectionList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;
            if (element->svID_r15.satellite_id == id) return true;
        }
    }

    if (code_bias) {
        auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;
            if (element->svID_r15.satellite_id == id) return true;
        }
    }

    if (phase_bias) {
        auto& list = phase_bias->ssr_PhaseBiasSatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;
            if (element->svID_r16.satellite_id == id) return true;
        }
    }

    if (ura) {
        auto& list = ura->ssr_URA_SatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;
            if (element->svID_r16.satellite_id == id) return true;
        }
    }

    return false;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15* orbit) {
    if (!orbit) return;
    auto  iod = static_cast<uint16_t>(orbit->iod_ssr_r15);
    auto& ocb = mOcbData[iod];

    // TODO(ewasjon): [low-priority] Filter based on satellite reference datum.
    auto epoch_time = spartn_time_from(orbit->epochTime_r15);
    auto key        = OcbKey{gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections                 = ocb.mKeyedCorrections[key];
    corrections.gnss_id               = gnss_id;
    corrections.iod                   = iod;
    corrections.epoch_time            = epoch_time;
    corrections.orbit                 = orbit;
    corrections.orbit_update_interval = decode::ssrUpdateInterval_r15(orbit->ssrUpdateInterval_r15);
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15* clock) {
    if (!clock) return;
    auto  iod = static_cast<uint16_t>(clock->iod_ssr_r15);
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(clock->epochTime_r15);
    auto key        = OcbKey{gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections                 = ocb.mKeyedCorrections[key];
    corrections.gnss_id               = gnss_id;
    corrections.iod                   = iod;
    corrections.epoch_time            = epoch_time;
    corrections.clock                 = clock;
    corrections.clock_update_interval = decode::ssrUpdateInterval_r15(clock->ssrUpdateInterval_r15);
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15* code_bias) {
    if (!code_bias) return;
    auto  iod = static_cast<uint16_t>(code_bias->iod_ssr_r15);
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(code_bias->epochTime_r15);
    auto key        = OcbKey{gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.code_bias  = code_bias;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16* phase_bias) {
    if (!phase_bias) return;
    auto  iod = static_cast<uint16_t>(phase_bias->iod_ssr_r16);
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(phase_bias->epochTime_r16);
    auto key        = OcbKey{gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections      = ocb.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.epoch_time = epoch_time;
    corrections.phase_bias = phase_bias;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_URA_r16* ura) {
    if (!ura) return;
    auto  iod = static_cast<uint16_t>(ura->iod_ssr_r16);
    auto& ocb = mOcbData[iod];

    auto epoch_time = spartn_time_from(ura->epochTime_r16);
    auto key        = OcbKey{gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections   = ocb.mKeyedCorrections[key];
    corrections.gnss_id = gnss_id;
    corrections.iod     = iod;
    // URA epoch time may update slower and using it could override the epoch time for the other
    // data in OCB. Therefore, we don't set the epoch time here, except if the corrections are
    // grouped by epoch time.
    if (mGroupByEpochTime) {
        corrections.epoch_time = epoch_time;
    }
    corrections.ura = ura;
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

static Bias bias_from_signal(SystemMapping const* mapping, bool is_phase, long from_id,
                             double correction, double continuity_indicator, bool fix_flag,
                             bool translate, bool correction_shift) {
    if (from_id >= mapping->signal_count) {
        return Bias::invalid();
    }

    auto type = mapping->to_spartn[from_id];
    if (type == INVALID_MAPPING) {
        if (!translate) {
            return Bias::invalid();
        }

        auto to_id = mapping->mapping[from_id];
        if (to_id == INVALID_MAPPING) {
            return Bias::invalid();
        } else if (to_id >= mapping->signal_count) {
            return Bias::invalid();
        }

        auto from_freq          = mapping->freq[from_id];
        auto to_freq            = mapping->freq[to_id];
        auto scale              = from_freq / to_freq;
        auto shifted_correction = correction * scale;
        if (!correction_shift) {
            shifted_correction = correction;
        }

#ifdef SPARTN_DEBUG_PRINT
        printf("        from: %2ld '%-16s' %7.2f\n", from_id, mapping->signal_name(from_id),
               correction);
        printf("          to: %2hhu '%-16s' %7.2f\n", to_id, mapping->signal_name(to_id),
               shifted_correction);
#endif

        auto new_bias =
            bias_from_signal(mapping, is_phase, to_id, shifted_correction, continuity_indicator,
                             fix_flag, translate, correction_shift);
        new_bias.mapped = true;
        return new_bias;
    }

    return Bias{from_id, correction, continuity_indicator, fix_flag, type, false};
}

static bool phase_bias_fix_flag_from_value(long value) {
    switch (value) {
    case 0: return true;
    case 1: return true;
    case 2: return false;
    default: return false;
    }
}

static bool phase_bias_fix_flag(SSR_PhaseBiasSignalElement_r16 const& signal) {
    // NOTE(ewasjon): If the phaseBiasIntegerIndicator field is not present then it is
    // interpreted as having Value 0 (Undifferenced Integer).
    long value = 0;
    if (signal.phaseBiasIntegerIndicator_r16) {
        value = *signal.phaseBiasIntegerIndicator_r16;
    }

    return phase_bias_fix_flag_from_value(value);
}

template <typename BiasToSignal>
static std::map<uint8_t, Bias>
phase_biases(SystemMapping const* mapping, SSR_PhaseBiasSatElement_r16 const& satellite,
             BiasToSignal* bias_to_signal, bool ignore_l2l, bool translate, bool correction_shift) {
    std::map<uint8_t, Bias> biases_by_type;

#if defined(SPARTN_DEBUG_PRINT)
    printf("  PHASE BIAS:\n");
#endif
    auto& list = satellite.ssr_PhaseBiasSignalList_r16.list;

    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto signal_id  = decode::signal_id(element->signal_and_tracking_mode_ID_r16);
        auto correction = decode::phaseBias_r16(element->phaseBias_r16);
        auto continuity_indicator =
            320.0;  // TODO(ewasjon): [low-priority] Compute the continuity indicator.
        auto fix_flag = phase_bias_fix_flag(*element);

        auto bias = (*bias_to_signal)(mapping, true, signal_id, correction, continuity_indicator,
                                      fix_flag, translate, correction_shift);
        if (bias.signal_id == -1) {
#if defined(SPARTN_DEBUG_PRINT) && SPARTN_DEBUG_PRINT > 1
            printf("    ?         %2ld '%-16s' %7.2f%s\n", signal_id,
                   mapping->signal_name(signal_id), correction, fix_flag ? " (fix)" : " (unfixed)");
#endif
            continue;
        }

        if (mapping->gnss_id == GNSS_ID__gnss_id_gps && bias.type == 2 && ignore_l2l) {
#ifdef SPARTN_DEBUG_PRINT
            printf("    -%s (%u)  %2ld '%-16s' %7.2f%s\n",
                   bias_type_name(mapping->gnss_id, true, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction,
                   fix_flag ? " (fix)" : " (unfixed)");
#endif
            continue;
        }

        if (biases_by_type.count(bias.type) == 0) {
#ifdef SPARTN_DEBUG_PRINT
            printf("    +%s (%u)  %2ld '%-16s' %7.2f%s\n",
                   bias_type_name(mapping->gnss_id, true, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction,
                   fix_flag ? " (fix)" : " (unfixed)");
#endif
            biases_by_type[bias.type] = bias;
        } else if (!bias.mapped && biases_by_type[bias.type].mapped) {
#ifdef SPARTN_DEBUG_PRINT
            // If the bias is mapped and the new bias is not mapped, then we want to prioritize
            // the "original" bias.
            printf("    =%s (%u)  %2ld '%-16s' %7.2f%s\n",
                   bias_type_name(mapping->gnss_id, true, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction,
                   fix_flag ? " (fix)" : " (unfixed)");
#endif
            biases_by_type[bias.type] = bias;
        } else {
#ifdef SPARTN_DEBUG_PRINT
            printf("    !%s (%u)  %2ld '%-16s' %7.2f%s\n",
                   bias_type_name(mapping->gnss_id, true, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction,
                   fix_flag ? " (fix)" : " (unfixed)");
#endif
        }
    }

    return biases_by_type;
}

template <typename BiasToSignal>
static std::map<uint8_t, Bias>
code_biases(SystemMapping const* mapping, SSR_CodeBiasSatElement_r15 const& satellite,
            BiasToSignal* bias_to_signal, bool ignore_l2l, bool translate, bool correction_shift) {
    std::map<uint8_t, Bias> biases_by_type;

#if defined(SPARTN_DEBUG_PRINT)
    printf("  CODE BIAS:\n");
#endif
    auto& list = satellite.ssr_CodeBiasSignalList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto signal_id  = decode::signal_id(element->signal_and_tracking_mode_ID_r15);
        auto correction = decode::codeBias_r15(element->codeBias_r15);

        auto bias = (*bias_to_signal)(mapping, false, signal_id, correction, 0.0, false, translate,
                                      correction_shift);
        if (bias.signal_id == -1) {
#if defined(SPARTN_DEBUG_PRINT) && SPARTN_DEBUG_PRINT > 1
            printf("    ?         %2ld '%-16s' %7.2f\n", signal_id, mapping->signal_name(signal_id),
                   correction);
#endif
            continue;
        }

        if (mapping->gnss_id == GNSS_ID__gnss_id_gps && bias.type == 2 && ignore_l2l) {
#ifdef SPARTN_DEBUG_PRINT
            printf("    -%s (%u)  %2ld '%-16s' %7.2f\n",
                   bias_type_name(mapping->gnss_id, true, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction);
#endif
            continue;
        }

        if (biases_by_type.count(bias.type) == 0) {
#ifdef SPARTN_DEBUG_PRINT
            printf("    +%s (%u)  %2ld '%-16s' %7.2f\n",
                   bias_type_name(mapping->gnss_id, false, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction);
#endif
            biases_by_type[bias.type] = bias;
        } else if (!bias.mapped && biases_by_type[bias.type].mapped) {
#ifdef SPARTN_DEBUG_PRINT
            // If the bias is mapped and the new bias is not mapped, then we want to prioritize
            // the "original" bias.
            printf("    =%s (%u)  %2ld '%-16s' %7.2f\n",
                   bias_type_name(mapping->gnss_id, false, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction);
#endif
            biases_by_type[bias.type] = bias;
        } else {
#ifdef SPARTN_DEBUG_PRINT
            printf("    !%s (%u)  %2ld '%-16s' %7.2f\n",
                   bias_type_name(mapping->gnss_id, false, bias.type), bias.type, bias.signal_id,
                   mapping->signal_name(bias.signal_id), bias.correction);
#endif
        }
    }

    return biases_by_type;
}

static void generate_gps_bias_block(MessageBuilder& builder, SystemMapping const* mapping,
                                    SSR_CodeBiasSatElement_r15 const*  code_bias,
                                    SSR_PhaseBiasSatElement_r16 const* phase_bias, bool ignore_l2l,
                                    bool code_bias_translate, bool code_bias_no_correction_shift,
                                    bool phase_bias_translate,
                                    bool phase_bias_no_correction_shift) {
    if (!phase_bias) {
        builder.sf025_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(mapping, *phase_bias, &bias_from_signal, ignore_l2l,
                                            phase_bias_translate, phase_bias_no_correction_shift);

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
        auto types_of_biases = code_biases(mapping, *code_bias, &bias_from_signal, ignore_l2l,
                                           code_bias_translate, code_bias_no_correction_shift);

        builder.sf027(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

static void generate_glo_bias_block(MessageBuilder& builder, SystemMapping const* mapping,
                                    SSR_CodeBiasSatElement_r15 const*  code_bias,
                                    SSR_PhaseBiasSatElement_r16 const* phase_bias, bool ignore_l2l,
                                    bool code_bias_translate, bool code_bias_correction_shift,
                                    bool phase_bias_translate, bool phase_bias_correction_shift) {
    if (!phase_bias) {
        builder.sf026_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(mapping, *phase_bias, &bias_from_signal, ignore_l2l,
                                            phase_bias_translate, phase_bias_correction_shift);

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
        auto types_of_biases = code_biases(mapping, *code_bias, &bias_from_signal, ignore_l2l,
                                           code_bias_translate, code_bias_correction_shift);

        builder.sf028(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

static void generate_gal_bias_block(MessageBuilder& builder, SystemMapping const* mapping,
                                    SSR_CodeBiasSatElement_r15 const*  code_bias,
                                    SSR_PhaseBiasSatElement_r16 const* phase_bias, bool ignore_l2l,
                                    bool code_bias_translate, bool code_bias_correction_shift,
                                    bool phase_bias_translate, bool phase_bias_correction_shift) {
    if (!phase_bias) {
        builder.sf102_raw(false, 0);
    } else {
        auto types_of_biases = phase_biases(mapping, *phase_bias, &bias_from_signal, ignore_l2l,
                                            phase_bias_translate, phase_bias_correction_shift);

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
        auto types_of_biases = code_biases(mapping, *code_bias, &bias_from_signal, ignore_l2l,
                                           code_bias_translate, code_bias_correction_shift);

        builder.sf105(&types_of_biases);
        for (auto& kvp : types_of_biases) {
            builder.sf029(kvp.second.correction);
        }
    }
}

void Generator::generate_ocb(uint16_t iod) {
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
              [](OcbCorrections const* a, OcbCorrections const* b) {
                  return subtype_from_gnss_id(a->gnss_id) < subtype_from_gnss_id(b->gnss_id);
              });

    for (size_t message_id = 0; message_id < messages.size(); message_id++) {
        auto& corrections = *messages[message_id];
        auto  epoch_time  = corrections.epoch_time.rounded_seconds;
        auto  gnss_id     = corrections.gnss_id;

        auto satellites = corrections.satellites();

#ifdef SPARTN_DEBUG_PRINT
        printf("OCB: time=%u, gnss=%ld, iod=%hu\n", epoch_time, gnss_id, iod);
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

        auto siou = iod;
        if (mIncreasingSiou) {
            siou = mSiouIndex;
        }

        MessageBuilder builder{0 /* OCB */, subtype, epoch_time};
        builder.sf005(siou);
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
                builder.sf015(mContinuityIndicator);
            } else {
                builder.sf015(320.0);  // TODO(ewasjon): compute the continuity indicator
            }

            if (satellite.orbit) {
                auto& orbit = *satellite.orbit;
                builder.orbit_iode(gnss_id, orbit.iod_r15, mIodeShift);

                auto radial = decode::delta_radial_r15(orbit.delta_radial_r15);
                auto along  = decode::delta_AlongTrack_r15(orbit.delta_AlongTrack_r15);
                auto cross  = decode::delta_CrossTrack_r15(orbit.delta_CrossTrack_r15);

                if(mFlipOrbitCorrection) {
                    radial *= -1;
                    cross *= -1;
                }

                builder.sf020(radial);
                builder.sf020(along);
                builder.sf020(cross);

#ifdef SPARTN_DEBUG_PRINT
                printf("    ORBIT UPDATE INTERVAL: %f\n", corrections.orbit_update_interval);
                printf("    RADIAL: %f\n", radial);
                printf("    ALONG: %f\n", along);
                printf("    CROSS: %f\n", cross);
                if (orbit.dot_delta_radial_r15)
                    printf("    DOT RADIAL: %ld\n", *orbit.dot_delta_radial_r15);
                if (orbit.dot_delta_AlongTrack_r15)
                    printf("    DOT ALONG: %ld\n", *orbit.dot_delta_AlongTrack_r15);
                if (orbit.dot_delta_CrossTrack_r15)
                    printf("    DOT CROSS: %ld\n", *orbit.dot_delta_CrossTrack_r15);
#endif

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
                auto c1 = decode::delta_Clock_C1_r15(clock.delta_Clock_C1_r15);
                auto c2 = decode::delta_Clock_C2_r15(clock.delta_Clock_C2_r15);

#ifdef SPARTN_DEBUG_PRINT
                printf("    CLOCK UPDATE INTERVAL: %f\n", corrections.clock_update_interval);
                printf("    C0: %f\n", c0);
                printf("    C1: %f\n", c1);
                printf("    C2: %f\n", c2);
#endif

                // t_0 = epochTime + (0.5 * ssrUpdateInterval)
                // TODO(ewasjon): [low-priority] Include SSR update interval. This is fine not to
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

#ifdef SPARTN_DEBUG_PRINT
                printf("    clock: %f%s\n", dc, mUBloxClockCorrection ? " [u-blox]" : "");
#endif

                builder.sf020(dc);

                if (mUraOverride >= 0) {
                    uint8_t ura_value = mUraOverride > 7 ? 7 : static_cast<uint8_t>(mUraOverride);
                    builder.sf024_raw(ura_value);
#ifdef SPARTN_DEBUG_PRINT
                    printf("    sf024: %d (%u) [override]\n", mUraOverride, ura_value);
#endif
                } else if (satellite.ura) {
                    auto& ura     = *satellite.ura;
                    auto  quality = decode::ssr_URA_r16(ura.ssr_URA_r16);
                    if (quality.invalid) {
                        uint8_t ura_value =
                            mUraDefault < 0 ? 0 : (mUraDefault > 7 ? 7 : mUraDefault);
                        builder.sf024_raw(ura_value);
#ifdef SPARTN_DEBUG_PRINT
                        printf("    sf024: %d (%u) [default/invalid]\n", mUraDefault, ura_value);
#endif
                    } else {
                        auto ura_value = builder.sf024(quality.value);
#ifdef SPARTN_DEBUG_PRINT
                        printf("    sf024: %f (%u)\n", quality.value, ura_value);
#endif
                    }
                } else {
                    uint8_t ura_value = mUraDefault < 0 ? 0 : (mUraDefault > 7 ? 7 : mUraDefault);
                    builder.sf024_raw(ura_value);
#ifdef SPARTN_DEBUG_PRINT
                    printf("    sf024: %d (%u) [default/missing]\n", mUraDefault, ura_value);
#endif
                }
            }

            if (satellite.code_bias || satellite.phase_bias) {
                switch (gnss_id) {
                case GNSS_ID__gnss_id_gps:
                    generate_gps_bias_block(builder, &GPS_SM, satellite.code_bias,
                                            satellite.phase_bias, mIgnoreL2L, mCodeBiasTranslate,
                                            mCodeBiasCorrectionShift, mPhaseBiasTranslate,
                                            mPhaseBiasCorrectionShift);
                    break;
                case GNSS_ID__gnss_id_glonass:
                    generate_glo_bias_block(builder, &GLO_SM, satellite.code_bias,
                                            satellite.phase_bias, mIgnoreL2L, mCodeBiasTranslate,
                                            mCodeBiasCorrectionShift, mPhaseBiasTranslate,
                                            mPhaseBiasCorrectionShift);
                    break;
                case GNSS_ID__gnss_id_galileo:
                    generate_gal_bias_block(builder, &GAL_SM, satellite.code_bias,
                                            satellite.phase_bias, mIgnoreL2L, mCodeBiasTranslate,
                                            mCodeBiasCorrectionShift, mPhaseBiasTranslate,
                                            mPhaseBiasCorrectionShift);
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

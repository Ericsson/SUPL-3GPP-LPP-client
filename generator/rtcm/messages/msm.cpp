#include "msm.hpp"
#include "encoder.hpp"
#include "helper.hpp"

#include <cmath>
#include <cstdio>
#include <inttypes.h>

using namespace generator::rtcm;

static uint32_t msm_message_id(uint32_t msm, GenericGnssId gnss) {
    switch (gnss) {
    case GenericGnssId::GPS: return 1070 + msm;
    case GenericGnssId::GLONASS: return 1080 + msm;
    case GenericGnssId::GALILEO: return 1090 + msm;
    case GenericGnssId::BEIDOU: return 1120 + msm;
    }
    return 0;
}

static void df397(Encoder& encoder, const Satellite& satellite) {
    if (satellite.integer_ms.valid && satellite.integer_ms.value >= 0 &&
        satellite.integer_ms.value <= 254) {
        encoder.u8(8, static_cast<uint8_t>(satellite.integer_ms.value));
    } else {
        encoder.u8(8, 0xFF);
    }
}

static void df398(Encoder& encoder, const Satellite& satellite) {
    auto value = ROUND(satellite.rough_range.value * RTCM_P2_10);
    if (satellite.rough_range.valid && value >= 0 && value <= 1024) {
        encoder.u16(10, static_cast<uint32_t>(value));
    } else {
        // TODO: How to we report an invalid if the DF doesn't have an invalid bit pattern?
        encoder.u16(10, 0x3FF);
    }
}

static void df399(Encoder& encoder, const Satellite& satellite) {
    auto value = static_cast<int64_t>(ROUND(satellite.rough_phase_range_rate.value));
    if (satellite.rough_phase_range_rate.valid && value >= -8191 && value <= 8191) {
        encoder.i16(14, static_cast<int16_t>(value));
    } else {
        encoder.u16(14, 0x2000);
    }
}

static void df419(Encoder& encoder, const Satellite& satellite) {
    auto channel = satellite.frequency_channel.value + 7;
    if (satellite.frequency_channel.valid && channel >= 0 && channel <= 14) {
        encoder.u8(4, static_cast<uint8_t>(channel));
    } else {
        encoder.u8(4, 0xF);
    }
}

static void df_ext(Encoder& encoder, const Satellite& satellite) {
    if (satellite.id.gnss() == SatelliteId::Gnss::GLONASS) {
        df419(encoder, satellite);
    } else {
        encoder.u8(4, 0xF);
    }
}

static void df400(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.fine_pseudo_range.value * RTCM_P2_24));
    if (signal.fine_phase_range.valid) {
        encoder.i16(15, static_cast<int16_t>(value));
    } else {
        encoder.i16(15, 0x4000);
    }
}

static void df401(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.fine_phase_range.value * RTCM_P2_29));
    if (signal.fine_pseudo_range.valid) {
        encoder.i32(22, static_cast<int32_t>(value));
    } else {
        encoder.i32(22, 0x200000);
    }
}

static void df402(Encoder& encoder, const Signal& signal) {
    auto value = to_msm_lock(signal.lock_time.value);
    if (signal.lock_time.valid && value >= 0 && value <= 15) {
        encoder.u8(4, static_cast<uint8_t>(value));
    } else {
        encoder.u8(4, 0);
    }
}

static void df403(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.carrier_to_noise_ratio.value));
    if (signal.carrier_to_noise_ratio.valid && value >= 1 && value <= 63) {
        encoder.u8(6, static_cast<uint8_t>(value));
    } else {
        encoder.u8(6, 0);
    }
}

static void df404(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.fine_phase_range_rate.value / 0.0001));
    if (signal.fine_phase_range_rate.valid && value > -16384 && value <= 16383) {
        encoder.i16(15, static_cast<int16_t>(value));
    } else {
        encoder.i16(15, 0x4000);
    }
}

static void df405(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.fine_pseudo_range.value * RTCM_P2_29));
    if (signal.fine_phase_range.valid) {
        encoder.i32(20, static_cast<int32_t>(value));
    } else {
        encoder.i32(20, 0x80000);
    }
}

static void df406(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.fine_phase_range.value * RTCM_P2_31));
    if (signal.fine_pseudo_range.valid) {
        encoder.i32(24, static_cast<int32_t>(value));
    } else {
        encoder.i32(24, 0x800000);
    }
}

static void df407(Encoder& encoder, const Signal& signal) {
    auto value = to_msm_lock_ex(signal.lock_time.value);
    if (signal.lock_time.valid && value >= 0 && value <= 704) {
        encoder.u16(10, static_cast<uint16_t>(value));
    } else {
        encoder.u16(10, 0);
    }
}

static void df408(Encoder& encoder, const Signal& signal) {
    auto value = static_cast<int64_t>(ROUND(signal.carrier_to_noise_ratio.value / 0.0625));
    if (signal.carrier_to_noise_ratio.valid && value >= 1 && value <= 1023) {
        encoder.u16(10, static_cast<uint16_t>(value));
    } else {
        encoder.u16(10, 0);
    }
}

static void df420(Encoder& encoder, const Signal& signal) {
    encoder.b(signal.half_cycle_ambiguity);
}

//
// Satellites
//

#define FOR_EACH_SAT(X)                                                                            \
    for (auto& satellite : satellites) {                                                           \
        X(encoder, *satellite);                                                                    \
    }

static void generate_msm4_satellites(Encoder&                             encoder,
                                     const std::vector<const Satellite*>& satellites) {
    FOR_EACH_SAT(df397);
    FOR_EACH_SAT(df398);
}

static void generate_msm5_satellites(Encoder&                             encoder,
                                     const std::vector<const Satellite*>& satellites) {
    FOR_EACH_SAT(df397);
    FOR_EACH_SAT(df_ext);
    FOR_EACH_SAT(df398);
    FOR_EACH_SAT(df399);
}

static void generate_msm6_satellites(Encoder&                             encoder,
                                     const std::vector<const Satellite*>& satellites) {
    FOR_EACH_SAT(df397);
    FOR_EACH_SAT(df398);
}

static void generate_msm7_satellites(Encoder&                             encoder,
                                     const std::vector<const Satellite*>& satellites) {
    FOR_EACH_SAT(df397);
    FOR_EACH_SAT(df_ext);
    FOR_EACH_SAT(df398);
    FOR_EACH_SAT(df399);
}

static void generate_msm_satellites(uint32_t msm, Encoder& encoder,
                                    const std::vector<const Satellite*>& satellites) {
    switch (msm) {
    case 4: generate_msm4_satellites(encoder, satellites); break;
    case 5: generate_msm5_satellites(encoder, satellites); break;
    case 6: generate_msm6_satellites(encoder, satellites); break;
    case 7: generate_msm7_satellites(encoder, satellites); break;
    }
}

//
// Signals
//

#define FOR_EACH_SIG(X)                                                                            \
    for (auto& signal : signals) {                                                                 \
        X(encoder, *signal);                                                                       \
    }

static void generate_msm4_signals(Encoder& encoder, const std::vector<const Signal*>& signals) {
    FOR_EACH_SIG(df400);
    FOR_EACH_SIG(df401);
    FOR_EACH_SIG(df402);
    FOR_EACH_SIG(df420);
    FOR_EACH_SIG(df403);
}

static void generate_msm5_signals(Encoder& encoder, const std::vector<const Signal*>& signals) {
    FOR_EACH_SIG(df400);
    FOR_EACH_SIG(df401);
    FOR_EACH_SIG(df402);
    FOR_EACH_SIG(df420);
    FOR_EACH_SIG(df403);
    FOR_EACH_SIG(df404);
}

static void generate_msm6_signals(Encoder& encoder, const std::vector<const Signal*>& signals) {
    FOR_EACH_SIG(df405);
    FOR_EACH_SIG(df406);
    FOR_EACH_SIG(df407);
    FOR_EACH_SIG(df420);
    FOR_EACH_SIG(df408);
}

static void generate_msm7_signals(Encoder& encoder, const std::vector<const Signal*>& signals) {
    FOR_EACH_SIG(df405);
    FOR_EACH_SIG(df406);
    FOR_EACH_SIG(df407);
    FOR_EACH_SIG(df420);
    FOR_EACH_SIG(df408);
    FOR_EACH_SIG(df404);
}

static void generate_msm_signals(uint32_t msm, Encoder& encoder,
                                 const std::vector<const Signal*>& signals) {
    switch (msm) {
    case 4: generate_msm4_signals(encoder, signals); break;
    case 5: generate_msm5_signals(encoder, signals); break;
    case 6: generate_msm6_signals(encoder, signals); break;
    case 7: generate_msm7_signals(encoder, signals); break;
    }
}

//
// Header
//

extern generator::rtcm::Message generate_msm(uint32_t msm, bool last_msm, GenericGnssId gnss,
                                             const CommonObservationInfo& common,
                                             const Observations&          observations) {
    auto message_id = msm_message_id(msm, gnss);

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, common.reference_station_id);
    epoch_time(encoder, observations.time, gnss);
    encoder.b(!last_msm /* multiple message bit */);
    encoder.u8(3, 0u /* iod */);
    encoder.reserve(7);
    encoder.u8(2, static_cast<uint8_t>(common.clock_steering));
    encoder.u8(2, static_cast<uint8_t>(common.external_clock));
    encoder.u8(1, static_cast<uint8_t>(common.smooth_indicator));
    encoder.u8(3, static_cast<uint8_t>(common.smooth_interval));

    std::vector<const Satellite*> satellites;
    std::vector<const Signal*>    signals;

    for (auto& satellite : observations.satellites) {
        auto id = satellite.id.as_msm();
        if (id.valid && id.value >= 1 && id.value <= 64) {
            satellites.push_back(&satellite);
        }
    }

    for (auto& signal : observations.signals) {
        auto signal_id = signal.id.as_msm();
        if (!(signal_id.valid && signal_id.value >= 1 && signal_id.value <= 32)) continue;

        auto satellite_id = signal.satellite.as_msm();
        if (!(satellite_id.valid && satellite_id.value >= 1 && satellite_id.value <= 64)) continue;
        signals.push_back(&signal);
    }

    uint64_t satellite_ids = 0;
    for (auto satellite : satellites) {
        auto rtcm_id   = satellite->id.as_msm().value;
        auto bit_index = rtcm_id - 1;
        auto bit       = 1ull << (63 - bit_index);
        satellite_ids |= bit;
    }

    size_t satellite_count = 0;
    size_t satellite_lookup[64];
    for (auto i = 0; i < 64; i++) {
        auto bit = 1ull << (63 - i);
        if (satellite_ids & bit) {
            satellite_lookup[i] = satellite_count++;
        }
    }

    uint32_t signal_ids = 0;
    for (auto signal : signals) {
        auto rtcm_id   = signal->id.as_msm().value;
        auto bit_index = rtcm_id - 1;
        auto bit       = 1u << (31 - bit_index);
        signal_ids |= bit;
    }

    size_t signal_count = 0;
    size_t signal_lookup[32];
    for (auto i = 0; i < 32; i++) {
        auto bit = 1u << (31 - i);
        if (signal_ids & bit) {
            signal_lookup[i] = signal_count++;
        }
    }

    encoder.u64(64, satellite_ids);
    encoder.u32(32, signal_ids);

    auto cell_count = satellite_count * signal_count;
    if (cell_count > 64) {
        // TODO: How do we report this?
        cell_count = 64;
    }

    bool cell_data[64] = {0};
    for (auto& signal : signals) {
        auto signal_id       = signal->id.as_msm();
        auto satellite_id    = signal->satellite.as_msm();
        auto satellite_index = satellite_lookup[satellite_id.value - 1];
        auto signal_index    = signal_lookup[signal_id.value - 1];
        auto cell_index      = satellite_index * signal_count + signal_index;
        if (cell_index >= 64) continue;
        cell_data[cell_index] = true;
    }

    for (size_t i = 0; i < satellite_count; i++) {
        for (size_t j = 0; j < signal_count; j++) {
            auto cell_index = i * signal_count + j;
            if (cell_index >= 64) continue;
        }
    }

    for (size_t i = 0; i < cell_count; i++) {
        encoder.b(cell_data[i]);
    }

    generate_msm_satellites(msm, encoder, satellites);
    generate_msm_signals(msm, encoder, signals);

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}

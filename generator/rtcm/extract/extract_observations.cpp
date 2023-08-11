#include "extract.hpp"

#include <GNSS-RTK-Observations-r15.h>
#include <GNSS-RTK-SatelliteDataElement-r15.h>
#include <GNSS-RTK-SatelliteSignalDataElement-r15.h>
#include <asn.1/bit_string.hpp>

using namespace generator::rtcm;

namespace decode {

static Maybe<double> fine_phase_range(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    return static_cast<double>(src_signal.fine_PhaseRange_r15) * RTCM_N2_31;
}

static Maybe<double> fine_pseudo_range(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    return static_cast<double>(src_signal.fine_PseudoRange_r15) * RTCM_N2_29;
}

static Maybe<double>
fine_phase_range_rate(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    if (src_signal.fine_PhaseRangeRate_r15) {
        return static_cast<double>(*src_signal.fine_PhaseRangeRate_r15) * 0.0001;
    } else {
        return Maybe<double>();
    }
}

static Maybe<double>
carrier_to_noise_ratio(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    if (src_signal.carrier_to_noise_ratio_r15) {
        return static_cast<double>(*src_signal.carrier_to_noise_ratio_r15) * RTCM_N2_4;
    } else {
        return Maybe<double>();
    }
}

#define LOCK_TIME_SIZE 22
RTCM_CONSTEXPR static uint64_t LOCK_TIME_COEFF[LOCK_TIME_SIZE] = {
    1,    2,    4,    8,     16,    32,    64,     128,    256,    512,     1024,
    2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
};

RTCM_CONSTEXPR static uint64_t LOCK_TIME_OFFSET[LOCK_TIME_SIZE] = {
    0,        64,       256,       768,       2048,      5120,       12288,   28672,
    65536,    147456,   327680,    720896,    1572864,   3407872,    7340032, 15728640,
    33554432, 71303168, 150994944, 318767104, 671088640, 1409286144,
};

RTCM_CONSTEXPR static uint64_t LOCK_TIME_BASE[LOCK_TIME_SIZE] = {
    0,   64,  96,  128, 160, 192, 224, 256, 288, 320, 352,
    384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704,
};

static Maybe<double> lock_time(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    auto value = static_cast<uint64_t>(src_signal.lockTimeIndicator_r15);
    if (value >= LOCK_TIME_BASE[LOCK_TIME_SIZE - 1]) {
        return 67108864 / 1000.0;
    }

    for (auto i = 0; i < LOCK_TIME_SIZE - 1; i++) {
        auto start = LOCK_TIME_BASE[i];
        auto end   = LOCK_TIME_BASE[i + 1];
        if (value >= start && value < end) {
            return (LOCK_TIME_COEFF[i] * value - LOCK_TIME_OFFSET[i]) / 1000.0;
        }
    }

    return Maybe<double>();
}

static bool half_cycle_ambiguity(const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    auto halfcycle = helper::BitString::from(&src_signal.halfCycleAmbiguityIndicator_r15);
    return halfcycle->get_bit(0) != 0;
}

static Maybe<double> rough_range(const GNSS_RTK_SatelliteDataElement_r15& src_satellite) {
    return static_cast<double>(src_satellite.rough_range_r15) * RTCM_N2_10;
}

static Maybe<int32_t> integer_ms(const GNSS_RTK_SatelliteDataElement_r15& src_satellite) {
    if (src_satellite.integer_ms_r15) {
        return static_cast<int32_t>(*src_satellite.integer_ms_r15);
    } else {
        return Maybe<int32_t>();
    }
}

static Maybe<double>
rough_phase_range_rate(const GNSS_RTK_SatelliteDataElement_r15& src_satellite) {
    if (src_satellite.rough_phase_range_rate_r15) {
        return static_cast<double>(*src_satellite.rough_phase_range_rate_r15);
    } else {
        return Maybe<double>();
    }
}

static SatelliteId satellite_id(GenericGnssId                            gnss_id,
                                const GNSS_RTK_SatelliteDataElement_r15& src_satellite) {
    auto gnss = SatelliteId::Gnss::UNKNOWN;
    switch (gnss_id) {
    case GenericGnssId::GPS: gnss = SatelliteId::Gnss::GPS; break;
    case GenericGnssId::GLONASS: gnss = SatelliteId::Gnss::GLONASS; break;
    case GenericGnssId::GALILEO: gnss = SatelliteId::Gnss::GALILEO; break;
    case GenericGnssId::BEIDOU: gnss = SatelliteId::Gnss::BEIDOU; break;
    }

    auto id = src_satellite.svID_r15.satellite_id;
    return SatelliteId::from_lpp(gnss, id);
}

static SignalId signal_id(GenericGnssId                                  gnss_id,
                          const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    auto gnss = SignalId::Gnss::UNKNOWN;
    switch (gnss_id) {
    case GenericGnssId::GPS: gnss = SignalId::Gnss::GPS; break;
    case GenericGnssId::GLONASS: gnss = SignalId::Gnss::GLONASS; break;
    case GenericGnssId::GALILEO: gnss = SignalId::Gnss::GALILEO; break;
    case GenericGnssId::BEIDOU: gnss = SignalId::Gnss::BEIDOU; break;
    }

    auto id = src_signal.gnss_SignalID_r15.gnss_SignalID;
    if (src_signal.gnss_SignalID_r15.ext1 &&
        src_signal.gnss_SignalID_r15.ext1->gnss_SignalID_Ext_r15) {
        id = *src_signal.gnss_SignalID_r15.ext1->gnss_SignalID_Ext_r15;
    }

    return SignalId::from_lpp(gnss, id);
}
}  // namespace decode

static void extract_signal(Observations& observations, GenericGnssId gnss_id,
                           SatelliteId                                    satellite_id,
                           const GNSS_RTK_SatelliteSignalDataElement_r15& src_signal) {
    Signal dst_signal{};
    dst_signal.id                     = decode::signal_id(gnss_id, src_signal);
    dst_signal.satellite              = satellite_id;
    dst_signal.fine_phase_range       = decode::fine_phase_range(src_signal);
    dst_signal.fine_pseudo_range      = decode::fine_pseudo_range(src_signal);
    dst_signal.fine_phase_range_rate  = decode::fine_phase_range_rate(src_signal);
    dst_signal.carrier_to_noise_ratio = decode::carrier_to_noise_ratio(src_signal);
    dst_signal.lock_time              = decode::lock_time(src_signal);
    dst_signal.half_cycle_ambiguity   = decode::half_cycle_ambiguity(src_signal);

    observations.signals.emplace_back(dst_signal);
}

static void extract_satellite(Observations& observations, GenericGnssId gnss_id,
                              const GNSS_RTK_SatelliteDataElement_r15& src_satellite) {
    Satellite dst_satellite{};
    dst_satellite.id                     = decode::satellite_id(gnss_id, src_satellite);
    dst_satellite.rough_range            = decode::rough_range(src_satellite);
    dst_satellite.integer_ms             = decode::integer_ms(src_satellite);
    dst_satellite.rough_phase_range_rate = decode::rough_phase_range_rate(src_satellite);

    auto& list = src_satellite.gnss_rtk_SatelliteSignalDataList_r15.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        extract_signal(observations, gnss_id, dst_satellite.id, *list.array[i]);
    }

    observations.satellites.emplace_back(dst_satellite);
}

extern void extract_observations(RtkData& data, GenericGnssId gnss_id,
                                 const GNSS_RTK_Observations_r15& src_observation) {
    auto  dst_observation = std::unique_ptr<Observations>(new Observations());
    auto& observation     = *dst_observation.get();
    observation.time      = decode::epoch_time(src_observation.epochTime_r15);

    auto& list = src_observation.gnss_ObservationList_r15.list;
    for (auto i = 0; i < list.count; i++) {
        if (!list.array[i]) continue;
        extract_satellite(observation, gnss_id, *list.array[i]);
    }

    data.observations[gnss_id] = std::move(dst_observation);
}

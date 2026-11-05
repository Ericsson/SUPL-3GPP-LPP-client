#include <doctest/doctest.h>
#include <generator/rtcm/generator.hpp>

#include <cstdint>
#include <vector>

using namespace generator::rtcm;

namespace {

uint64_t bits(std::vector<uint8_t> const& data, size_t start, size_t count) {
    uint64_t value = 0;
    for (size_t i = 0; i < count; i++) {
        auto bit = start + i;
        value    = (value << 1) | ((data[bit / 8] >> (7 - (bit % 8))) & 1u);
    }
    return value;
}

Observations observations(SatelliteId satellite, SignalId signal) {
    Observations obs{};
    obs.time = ts::Tai::now();

    Satellite sat{};
    sat.id          = satellite;
    sat.integer_ms  = 70;
    sat.rough_range = 0.5;
    if (satellite.is_glonass()) sat.frequency_channel = -3;
    obs.satellites.push_back(sat);

    Signal sig{};
    sig.id                = signal;
    sig.satellite         = satellite;
    sig.fine_pseudo_range = 0.0;
    sig.fine_phase_range  = 0.0;
    sig.lock_time         = 100.0;
    obs.signals.push_back(sig);
    return obs;
}

// MSM header: frame (24) + type (12) + station (12) + epoch (30) + flags (22)
constexpr size_t SATELLITE_MASK = 24 + 12 + 12 + 30 + 1 + 3 + 7 + 2 + 2 + 1 + 3;

}  // namespace

TEST_CASE("RTCM MSM - QZSS") {
    CommonObservationInfo common{};
    auto obs = observations(SatelliteId::from_qzs_prn(194), SignalId::QZSS_L1C_D_P);

    auto messages = generate_msm(4, true, GenericGnssId::QZSS, common, obs);
    REQUIRE(messages.size() == 1);
    CHECK(messages[0].id() == 1114);

    auto& data = messages[0].data();
    CHECK(bits(data, 24, 12) == 1114);
    // J02 is satellite mask id 2, L1C(D+P) is signal mask id 32
    CHECK(bits(data, SATELLITE_MASK, 64) == (1ull << 62));
    CHECK(bits(data, SATELLITE_MASK + 64, 32) == 1u);
}

TEST_CASE("RTCM MSM - GLONASS frequency channel") {
    CommonObservationInfo common{};
    auto obs = observations(SatelliteId::from_glo_slot(5), SignalId::GLONASS_G1_CA);

    auto messages = generate_msm(5, true, GenericGnssId::GLONASS, common, obs);
    REQUIRE(messages.size() == 1);
    CHECK(messages[0].id() == 1085);

    auto& data = messages[0].data();
    // GLONASS epoch is 3 + 27 bits, same length as the other GNSS
    CHECK(bits(data, SATELLITE_MASK, 64) == (1ull << 59));
    // one cell, then MSM5 satellite data: integer ms (8), extended info (4)
    auto satellite_data = SATELLITE_MASK + 64 + 32 + 1;
    CHECK(bits(data, satellite_data, 8) == 70);
    CHECK(bits(data, satellite_data + 8, 4) == 4);  // channel -3 + 7
}

TEST_CASE("RTCM 1230 - GLONASS code-phase biases") {
    BiasInformation bias{};
    bias.reference_station_id = 12;
    bias.indicator            = true;
    bias.mask                 = 0xF;
    bias.l1_ca                = 0.0;
    bias.l1_p                 = 0.0;
    bias.l2_ca                = 0.0;
    bias.l2_p                 = 0.0;

    auto message = generate_1230(bias);
    CHECK(message.id() == 1230);
    auto& data = message.data();
    CHECK(bits(data, 24, 12) == 1230);
    CHECK(bits(data, 36, 12) == 12);
    CHECK(bits(data, 48, 1) == 1);
    CHECK(bits(data, 52, 4) == 0xF);
}

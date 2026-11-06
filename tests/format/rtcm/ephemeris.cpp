#include <doctest/doctest.h>
#include <format/rtcm/1020.hpp>
#include <format/rtcm/1044.hpp>
#include <format/rtcm/ephemeris.hpp>
#include <format/rtcm/parser.hpp>
#include <time/glo.hpp>
#include <time/utc.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

namespace {

struct BitWriter {
    std::vector<uint8_t> data;
    size_t               bit = 0;

    void u(size_t n, uint64_t value) {
        for (size_t i = 0; i < n; i++, bit++) {
            if (bit / 8 >= data.size()) data.push_back(0);
            if ((value >> (n - 1 - i)) & 1u)
                data[bit / 8] |= static_cast<uint8_t>(0x80u >> (bit % 8));
        }
    }
    void s(size_t n, int64_t value) { u(n, static_cast<uint64_t>(value) & ((1ull << n) - 1)); }
    void sm(size_t n, int64_t value) {
        auto magnitude = static_cast<uint64_t>(value < 0 ? -value : value);
        u(n, (value < 0 ? (1ull << (n - 1)) : 0) | magnitude);
    }
};

uint32_t crc24q(std::vector<uint8_t> const& data) {
    uint32_t crc = 0;
    for (auto byte : data) {
        crc ^= static_cast<uint32_t>(byte) << 16;
        for (int i = 0; i < 8; i++) {
            crc <<= 1;
            if (crc & 0x1000000) crc ^= 0x1864CFB;
        }
    }
    return crc & 0xFFFFFF;
}

std::vector<uint8_t> frame(BitWriter const& payload) {
    BitWriter w;
    w.u(8, 0xD3);
    w.u(6, 0);
    w.u(10, payload.data.size());
    for (auto byte : payload.data)
        w.u(8, byte);
    w.u(24, crc24q(w.data));
    return w.data;
}

std::unique_ptr<format::rtcm::Message> parse(std::vector<uint8_t> const& bytes) {
    format::rtcm::Parser parser;
    parser.append(bytes.data(), bytes.size());
    return parser.try_parse();
}

}  // namespace

TEST_CASE("RTCM 1044 - QZSS ephemeris") {
    BitWriter w;
    w.u(12, 1044);
    w.u(4, 2);             // J02 = PRN 194
    w.u(16, 32400);        // toc / 16
    w.s(8, 0);             // af2
    w.s(16, -12);          // af1
    w.s(22, -1000);        // af0
    w.u(8, 64);            // IODE
    w.s(16, -83);          // Crs
    w.s(16, 100);          // delta n
    w.s(32, -100000);      // M0
    w.s(16, -33);          // Cuc
    w.u(32, 600000000);    // e
    w.s(16, 1718);         // Cus
    w.u(32, 2702000000u);  // sqrt(A)
    w.u(16, 32400);        // toe / 16
    w.s(16, 97);           // Cic
    w.s(32, 1000000);      // OMEGA0
    w.s(16, 2);            // Cis
    w.s(32, 600000000);    // i0
    w.s(16, 10558);        // Crc
    w.s(32, -1000000);     // omega
    w.s(24, -1500);        // OMEGA dot
    w.s(14, 10);           // IDOT
    w.u(2, 1);             // codes on L2
    w.u(10, 373);          // week
    w.u(4, 2);             // URA
    w.u(6, 0);             // health
    w.s(8, 5);             // TGD
    w.u(10, 64);           // IODC
    w.u(1, 0);             // fit

    auto message = parse(frame(w));
    REQUIRE(message);
    auto rtcm = dynamic_cast<format::rtcm::Rtcm1044*>(message.get());
    REQUIRE(rtcm);
    CHECK(rtcm->prn == 194);
    CHECK(rtcm->toc == doctest::Approx(518400.0));
    CHECK(rtcm->af0 == doctest::Approx(std::ldexp(-1000.0, -31)));
    CHECK(rtcm->sqrt_a == doctest::Approx(std::ldexp(2702000000.0, -19)));
    CHECK(rtcm->m0 == doctest::Approx(std::ldexp(-100000.0, -31) * M_PI));
    CHECK(rtcm->omega_dot == doctest::Approx(std::ldexp(-1500.0, -43) * M_PI));
    CHECK(rtcm->week == 373);
    CHECK(rtcm->iodc == 64);
    CHECK(rtcm->tgd == doctest::Approx(std::ldexp(5.0, -31)));

    auto eph = format::rtcm::to_ephemeris(*rtcm);
    CHECK(eph.prn == 194);
    CHECK(eph.lpp_iod == 64);
    CHECK(eph.a == doctest::Approx(rtcm->sqrt_a * rtcm->sqrt_a));

    auto clone = message->clone();
    auto copy  = dynamic_cast<format::rtcm::Rtcm1044*>(clone.get());
    REQUIRE(copy);
    CHECK(copy->iodc == 64);
}

TEST_CASE("RTCM 1020 - GLONASS ephemeris") {
    BitWriter w;
    w.u(12, 1020);
    w.u(6, 5);   // slot
    w.u(5, 8);   // channel + 7
    w.u(1, 0);   // almanac health
    w.u(1, 0);   // almanac health availability
    w.u(2, 0);   // P1
    w.u(5, 3);   // tk hours
    w.u(6, 14);  // tk minutes
    w.u(1, 1);   // tk 30 seconds
    w.u(1, 0);   // Bn MSB
    w.u(1, 1);   // P2
    w.u(7, 13);  // tb
    // x', x, x''
    w.sm(24, -2215512);
    w.sm(27, 21979888);
    w.sm(5, 1);
    w.sm(24, -1430340);
    w.sm(27, -23431435);
    w.sm(5, 0);
    w.sm(24, 245000);
    w.sm(27, 39986766);
    w.sm(5, -3);
    w.u(1, 0);         // P3
    w.sm(11, 0);       // gamma
    w.u(2, 0);         // P
    w.u(1, 0);         // ln
    w.sm(22, -25676);  // tau
    w.sm(5, 0);        // delta tau
    w.u(5, 0);         // En
    w.u(1, 0);         // P4
    w.u(4, 0);         // FT
    w.u(11, 888);      // NT
    w.u(2, 1);         // M
    w.u(1, 1);         // additional data
    w.u(11, 0);        // NA
    w.sm(32, 0);       // tau c
    w.u(5, 8);         // N4
    w.sm(22, 0);       // tau GPS
    w.u(1, 0);         // ln
    w.u(7, 0);         // reserved

    auto message = parse(frame(w));
    REQUIRE(message);
    auto rtcm = dynamic_cast<format::rtcm::Rtcm1020*>(message.get());
    REQUIRE(rtcm);
    CHECK(rtcm->slot == 5);
    CHECK(rtcm->frequency_channel == 1);
    CHECK(rtcm->tb == 13);
    CHECK(rtcm->tk_s == 30);
    CHECK(rtcm->position[0] == doctest::Approx(std::ldexp(21979888.0, -11)));
    CHECK(rtcm->position[1] == doctest::Approx(std::ldexp(-23431435.0, -11)));
    CHECK(rtcm->velocity[0] == doctest::Approx(std::ldexp(-2215512.0, -20)));
    CHECK(rtcm->acceleration[2] == doctest::Approx(std::ldexp(-3.0, -30)));
    CHECK(rtcm->tau_n == doctest::Approx(std::ldexp(-25676.0, -30)));
    CHECK(rtcm->n_t == 888);
    CHECK(rtcm->n_4 == 8);

    auto eph = format::rtcm::to_ephemeris(*rtcm, ts::Tai::now());
    CHECK(eph.slot_number == 5);
    CHECK(eph.frequency_number == 1);
    CHECK(eph.lpp_iod == 13);
    CHECK(eph.reference_time.time_of_day().full_seconds() == doctest::Approx(13 * 900.0));
    // N4 = 8 (2024-2027), NT = 888 -> 2026-06-06
    auto expected = ts::Glo(ts::Utc::from_date_time(2026, 6, 6, 0, 15, 0.0));
    CHECK(eph.reference_time.timestamp().full_seconds() ==
          doctest::Approx(expected.timestamp().full_seconds()));
}

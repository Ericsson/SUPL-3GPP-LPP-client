#include <doctest/doctest.h>
#include <ephemeris/glo.hpp>
#include <format/nav/glo.hpp>
#include <time/glo.hpp>
#include <time/utc.hpp>

#include <cmath>
#include <cstdint>

namespace {

// Builds a GLONASS string from ICD bit fields and fills in the Hamming code (ICD 5.0, 4.7)
struct StringBuilder {
    uint32_t bits[4]{};

    void set(int b, bool value) {
        auto offset = 85 - b;
        auto mask   = 1u << (31 - (offset % 32));
        if (value)
            bits[offset / 32] |= mask;
        else
            bits[offset / 32] &= ~mask;
    }
    bool get(int b) const {
        auto offset = 85 - b;
        return (bits[offset / 32] >> (31 - (offset % 32))) & 1u;
    }
    void field(int high, int low, uint32_t value) {
        for (auto b = low; b <= high; b++)
            set(b, (value >> (b - low)) & 1u);
    }
    void sm(int high, int low, int64_t value) {
        auto length    = high - low + 1;
        auto magnitude = static_cast<uint32_t>(value < 0 ? -value : value);
        field(high, low, (value < 0 ? (1u << (length - 1)) : 0u) | magnitude);
    }

    format::nav::GloString finish(uint32_t frame) {
        auto position = 3;
        int  check[7] = {};
        for (auto b = 9; b <= 85; b++, position++) {
            while ((position & (position - 1)) == 0)
                position++;
            if (!get(b)) continue;
            for (auto k = 0; k < 7; k++)
                if (position & (1 << k)) check[k] ^= 1;
        }
        for (auto k = 0; k < 7; k++)
            set(k + 1, check[k] != 0);
        // check bits 1..7, then the overall parity in bit 8
        auto parity = 0;
        for (auto b = 1; b <= 85; b++)
            if (b != 8) parity ^= get(b) ? 1 : 0;
        set(8, parity != 0);
        bits[3] = frame;
        return format::nav::GloString::from_sfrbx(bits);
    }
};

}  // namespace

TEST_CASE("GLONASS navigation string decoding") {
    StringBuilder s1;
    s1.field(84, 81, 1);
    s1.sm(64, 41, -2215512);
    s1.sm(40, 36, 1);
    s1.sm(35, 9, 21979888);

    StringBuilder s2;
    s2.field(84, 81, 2);
    s2.field(80, 78, 0);
    s2.field(76, 70, 13);
    s2.sm(64, 41, -1430340);
    s2.sm(40, 36, 0);
    s2.sm(35, 9, -23431435);

    StringBuilder s3;
    s3.field(84, 81, 3);
    s3.sm(79, 69, 2);
    s3.sm(64, 41, 245000);
    s3.sm(40, 36, -3);
    s3.sm(35, 9, 39986766);

    StringBuilder s4;
    s4.field(84, 81, 4);
    s4.sm(80, 59, -25676);
    s4.field(53, 49, 2);
    s4.field(26, 16, 888);
    s4.field(10, 9, 1);

    StringBuilder s5;
    s5.field(84, 81, 5);
    s5.field(36, 32, 8);

    auto now = ts::Tai(ts::Utc::from_date_time(2026, 6, 6, 1, 0, 0.0));

    auto str1 = s1.finish(7);
    CHECK(str1.check());
    CHECK(str1.number() == 1);

    auto corrupted = str1;
    corrupted.bits[1] ^= 0x100;
    CHECK_FALSE(corrupted.check());

    format::nav::GloEphemerisCollector collector;
    ephemeris::GloEphemeris            eph{};
    CHECK_FALSE(collector.process(5, 1, s5.finish(7), now, eph));
    CHECK_FALSE(collector.process(5, 1, str1, now, eph));
    CHECK_FALSE(collector.process(5, 1, s2.finish(7), now, eph));
    CHECK_FALSE(collector.process(5, 1, s3.finish(7), now, eph));
    REQUIRE(collector.process(5, 1, s4.finish(7), now, eph));

    CHECK(eph.slot_number == 5);
    CHECK(eph.frequency_number == 1);
    CHECK(eph.lpp_iod == 13);
    CHECK(eph.health == 0);
    CHECK(eph.age == 2);
    CHECK(eph.position.x == doctest::Approx(std::ldexp(21979888.0, -11)));
    CHECK(eph.position.y == doctest::Approx(std::ldexp(-23431435.0, -11)));
    CHECK(eph.velocity.z == doctest::Approx(std::ldexp(245000.0, -20)));
    CHECK(eph.acceleration.z == doctest::Approx(std::ldexp(-3.0, -30)));
    CHECK(eph.gamma_n == doctest::Approx(std::ldexp(2.0, -40)));
    CHECK(eph.tau_n == doctest::Approx(std::ldexp(-25676.0, -30)));
    auto expected = ts::Glo(ts::Utc::from_date_time(2026, 6, 6, 0, 15, 0.0));
    CHECK(eph.reference_time.timestamp().full_seconds() ==
          doctest::Approx(expected.timestamp().full_seconds()));

    // strings from different frames are not combined
    format::nav::GloEphemerisCollector mixed;
    CHECK_FALSE(mixed.process(5, 1, s1.finish(7), now, eph));
    CHECK_FALSE(mixed.process(5, 1, s2.finish(8), now, eph));
    CHECK_FALSE(mixed.process(5, 1, s3.finish(8), now, eph));
    CHECK_FALSE(mixed.process(5, 1, s4.finish(8), now, eph));
}

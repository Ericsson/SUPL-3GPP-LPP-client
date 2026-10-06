#include <cstdio>
#include <cstring>
#include <doctest/doctest.h>
#include <format/nmea/gga.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>

namespace {

/// Build an NMEA sentence with a correct checksum from its body.
std::string sentence(std::string const& body) {
    unsigned checksum = 0;
    for (auto c : body) {
        checksum ^= static_cast<unsigned char>(c);
    }
    char suffix[8];
    snprintf(suffix, sizeof(suffix), "*%02X\r\n", checksum & 0xFF);
    return "$" + body + suffix;
}

/// Parse a single GGA sentence with the given time field. Returns nullptr if it was not
/// accepted as a GGA message.
std::unique_ptr<format::nmea::Message> parse_gga_with_time(std::string const& time) {
    auto body = "GPGGA," + time + ",4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    auto text = sentence(body);

    format::nmea::Parser parser;
    parser.append(reinterpret_cast<uint8_t const*>(text.data()), text.size());
    auto message = parser.try_parse();
    if (!message) return nullptr;
    if (!dynamic_cast<format::nmea::GgaMessage*>(message.get())) return nullptr;
    return message;
}

double time_of_day_of(format::nmea::Message const& message) {
    auto const& gga = dynamic_cast<format::nmea::GgaMessage const&>(message);
    return gga.time_of_day().timestamp().full_seconds();
}

}  // namespace

TEST_CASE("NMEA GGA - time field fraction is optional") {
    // [NMEA 0183]: the decimal fraction of the seconds field is optional. Receivers that emit
    // integer seconds must not be rejected.
    auto without = parse_gga_with_time("123519");
    REQUIRE(without != nullptr);

    auto with = parse_gga_with_time("123519.00");
    REQUIRE(with != nullptr);

    CHECK(time_of_day_of(*without) == doctest::Approx(time_of_day_of(*with)));
}

TEST_CASE("NMEA GGA - time field fraction is scaled by its digit count") {
    auto base = parse_gga_with_time("123519");
    REQUIRE(base != nullptr);
    auto reference = time_of_day_of(*base);

    struct Case {
        char const* time;
        double      offset;
    };
    Case const cases[] = {
        {"123519.5", 0.5},   {"123519.50", 0.5},  {"123519.500", 0.5}, {"123519.3", 0.3},
        {"123519.300", 0.3}, {"123519.05", 0.05}, {"123519.25", 0.25}, {"123519.123456", 0.123456},
    };

    for (auto const& c : cases) {
        auto message = parse_gga_with_time(c.time);
        REQUIRE_MESSAGE(message != nullptr, c.time);
        CHECK_MESSAGE(time_of_day_of(*message) - reference ==
                          doctest::Approx(c.offset).epsilon(1e-6),
                      c.time);
    }
}

TEST_CASE("NMEA GGA - malformed time fields are rejected") {
    // A short hhmmss field used to be silently misparsed via substr: "12351.0" became
    // 12:35:01. Out-of-range values were accepted outright.
    CHECK(parse_gga_with_time("12351.0") == nullptr);    // 5 digits
    CHECK(parse_gga_with_time("1235190.0") == nullptr);  // 7 digits
    CHECK(parse_gga_with_time("253519.0") == nullptr);   // hour 25
    CHECK(parse_gga_with_time("126019.0") == nullptr);   // minute 60
    CHECK(parse_gga_with_time("123561.0") == nullptr);   // second 61

    CHECK(parse_gga_with_time("12351") == nullptr);
    CHECK(parse_gga_with_time("253519") == nullptr);
    CHECK(parse_gga_with_time("") == nullptr);
    CHECK(parse_gga_with_time("12.34.56") == nullptr);
}

TEST_CASE("NMEA parser - valid GGA message") {
    format::nmea::Parser parser;
    char const* msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message != nullptr);
    CHECK(message->prefix() == "GPGGA");
}

TEST_CASE("NMEA parser - invalid checksum") {
    format::nmea::Parser parser;
    char const* msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*FF\r\n";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

TEST_CASE("NMEA parser - incomplete message") {
    format::nmea::Parser parser;
    char const*          msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

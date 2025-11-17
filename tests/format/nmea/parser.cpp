#include <doctest/doctest.h>
#include <format/nmea/parser.hpp>
#include <format/nmea/message.hpp>
#include <cstring>

TEST_CASE("NMEA parser - valid GGA message") {
    format::nmea::Parser parser;
    const char* msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    parser.append(reinterpret_cast<const uint8_t*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message != nullptr);
    CHECK(message->prefix() == "GPGGA");
}

TEST_CASE("NMEA parser - invalid checksum") {
    format::nmea::Parser parser;
    const char* msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*FF\r\n";
    parser.append(reinterpret_cast<const uint8_t*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

TEST_CASE("NMEA parser - incomplete message") {
    format::nmea::Parser parser;
    const char* msg = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    parser.append(reinterpret_cast<const uint8_t*>(msg), std::strlen(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

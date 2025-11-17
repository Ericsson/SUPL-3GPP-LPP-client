#include <doctest/doctest.h>
#include <format/rtcm/message.hpp>
#include <format/rtcm/parser.hpp>

TEST_CASE("RTCM parser - invalid preamble") {
    format::rtcm::Parser parser;
    uint8_t const        msg[] = {0xFF, 0x00, 0x13, 0x3E, 0xD0};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

TEST_CASE("RTCM parser - incomplete message") {
    format::rtcm::Parser parser;
    uint8_t const        msg[] = {0xD3, 0x00, 0x13};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

TEST_CASE("RTCM parser - preamble only") {
    format::rtcm::Parser parser;
    uint8_t const        msg[] = {0xD3};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

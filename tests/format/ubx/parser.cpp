#include <doctest/doctest.h>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>

TEST_CASE("UBX parser - valid ACK-ACK message") {
    format::ubx::Parser parser;
    uint8_t const       msg[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x01, 0x0F, 0x38};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message != nullptr);
    if (message) {
        CHECK(message->message_class() == 0x05);
        CHECK(message->message_id() == 0x01);
    }
}

TEST_CASE("UBX parser - invalid sync bytes") {
    format::ubx::Parser parser;
    uint8_t const       msg[] = {0xFF, 0xFF, 0x05, 0x01, 0x02, 0x00, 0x06, 0x01, 0x0F, 0x38};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

TEST_CASE("UBX parser - incomplete message") {
    format::ubx::Parser parser;
    uint8_t const       msg[] = {0xB5, 0x62, 0x05, 0x01};
    parser.append(msg, sizeof(msg));
    auto message = parser.try_parse();
    CHECK(message == nullptr);
}

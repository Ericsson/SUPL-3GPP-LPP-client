#include <cstring>
#include <doctest/doctest.h>
#include <format/at/parser.hpp>

TEST_CASE("AT parser - OK response") {
    format::at::Parser parser;
    char const*        msg = "OK\r\n";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    parser.process();
    CHECK(parser.has_lines());
    CHECK(parser.count() == 1);
    if (parser.has_lines()) {
        CHECK(parser.peek_line() == "OK");
    }
}

TEST_CASE("AT parser - ERROR response") {
    format::at::Parser parser;
    char const*        msg = "ERROR\r\n";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    parser.process();
    CHECK(parser.has_lines());
    if (parser.has_lines()) {
        CHECK(parser.peek_line() == "ERROR");
    }
}

TEST_CASE("AT parser - multiple lines") {
    format::at::Parser parser;
    char const*        msg = "+CGMI: Manufacturer\r\nOK\r\n";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    parser.process();
    CHECK(parser.count() == 2);
}

TEST_CASE("AT parser - incomplete line") {
    format::at::Parser parser;
    char const*        msg = "OK";
    parser.append(reinterpret_cast<uint8_t const*>(msg), std::strlen(msg));
    parser.process();
    CHECK(parser.count() == 0);
}

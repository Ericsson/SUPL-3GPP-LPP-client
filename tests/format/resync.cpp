// Regression tests for the parser resynchronization contract.
//
// `try_parse()` must either return a message, or consume nothing and return nullptr. It must
// never consume bytes and report "nothing", because every caller drains with
// `while (auto msg = try_parse())` and would stop early, leaving the buffer to back up until it
// overflows. These tests feed interleaved/corrupted streams and assert that a single drain loop
// recovers every valid message.

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <doctest/doctest.h>

#include <format/at/parser.hpp>
#include <format/ctrl/message.hpp>
#include <format/ctrl/parser.hpp>
#include <format/nmea/message.hpp>
#include <format/nmea/parser.hpp>
#include <format/rtcm/message.hpp>
#include <format/rtcm/parser.hpp>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>

namespace {

//
// RTCM helpers
//

uint32_t crc24q(uint8_t const* data, size_t length) {
    static uint32_t const POLYNOMIAL = 0x1864CFBu;
    uint32_t              crc        = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= static_cast<uint32_t>(data[i]) << 16u;
        for (int bit = 0; bit < 8; bit++) {
            crc <<= 1u;
            if ((crc & 0x1000000u) != 0) crc ^= POLYNOMIAL;
        }
    }
    return crc & 0xFFFFFFu;
}

/// Build a complete RTCM3 frame with the given message number and a deterministic payload.
std::vector<uint8_t> rtcm_frame(uint16_t type, size_t payload_length) {
    REQUIRE(payload_length >= 2);
    std::vector<uint8_t> frame;
    frame.push_back(0xD3);
    frame.push_back(static_cast<uint8_t>((payload_length >> 8) & 0x03));
    frame.push_back(static_cast<uint8_t>(payload_length & 0xFF));
    frame.push_back(static_cast<uint8_t>(type >> 4));
    frame.push_back(static_cast<uint8_t>((type & 0x0F) << 4));
    for (size_t i = 2; i < payload_length; i++) {
        // Include 0xD3 bytes so the payload itself contains false sync markers.
        frame.push_back(i % 7 == 0 ? 0xD3 : static_cast<uint8_t>(i));
    }
    auto crc = crc24q(frame.data(), frame.size());
    frame.push_back(static_cast<uint8_t>((crc >> 16) & 0xFF));
    frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(crc & 0xFF));
    return frame;
}

//
// UBX helpers
//

std::vector<uint8_t> ubx_frame(uint8_t message_class, uint8_t message_id, size_t payload_length) {
    std::vector<uint8_t> frame;
    frame.push_back(0xB5);
    frame.push_back(0x62);
    frame.push_back(message_class);
    frame.push_back(message_id);
    frame.push_back(static_cast<uint8_t>(payload_length & 0xFF));
    frame.push_back(static_cast<uint8_t>((payload_length >> 8) & 0xFF));
    for (size_t i = 0; i < payload_length; i++) {
        frame.push_back(i % 5 == 0 ? 0xB5 : static_cast<uint8_t>(i));
    }

    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    for (size_t i = 2; i < frame.size(); i++) {
        ck_a = static_cast<uint8_t>(ck_a + frame[i]);
        ck_b = static_cast<uint8_t>(ck_b + ck_a);
    }
    frame.push_back(ck_a);
    frame.push_back(ck_b);
    return frame;
}

void append(std::vector<uint8_t>& out, std::vector<uint8_t> const& in) {
    out.insert(out.end(), in.begin(), in.end());
}

void append(std::vector<uint8_t>& out, char const* text) {
    out.insert(out.end(), text, text + std::strlen(text));
}

/// Build an NMEA sentence with a correct checksum from its body (without '$' and '*XY').
std::string nmea_sentence(std::string const& body) {
    unsigned checksum = 0;
    for (auto c : body) {
        checksum ^= static_cast<unsigned char>(c);
    }
    char suffix[8];
    snprintf(suffix, sizeof(suffix), "*%02X\r\n", checksum & 0xFF);
    return "$" + body + suffix;
}

void append_nmea(std::vector<uint8_t>& out, std::string const& body) {
    auto sentence = nmea_sentence(body);
    out.insert(out.end(), sentence.begin(), sentence.end());
}

/// Non-RTCM, non-UBX, non-NMEA filler that still contains bytes that look like sync markers.
std::vector<uint8_t> junk(size_t length) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i < length; i++) {
        switch (i % 5) {
        case 0: out.push_back(0xFD); break;
        case 1: out.push_back(0xD3); break;
        case 2: out.push_back(0xB5); break;
        case 3: out.push_back(0x62); break;
        default: out.push_back('$'); break;
        }
    }
    return out;
}

/// Trailing filler that contains no sync marker for any format. A length-prefixed format
/// cannot reject a false sync near the end of the buffer until enough bytes have arrived to
/// evaluate its checksum, so streaming tests end with a flush to resolve pending candidates.
std::vector<uint8_t> flush() {
    return std::vector<uint8_t>(16384, 0x00);
}

template <typename Parser>
size_t drain(Parser& parser) {
    size_t count = 0;
    while (auto message = parser.try_parse()) {
        count++;
    }
    return count;
}

}  // namespace

//
// RTCM
//

TEST_CASE("RTCM parser - recovers both frames across interleaved junk") {
    std::vector<uint8_t> stream;
    append(stream, rtcm_frame(1005, 19));
    append(stream, junk(200));
    append_nmea(stream, "GNGGA,202328.300,4445.92375387,N,02028.83270759,E,1,15,0.84,181.7,M");
    append(stream, junk(37));
    append(stream, rtcm_frame(1074, 107));
    append(stream, flush());

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 2);
    CHECK(parser.discarded_bytes() > 0);
    CHECK(parser.overflow_events() == 0);
}

TEST_CASE("RTCM parser - recovers after a truncated frame") {
    // A frame whose declared length is never satisfied, as produced by a lossy serial link.
    auto truncated = rtcm_frame(1074, 107);
    truncated.resize(40);

    std::vector<uint8_t> stream;
    append(stream, truncated);
    append(stream, rtcm_frame(1084, 53));
    append(stream, flush());

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
    CHECK(parser.frame_errors() > 0);
}

TEST_CASE("RTCM parser - recovers after a corrupted payload") {
    auto corrupted = rtcm_frame(1094, 129);
    corrupted[20] ^= 0xFF;

    std::vector<uint8_t> stream;
    append(stream, corrupted);
    append(stream, rtcm_frame(1124, 120));
    append(stream, flush());

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
    CHECK(parser.frame_errors() > 0);
}

TEST_CASE("RTCM parser - many corruptions in one append do not stall the drain") {
    std::vector<uint8_t> stream;
    size_t               expected = 0;
    for (int i = 0; i < 200; i++) {
        auto broken = rtcm_frame(1074, 107);
        broken[30] ^= 0xFF;
        append(stream, broken);
        append(stream, rtcm_frame(1084, 53));
        expected++;
    }
    append(stream, flush());

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == expected);
    CHECK(parser.overflow_events() == 0);
}

TEST_CASE("RTCM parser - streaming interleaved input keeps up without overflow") {
    // This is the shape of the real failure: RTCM frames, NMEA sentences and filler bytes on
    // one serial line, delivered in read-sized chunks. Previously each resynchronization
    // aborted the caller's drain loop, so the buffer backed up until it overflowed.
    std::vector<uint8_t> stream;
    size_t               expected = 0;
    for (int epoch = 0; epoch < 400; epoch++) {
        append(stream, rtcm_frame(1074, 107));
        expected++;
        auto broken = rtcm_frame(1084, 53);
        broken[10] ^= 0xFF;  // truncation/corruption from the lossy link
        append(stream, broken);
        append(stream, junk(24));
        append_nmea(stream, "GNGGA,202328.300,4445.92375387,N,02028.83270759,E,1,15,0.84,181.7,M");
        append(stream, rtcm_frame(1094, 129));
        expected++;
    }
    append(stream, flush());

    format::rtcm::Parser parser;
    size_t               count  = 0;
    size_t               offset = 0;
    while (offset < stream.size()) {
        auto chunk = std::min<size_t>(4096, stream.size() - offset);
        REQUIRE(parser.append(stream.data() + offset, chunk));
        offset += chunk;
        count += drain(parser);
    }

    CHECK(count == expected);
    CHECK(parser.overflow_events() == 0);
}

TEST_CASE("RTCM parser - pure junk terminates and consumes everything") {
    auto stream = junk(4096);

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 0);
    // Only an incomplete trailing candidate may remain.
    CHECK(parser.buffer_length() < 8);
}

TEST_CASE("RTCM parser - nullptr never consumes data") {
    auto stream = rtcm_frame(1005, 19);
    stream.resize(stream.size() - 1);  // one byte short

    format::rtcm::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    auto before = parser.buffer_length();
    CHECK(parser.try_parse() == nullptr);
    CHECK(parser.buffer_length() == before);
    CHECK(parser.try_parse() == nullptr);
    CHECK(parser.buffer_length() == before);
}

//
// UBX
//

TEST_CASE("UBX parser - recovers both frames across interleaved junk") {
    std::vector<uint8_t> stream;
    append(stream, ubx_frame(0x05, 0x01, 2));
    append(stream, junk(300));
    append(stream, ubx_frame(0x05, 0x00, 2));
    append(stream, flush());

    format::ubx::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 2);
    CHECK(parser.discarded_bytes() > 0);
}

TEST_CASE("UBX parser - recovers after a corrupted checksum") {
    auto corrupted   = ubx_frame(0x05, 0x01, 2);
    corrupted.back() = static_cast<uint8_t>(corrupted.back() ^ 0xFF);

    std::vector<uint8_t> stream;
    append(stream, corrupted);
    append(stream, ubx_frame(0x05, 0x00, 2));
    append(stream, flush());

    format::ubx::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
    CHECK(parser.frame_errors() > 0);
}

TEST_CASE("UBX parser - maximum declared length fits the frame buffer") {
    // The frame buffer must hold the largest accepted payload plus the 8 byte envelope. A
    // declared length of exactly the payload maximum used to be copied 8 bytes past the end of
    // an 8192 byte stack buffer.
    std::vector<uint8_t> stream;
    stream.push_back(0xB5);
    stream.push_back(0x62);
    stream.push_back(0x05);
    stream.push_back(0x01);
    stream.push_back(0x00);  // length = 0x2000 = 8192, the accepted maximum
    stream.push_back(0x20);
    for (size_t i = 0; i < 16384; i++) {
        stream.push_back(static_cast<uint8_t>(i));
    }
    append(stream, ubx_frame(0x05, 0x00, 2));
    append(stream, flush());

    format::ubx::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) >= 1);
}

TEST_CASE("UBX parser - oversized declared length is rejected as a false sync") {
    std::vector<uint8_t> stream;
    stream.push_back(0xB5);
    stream.push_back(0x62);
    stream.push_back(0x05);
    stream.push_back(0x01);
    stream.push_back(0xFF);  // length = 0xFFFF, far beyond what is accepted
    stream.push_back(0xFF);
    append(stream, ubx_frame(0x05, 0x00, 2));
    append(stream, flush());

    format::ubx::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
    CHECK(parser.frame_errors() > 0);
}

TEST_CASE("UBX parser - pure junk terminates and consumes everything") {
    auto stream = junk(4096);

    format::ubx::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 0);
    CHECK(parser.buffer_length() < 8);
}

//
// NMEA
//

TEST_CASE("NMEA parser - recovers both sentences across interleaved binary") {
    std::vector<uint8_t> stream;
    append_nmea(stream, "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");
    append(stream, rtcm_frame(1074, 107));
    append(stream, junk(64));
    append_nmea(stream, "GPGGA,123520,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");
    append(stream, flush());

    format::nmea::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 2);
}

TEST_CASE("NMEA parser - recovers after a bad checksum") {
    std::vector<uint8_t> stream;
    append(stream, "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*FF\r\n");
    append_nmea(stream, "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");

    format::nmea::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
    CHECK(parser.frame_errors() > 0);
}

TEST_CASE("NMEA parser - recovers after a truncated sentence") {
    std::vector<uint8_t> stream;
    append(stream, "$GPGGA,123519,4807.038,N,0113");
    append_nmea(stream, "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");

    format::nmea::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
}

TEST_CASE("NMEA parser - unterminated sentence makes forward progress") {
    std::vector<uint8_t> stream;
    stream.push_back('$');
    for (size_t i = 0; i < 8192; i++) {
        stream.push_back('A');
    }
    append(stream, "\r\n");
    append_nmea(stream, "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");

    format::nmea::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
}

TEST_CASE("NMEA parser - unsupported sentence does not stop the drain") {
    std::vector<uint8_t> stream;
    append_nmea(stream, "GPZDA,201530.00,04,07,2002,00,00");
    append_nmea(stream, "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,");

    format::nmea::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    // The unsupported sentence is reported, not swallowed, and does not end the drain.
    CHECK(drain(parser) == 2);
}

TEST_CASE("NMEA parser - payload and checksum are split correctly") {
    auto body     = std::string("GPGST,172814.0,0.006,0.023,0.020,273.6,0.023,0.020,0.031");
    auto sentence = nmea_sentence(body);

    SUBCASE("CRLF line ending") {
        format::nmea::Parser parser;
        REQUIRE(parser.append(reinterpret_cast<uint8_t const*>(sentence.data()), sentence.size()));
        auto message = parser.try_parse();
        REQUIRE(message != nullptr);
        CHECK(message->prefix() == "GPGST");
        // The last data character must not be lost, and the checksum must not carry the line
        // ending.
        CHECK(message->payload() == body.substr(body.find(',') + 1));
        CHECK(message->checksum().size() == 2);
        CHECK(message->sentence() == sentence);
    }

    SUBCASE("LF-only line ending") {
        auto                 lf_sentence = sentence.substr(0, sentence.size() - 2) + "\n";
        format::nmea::Parser parser{true};
        REQUIRE(parser.append(reinterpret_cast<uint8_t const*>(lf_sentence.data()),
                              lf_sentence.size()));
        auto message = parser.try_parse();
        REQUIRE(message != nullptr);
        CHECK(message->prefix() == "GPGST");
        CHECK(message->payload() == body.substr(body.find(',') + 1));
        CHECK(message->checksum().size() == 2);
        // sentence() always reconstructs with CRLF.
        CHECK(message->sentence() == sentence);
    }
}

TEST_CASE("NMEA parser - reconstructed sentence round-trips with a valid checksum") {
    char const* bodies[] = {
        "GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,",
        "GPVTG,054.7,T,034.4,M,005.5,N,010.2,K",
        "GPGST,172814.0,0.006,0.023,0.020,273.6,0.023,0.020,0.031",
        "GPZDA,201530.00,04,07,2002,00,00",
    };

    for (auto body : bodies) {
        auto                 expected = nmea_sentence(body);
        std::vector<uint8_t> stream;
        append_nmea(stream, body);

        format::nmea::Parser parser;
        REQUIRE(parser.append(stream.data(), stream.size()));
        auto message = parser.try_parse();
        REQUIRE(message != nullptr);

        // The reconstruction must be byte-identical to the input: leading '$', single
        // checksum, single line ending.
        auto rebuilt = message->sentence();
        CHECK(rebuilt == expected);

        // And it must independently verify.
        auto star = rebuilt.rfind('*');
        REQUIRE(star != std::string::npos);
        REQUIRE(rebuilt[0] == '$');
        unsigned computed = 0;
        for (size_t i = 1; i < star; i++) {
            computed ^= static_cast<unsigned char>(rebuilt[i]);
        }
        auto declared = std::stoul(rebuilt.substr(star + 1, 2), nullptr, 16);
        CHECK(computed == declared);
    }
}

//
// CTRL
//

TEST_CASE("CTRL parser - unknown command does not stop the drain") {
    std::vector<uint8_t> stream;
    append(stream, "/SOMETHING,1,2\r\n");
    append(stream, "/CID,L,240,1,2,3\r\n");

    format::ctrl::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
}

TEST_CASE("CTRL parser - malformed command does not stop the drain") {
    std::vector<uint8_t> stream;
    append(stream, "/CID,L,not-a-number,1,2,3\r\n");
    append(stream, "/CID,L,240,1,2,3\r\n");

    format::ctrl::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
}

TEST_CASE("CTRL parser - overlong line does not stop the drain") {
    std::vector<uint8_t> stream;
    stream.push_back('/');
    for (size_t i = 0; i < 400; i++) {
        stream.push_back('X');
    }
    append(stream, "\r\n/CID,L,240,1,2,3\r\n");

    format::ctrl::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    CHECK(drain(parser) == 1);
}

//
// AT
//

TEST_CASE("AT parser - overlong line does not stop line extraction") {
    std::vector<uint8_t> stream;
    for (size_t i = 0; i < 400; i++) {
        stream.push_back('X');
    }
    append(stream, "\r\nOK\r\n");

    format::at::Parser parser;
    REQUIRE(parser.append(stream.data(), stream.size()));
    parser.process();

    bool found_ok = false;
    while (parser.has_lines()) {
        if (parser.skip_line() == "OK") found_ok = true;
    }
    CHECK(found_ok);
}

//
// Buffer overflow reporting
//

TEST_CASE("Parser - buffer overflow is counted and reported") {
    // The ring buffer holds 128 KiB. Overrunning it destroys unparsed data, which must be
    // accounted for instead of silently losing messages.
    format::rtcm::Parser parser;

    std::vector<uint8_t> chunk(4096, 0x00);
    for (int i = 0; i < 64; i++) {
        REQUIRE(parser.append(chunk.data(), chunk.size()));
    }

    CHECK(parser.overflow_events() > 0);
    CHECK(parser.overflow_bytes() > 0);
}

TEST_CASE("Parser - no overflow when the drain loop keeps up") {
    format::rtcm::Parser parser;

    for (int i = 0; i < 64; i++) {
        std::vector<uint8_t> chunk;
        append(chunk, rtcm_frame(1074, 107));
        append(chunk, junk(128));
        append(chunk, rtcm_frame(1084, 53));
        REQUIRE(parser.append(chunk.data(), chunk.size()));
        drain(parser);
    }

    CHECK(parser.overflow_events() == 0);
    CHECK(parser.overflow_bytes() == 0);
}

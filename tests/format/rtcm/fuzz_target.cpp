#include <cstddef>
#include <cstdint>
#include <format/rtcm/message.hpp>
#include <format/rtcm/parser.hpp>

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    format::rtcm::Parser parser;
    parser.append(data, size);

    while (auto message = parser.try_parse()) {
        (void)message->type();
    }

    return 0;
}

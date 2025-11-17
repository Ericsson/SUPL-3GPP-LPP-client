#include <cstddef>
#include <cstdint>
#include <format/ubx/message.hpp>
#include <format/ubx/parser.hpp>

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    format::ubx::Parser parser;
    parser.append(data, size);

    while (auto message = parser.try_parse()) {
        (void)message->message_class();
        (void)message->message_id();
    }

    return 0;
}

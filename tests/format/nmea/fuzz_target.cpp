#include <format/nmea/parser.hpp>
#include <format/nmea/message.hpp>
#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    format::nmea::Parser parser;
    parser.append(data, size);
    
    while (auto message = parser.try_parse()) {
        (void)message->prefix();
    }
    
    return 0;
}

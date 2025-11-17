#include <cstddef>
#include <cstdint>
#include <format/at/parser.hpp>

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    format::at::Parser parser;
    parser.append(data, size);
    parser.process();

    while (parser.has_lines()) {
        (void)parser.skip_line();
    }

    return 0;
}

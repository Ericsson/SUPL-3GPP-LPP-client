#include <cstddef>
#include <cstdint>
#include <format/antex/antex.hpp>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    std::string input(reinterpret_cast<char const*>(data), size);
    auto        antex = format::antex::Antex::from_string(input);
    (void)antex;
    return 0;
}

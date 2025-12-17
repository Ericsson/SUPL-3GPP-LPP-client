#pragma once
#include <cstdint>
#include <vector>

struct RawMessage {
    std::vector<uint8_t> data;

    RawMessage() = default;
    explicit RawMessage(std::vector<uint8_t> d) : data(std::move(d)) {}
    RawMessage(uint8_t const* buf, size_t len) : data(buf, buf + len) {}

    RawMessage(RawMessage const&)            = default;
    RawMessage(RawMessage&&)                 = default;
    RawMessage& operator=(RawMessage const&) = default;
    RawMessage& operator=(RawMessage&&)      = default;
};

namespace streamline {
template <typename T>
struct TypeName;

template <>
struct TypeName<RawMessage> {
    static char const* name() { return "RawMessage"; }
};
}  // namespace streamline

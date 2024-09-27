#pragma once
#include <core/core.hpp>

#include <vector>

namespace supl {

struct Payload {
    enum class Type {
        NOTHING,
        TIA801,
        RRC,
        RRLP,
        LPP,
    };

    Type                 type;
    std::vector<uint8_t> data;
};

struct POS {
    std::vector<Payload> payloads;
};

}  // namespace supl

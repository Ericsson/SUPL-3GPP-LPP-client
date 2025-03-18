#pragma once
#include <core/core.hpp>

namespace lpp {

struct Version {
    int major;
    int technical;
    int editorial;
};

constexpr static auto VERSION_16_4_0 = Version{16, 4, 0};
constexpr static auto VERSION_18_4_0 = Version{18, 4, 0};

}  // namespace lpp

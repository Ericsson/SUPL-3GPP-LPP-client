#pragma once
#include <core/core.hpp>

namespace supl {

struct Version {
    int major;
    int minor;
    int servind;
};

constexpr static auto VERSION_2_0 = Version{2, 0, 0};
constexpr static auto VERSION_2_1 = Version{2, 1, 0};

}  // namespace supl

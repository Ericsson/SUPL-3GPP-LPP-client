#pragma once
#include <supl/types.hpp>

namespace supl {

struct Version {
    int major;
    int minor;
    int servind;
};

constexpr static auto VERSION_2_0 = Version{2, 0, 0};

}  // namespace supl

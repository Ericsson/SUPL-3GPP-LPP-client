#pragma once
#include <core/core.hpp>

namespace generator::tokoro {

struct Geoid {
    enum class Model {
        EMBEDDED,
    };

    static double height(double latitude, double longitude, Model model = Model::EMBEDDED);
};

}  // namespace generator::tokoro

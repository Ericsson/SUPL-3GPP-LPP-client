#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {

struct Geoid {
    enum class Model {
        EMBEDDED,
    };

    static double height(double latitude, double longitude, Model model = Model::EMBEDDED);
};

}  // namespace tokoro
}  // namespace generator

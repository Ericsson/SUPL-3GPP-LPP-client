#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {

struct Geoid {
    enum class Model {
        EMBEDDED,
    };

    /// @param latitude Latitude in radians.
    /// @param longitude Longitude in radians.
    static double height(double latitude, double longitude, Model model = Model::EMBEDDED);
};

}  // namespace tokoro
}  // namespace generator

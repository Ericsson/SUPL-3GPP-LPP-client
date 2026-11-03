#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "generator/tokoro/troposphere.hpp"

namespace generator {
namespace tokoro {

struct TroposphereSite {
    ts::Tai time;
    Float3  llh;           // geodetic latitude [rad], east longitude [rad], ellipsoidal height [m]
    double  geoid_height;  // [m]
};

struct ZenithDelays {
    double hydrostatic;
    double wet;
};

struct MappingValues {
    double hydrostatic;
    double wet;
};

// Modified julian date (UTC) of the time
NODISCARD double troposphere_mjd(ts::Tai const& time) NOEXCEPT;

// Scaling of the SSR tropospheric delays to the height of the station: the ratio of the MOPS
// delays at the station height to the MOPS delays at the ellipsoid. It is independent of the
// zenith delay model and the mapping function.
NODISCARD bool height_scaling(TroposphereSite const& site, double& hydrostatic,
                              double& wet) NOEXCEPT;

// Zenith delays [m] of the model at the site. Returns false if the model can not be evaluated
// (data that is needed is not loaded, or the site/time is outside of the model).
NODISCARD bool zenith_delays(TroposphereModel model, TroposphereData const* data,
                             TroposphereSite const& site, ZenithDelays& delays) NOEXCEPT;

// Mapping function values at the elevation [rad]. `apply_height_correction` enables the height
// correction of the hydrostatic mapping function. Returns false if the mapping function can not
// be evaluated.
NODISCARD bool mapping_values(MappingFunction function, TroposphereData const* data,
                              TroposphereSite const& site, double elevation,
                              bool apply_height_correction, MappingValues& values) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator

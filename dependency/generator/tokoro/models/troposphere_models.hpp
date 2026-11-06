#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "data/troposphere.hpp"
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

// Mapping function value m_g(el) of the gradient term. `hydrostatic_mapping` is m_h(el) of the
// selected mapping function, used by `GradientMapping::MacMillan`. Returns 0 at and below the
// horizon and at zenith.
NODISCARD double gradient_mapping_value(GradientMapping mapping, double elevation,
                                        double hydrostatic_mapping) NOEXCEPT;

// Gradient term of the slant delay [m]: m_g * (G_N * cos(az) + G_E * sin(az)). The azimuth is
// measured east from north [rad], the gradients are in metres.
NODISCARD double gradient_delay(double gradient_mapping, double azimuth, double north,
                                double east) NOEXCEPT;

// Gradient (G_N, G_E) [m] from the slope of the SSR zenith delays across the grid cell.
//
// A ray at elevation el has its delay centred at the height H of an exponential atmosphere, which
// puts the part of the ray that matters H * cot(el) away from the station along the azimuth. A
// horizontal slope of the zenith delay therefore gives the delay m(el) * H * cot(el) *
// (dZ/dN * cos(az) + dZ/dE * sin(az)), i.e. G = H * dZ/dx with a gradient mapping m_h * cot(el).
// The scale heights are those of the hydrostatic (8.47 km) and wet (2 km) delays used for the VMF3
// height reduction. The scaling to the height of the station is applied to the slope like it is
// applied to the zenith delays. This is an approximation of a real gradient parameter.
NODISCARD bool ssr_grid_gradient(TroposphericCorrection const& ssr, double hydrostatic_scale,
                                 double wet_scale, double& north, double& east) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator

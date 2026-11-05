#pragma once
#include <core/core.hpp>

#include <memory>

#include <gnss/satellite_id.hpp>
#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

// Transforms the broadcast orbits from the frame of each GNSS to a common target frame.
// Default broadcast frames: GPS WGS 84 (realization by GPS week), GLONASS PZ-90.11. Galileo
// (GTRF), BeiDou (CGCS2000) and QZSS (JGS) have no transformation in the coordinates library and
// are not transformed unless a frame is set with `set_source`.
class OrbitFrames {
public:
    // nullptr if the target frame is unknown
    NODISCARD static std::unique_ptr<OrbitFrames> create(char const* target) NOEXCEPT;
    ~OrbitFrames();

    // Override the broadcast frame of a GNSS. False if the frame is unknown or not connected to
    // the target frame.
    NODISCARD bool set_source(SatelliteId::Gnss gnss, char const* frame) NOEXCEPT;

    // False if the orbit of the GNSS is not transformed
    bool apply(SatelliteId::Gnss gnss, ts::Tai const& time, Float3& position,
               Float3& velocity) const NOEXCEPT;

    NODISCARD char const* target_name() const NOEXCEPT;
    // Broadcast frame used for the GNSS at the time, nullptr if none
    NODISCARD char const* source_name(SatelliteId::Gnss gnss, ts::Tai const& time) const NOEXCEPT;

    struct Impl;

private:
    OrbitFrames() NOEXCEPT;

    std::unique_ptr<Impl> mImpl;
};

}  // namespace tokoro
}  // namespace generator

#include "frame_transform.hpp"

#include <cstring>

#include <coordinates/transform_graph.hpp>
#include <generator/tokoro/generator.hpp>
#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(tokoro, frame);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, frame)

namespace generator {
namespace tokoro {

using coordinates::FrameId;

static constexpr int GNSS_COUNT = 6;

struct OrbitFrames::Impl {
    coordinates::TransformGraph graph;
    FrameId                     target;
    bool                        has_source[GNSS_COUNT];
    FrameId                     source[GNSS_COUNT];
    bool                        gps_by_week;

    bool connected(FrameId from) const {
        return from == target || !graph.find_path(from, 2020.0, target, 2020.0).empty();
    }
};

static int index(SatelliteId::Gnss gnss) {
    return static_cast<int>(gnss);
}

OrbitFrames::OrbitFrames() NOEXCEPT : mImpl(new Impl{}) {}
OrbitFrames::~OrbitFrames() = default;

std::unique_ptr<OrbitFrames> OrbitFrames::create(char const* target) NOEXCEPT {
    FrameId target_id{};
    if (!coordinates::frame_from_name(target, target_id)) {
        ERRORF("unknown frame \"%s\"", target);
        return nullptr;
    }

    std::unique_ptr<OrbitFrames> frames(new OrbitFrames());
    auto&                        impl = *frames->mImpl;
    impl.target                       = target_id;
    impl.gps_by_week                  = true;
    if (impl.connected(FrameId::PZ90_11)) {
        impl.has_source[index(SatelliteId::GLONASS)] = true;
        impl.source[index(SatelliteId::GLONASS)]     = FrameId::PZ90_11;
    } else {
        WARNF("GLONASS orbit not transformed: no transformation from pz-90.11 to %s", target);
    }
    return frames;
}

bool OrbitFrames::set_source(SatelliteId::Gnss gnss, char const* frame) NOEXCEPT {
    FrameId id{};
    if (!coordinates::frame_from_name(frame, id)) {
        ERRORF("unknown frame \"%s\"", frame);
        return false;
    }
    if (!mImpl->connected(id)) {
        ERRORF("no transformation from %s to %s", frame, coordinates::frame_name(mImpl->target));
        return false;
    }
    mImpl->has_source[index(gnss)] = true;
    mImpl->source[index(gnss)]     = id;
    if (gnss == SatelliteId::GPS) mImpl->gps_by_week = false;
    return true;
}

static bool source_frame(OrbitFrames::Impl const& impl, SatelliteId::Gnss gnss, ts::Tai const& time,
                         FrameId& source) {
    if (gnss == SatelliteId::GPS && impl.gps_by_week) {
        source = coordinates::frame_from_gps_week(static_cast<int>(ts::Gps(time).week()));
        return true;
    }
    if (!impl.has_source[index(gnss)]) return false;
    source = impl.source[index(gnss)];
    return true;
}

// Decimal year, accurate enough for the time-dependent Helmert parameters
static double decimal_year(ts::Tai const& time) {
    static auto const J2000 = ts::Utc::from_date_time(2000, 1, 1, 12, 0, 0.0).timestamp();
    auto              dt    = (ts::Utc(time).timestamp() - J2000).full_seconds();
    return 2000.0 + dt / (365.25 * 86400.0);
}

bool OrbitFrames::apply(SatelliteId::Gnss gnss, ts::Tai const& time, Float3& position,
                        Float3& velocity) const NOEXCEPT {
    FrameId source{};
    if (!source_frame(*mImpl, gnss, time, source)) return false;
    if (source == mImpl->target) return true;

    auto epoch  = decimal_year(time);
    auto result = mImpl->graph.transform(source, mImpl->target, epoch, epoch,
                                         {position.x, position.y, position.z},
                                         {velocity.x, velocity.y, velocity.z});
    if (result.steps.empty()) {
        VERBOSEF("no transformation from %s to %s", coordinates::frame_name(source),
                 coordinates::frame_name(mImpl->target));
        return false;
    }

    VERBOSEF("%s -> %s: %+.4f %+.4f %+.4f m", coordinates::frame_name(source),
             coordinates::frame_name(mImpl->target), result.final_position.x() - position.x,
             result.final_position.y() - position.y, result.final_position.z() - position.z);
    position =
        Float3{result.final_position.x(), result.final_position.y(), result.final_position.z()};
    velocity =
        Float3{result.final_velocity.x(), result.final_velocity.y(), result.final_velocity.z()};
    return true;
}

char const* OrbitFrames::target_name() const NOEXCEPT {
    return coordinates::frame_name(mImpl->target);
}

char const* OrbitFrames::source_name(SatelliteId::Gnss gnss, ts::Tai const& time) const NOEXCEPT {
    FrameId source{};
    if (!source_frame(*mImpl, gnss, time, source)) return nullptr;
    return coordinates::frame_name(source);
}

bool gnss_from_name(char const* name, SatelliteId::Gnss& gnss) NOEXCEPT {
    if (strcmp(name, "gps") == 0) {
        gnss = SatelliteId::GPS;
    } else if (strcmp(name, "glo") == 0) {
        gnss = SatelliteId::GLONASS;
    } else if (strcmp(name, "gal") == 0) {
        gnss = SatelliteId::GALILEO;
    } else if (strcmp(name, "bds") == 0) {
        gnss = SatelliteId::BEIDOU;
    } else if (strcmp(name, "qzs") == 0) {
        gnss = SatelliteId::QZSS;
    } else {
        return false;
    }
    return true;
}

}  // namespace tokoro
}  // namespace generator

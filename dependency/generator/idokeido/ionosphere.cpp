#include "correction.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#include "idw.hpp"

LOGLET_MODULE2(idokeido, iono);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, iono)

namespace idokeido {

Scalar IonosphericPolynomial::evaluate(Vector3 const& llh) const NOEXCEPT {
    FUNCTION_SCOPE();

    VERBOSEF("ionospheric polynomial:");
    auto lat = llh.x() * constant::r2d;
    auto lon = llh.y() * constant::r2d;
    VERBOSEF("  lat: %.14f", lat);
    VERBOSEF("  lon: %.14f", lon);

    auto ref_lat = reference_point.x() * constant::r2d;
    auto ref_lon = reference_point.y() * constant::r2d;
    VERBOSEF("  ref_lat: %.14f", ref_lat);
    VERBOSEF("  ref_lon: %.14f", ref_lon);

    auto delta_lat = lat - ref_lat;
    auto delta_lon = lon - ref_lon;
    VERBOSEF("  delta_lat: %.14f", delta_lat);
    VERBOSEF("  delta_lon: %.14f", delta_lon);

    VERBOSEF("  c00: %.14f", c00);
    VERBOSEF("  c01: %.14f", c01);
    VERBOSEF("  c10: %.14f", c10);
    VERBOSEF("  c11: %.14f", c11);

    auto r00 = c00;
    auto r01 = c01 * delta_lat;
    auto r10 = c10 * delta_lon;
    auto r11 = c11 * delta_lat * delta_lon;

    VERBOSEF("  r00: %.14f", r00);
    VERBOSEF("  r01: %.14f", r01);
    VERBOSEF("  r10: %.14f", r10);
    VERBOSEF("  r11: %.14f", r11);

    auto result = r00 + r01 + r10 + r11;
    VERBOSEF("  result: %.14f", result);

    return result;
}

bool CorrectionPointSet::ionospheric_residual(Vector3 const& llh, SatelliteId sv_id,
                                              Scalar& residual) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto ref = Vector3{llh.x() * constant::r2d, llh.y() * constant::r2d, 0};

    VERBOSEF("ionosphere grid:");

    std::vector<IdwPoint> residuals;
    residuals.reserve(point_count);
    for (size_t i = 0; i < point_count; ++i) {
        auto& point = points[i];
        if (!point.valid) continue;
        auto it = point.ionospheric_residual.find(sv_id);
        if (it == point.ionospheric_residual.end()) continue;

        auto position =
            Vector3{point.position.x() * constant::r2d, point.position.y() * constant::r2d, 0};
        auto residual = it->second;
        residuals.push_back({
            .position = position,
            .residual = residual,
            .distance = (ref - position).norm(),
        });
    }

    if (residuals.empty()) {
        VERBOSEF("  no valid points");
        return false;
    }

    // Sort by distance
    std::sort(residuals.begin(), residuals.end(), [](IdwPoint const& a, IdwPoint const& b) {
        return a.distance < b.distance;
    });

#if !defined(DISALBE_VERBOSE)
    for (auto const& r : residuals) {
        VERBOSEF("  %+3.14f %+3.14f: %+.14f", r.position.x(), r.position.y(), r.residual);
    }
#endif

    // We don't want to use more than 4 points
    if (residuals.size() > 4) residuals.resize(4);

    auto w   = inverse_distance_weight(ref, residuals);
    residual = w;
    return true;
}

bool CorrectionPointSet::ionospheric_polynomial(Vector3 const& llh, SatelliteId sv_id,
                                                Scalar& result) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto it = ionospheric_polynomials.find(sv_id);
    if (it == ionospheric_polynomials.end()) return false;

    result = it->second.evaluate(llh);
    return true;
}

}  // namespace idokeido

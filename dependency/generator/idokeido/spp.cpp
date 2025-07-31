#include "spp.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, spp);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, spp)

namespace idokeido {

void EphemerisEngine::add(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gps_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GPS %d", ephemeris.prn);
        return;
    }

    auto& list = mGpsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GpsEphemeris const& a, ephemeris::GpsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

void EphemerisEngine::add(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gal_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GAL %d", ephemeris.prn);
        return;
    }

    auto& list = mGalEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GalEphemeris const& a, ephemeris::GalEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

void EphemerisEngine::add(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_bds_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: BDS %d", ephemeris.prn);
        return;
    }

    auto& list = mBdsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::BdsEphemeris const& a, ephemeris::BdsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

ephemeris::GpsEphemeris const* EphemerisEngine::find_gps(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mGpsEphemeris.find(satellite_id);
    if (it == mGpsEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto gps_time = ts::Gps(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                 ephemeris.is_valid(gps_time) ? " [time]" : "");
        if (!ephemeris.is_valid(gps_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

ephemeris::GalEphemeris const* EphemerisEngine::find_gal(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mGalEphemeris.find(satellite_id);
    if (it == mGalEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto gal_time = ts::Gst(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iod_nav,
                 ephemeris.is_valid(gal_time) ? " [time]" : "");
        if (!ephemeris.is_valid(gal_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

ephemeris::BdsEphemeris const* EphemerisEngine::find_bds(SatelliteId    satellite_id,
                                                         ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = mBdsEphemeris.find(satellite_id);
    if (it == mBdsEphemeris.end()) return nullptr;
    auto& list = it->second;

    auto bds_time = ts::Bdt(time);
    VERBOSEF("searching: %s", satellite_id.name());
    for (auto& ephemeris : list) {
        VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s", ephemeris.week_number, ephemeris.toe,
                 ephemeris.toc, ephemeris.lpp_iod, ephemeris.iodc,
                 ephemeris.is_valid(bds_time) ? " [time]" : "");
        if (!ephemeris.is_valid(bds_time)) continue;
        VERBOSEF("found: %s", satellite_id.name());
        return &ephemeris;
    }

    return nullptr;
}

EphemerisEngine::Satellite EphemerisEngine::evaluate(SatelliteId    satellite_id,
                                                     ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (satellite_id.is_gps()) {
        auto eph = find_gps(satellite_id, time);
        if (eph) return evaluate_gps(satellite_id, time, *eph);
    }
    if (satellite_id.is_galileo()) {
        auto eph = find_gal(satellite_id, time);
        if (eph) return evaluate_gal(satellite_id, time, *eph);
    }
    if (satellite_id.is_beidou()) {
        auto eph = find_bds(satellite_id, time);
        if (eph) return evaluate_bds(satellite_id, time, *eph);
    }

    return {};
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_gps(SatelliteId satellite_id, ts::Tai const& time,
                              ephemeris::GpsEphemeris const& eph) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto gps_time = ts::Gps(time);
    auto result   = eph.compute(gps_time);

    return {
        .id       = satellite_id,
        .position = {result.position.x, result.position.y, result.position.z},
        .velocity = {result.velocity.x, result.velocity.y, result.velocity.z},
        .clock    = result.clock,
    };
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_gal(SatelliteId satellite_id, ts::Tai const& time,
                              ephemeris::GalEphemeris const& eph) const NOEXCEPT {
    FUNCTION_SCOPE();
    TODOF("implement EphemerisEngine::evaluate_gal()");
    return {};
}

EphemerisEngine::Satellite
EphemerisEngine::evaluate_bds(SatelliteId satellite_id, ts::Tai const& time,
                              ephemeris::BdsEphemeris const& eph) const NOEXCEPT {
    FUNCTION_SCOPE();
    TODOF("implement EphemerisEngine::evaluate_gal()");
    return {};
}

SppEngine::SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine) NOEXCEPT
    : mConfiguration(std::move(configuration)),
      mEphemerisEngine(ephemeris_engine) {
    FUNCTION_SCOPE();
    DEBUGF("idokeido single-point positoning");
}

SppEngine::~SppEngine() {
    FUNCTION_SCOPE();
}

static long absolute_signal_id(SatelliteId satellite_id, SignalId signal_id) {
    auto sid = satellite_id.absolute_id();
    if (sid < 0) return -1;
    auto oid = signal_id.absolute_id();
    if (oid < 0) return -1;
    return sid * SIGNAL_ABS_COUNT + oid;
}

void SppEngine::observation(RawObservation const& raw) NOEXCEPT {
    FUNCTION_SCOPE();

    auto satellite_id = raw.satellite_id.absolute_id();
    auto signal_id    = absolute_signal_id(raw.satellite_id, raw.signal_id);
    if (satellite_id < 0 || signal_id < 0) {
        WARNF("invalid satellite or signal id: %s %s", raw.satellite_id.name(),
              raw.signal_id.name());
        return;
    }

    // Mark that the satellite is active
    mSatelliteMask[satellite_id]      = true;
    mSatelliteStates[satellite_id].id = raw.satellite_id;

    // Add the observation
    mObservationMask[signal_id]   = true;
    mObservationStates[signal_id] = Observation{
        .time          = raw.time,
        .satellite_id  = raw.satellite_id,
        .signal_id     = raw.signal_id,
        .pseudo_range  = raw.pseudo_range,
        .carrier_phase = raw.carrier_phase,
        .doppler       = raw.doppler,
        .snr           = raw.snr,
        .lock_time     = raw.lock_time,
    };

    VERBOSEF("new observation: %03ld:%04ld %s %s", satellite_id, signal_id, raw.satellite_id.name(),
             raw.signal_id.name());
}

template <typename T>
static std::string epf(T const& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

void SppEngine::select_best_observations() {
    FUNCTION_SCOPE();

    DEBUGF("selecting best observation for each satellite");
    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;

        auto& satellite    = mSatelliteStates[i];
        auto  satellite_id = satellite.id.absolute_id();
        ASSERT(satellite_id >= 0, "invalid satellite id");

        long   best_id  = -1;
        double best_snr = 0.0;

        for (long j = satellite_id * SIGNAL_ABS_COUNT; j < (satellite_id + 1) * SIGNAL_ABS_COUNT;
             ++j) {
            if (!mObservationMask[j]) continue;
            auto& observation = mObservationStates[j];
            // TODO: reject observations with too high SNR
            // TODO: reject observations with cycle slip
            // TODO: reject stale observations
            if (observation.snr > best_snr) {
                best_id  = j;
                best_snr = observation.snr;
            }
        }

        if (best_id >= 0) {
            satellite.main_observation_id = best_id;
        } else {
            mSatelliteMask[i]             = false;
            satellite.main_observation_id = -1;
            WARNF("no observation for satellite %03ld %s", satellite_id, satellite.id.name());
            continue;
        }

        // TODO: find secondary observation with highest SNR for ionospheric 1-order estimation

        DEBUGF("  %03ld %s: %04ld", satellite_id, satellite.id.name(),
               satellite.main_observation_id);
    }
}

void SppEngine::compute_satellite_states() {
    FUNCTION_SCOPE();

    DEBUGF("computing satellite states");
    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;

        auto& satellite = mSatelliteStates[i];
        ASSERT(satellite.main_observation_id >= 0, "no main observation for satellite");
        ASSERT(mObservationMask[satellite.main_observation_id], "main observation not found");
        auto& observation = mObservationStates[satellite.main_observation_id];
        auto  state       = mEphemerisEngine.evaluate(satellite.id, observation.time);
        if (!state.is_valid()) {
            mSatelliteMask[i] = false;
            WARNF("invalid state for satellite %03ld %s", satellite.id.absolute_id(),
                  satellite.id.name());
            continue;
        }

        satellite.position = state.position;

        DEBUGF("  %03ld %s: %s", satellite.id.absolute_id(), satellite.id.name(),
               epf(satellite.position).c_str());
    }
}

void SppEngine::evaluate() NOEXCEPT {
    FUNCTION_SCOPE();
    DEBUGF("evaluating %zu satellites and %zu observations", mSatelliteMask.count(),
           mObservationMask.count());

    select_best_observations();
    compute_satellite_states();

    // We can only solve if we have enough satellites
    auto satellite_count = mSatelliteMask.count();
    if (satellite_count < 4) {
        WARNF("not enough satellites: %d < 4", satellite_count);
        return;
    }

    // Initial guess position: (0,0,0), receiver bias: 0, this will _always_ converge to the
    // correct solution (if possible)
    Eigen::Vector4d current = {0, 0, 0, 0};

    for (size_t it = 0; it < 10; ++it) {
        DEBUGF("iteration %d: %s", it, epf(current).c_str());
        // Compute the geometric range to the guess
        Eigen::MatrixXd residuals{satellite_count, 1};
        Eigen::MatrixXd design_matrix{satellite_count, 4};
        for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
            if (!mSatelliteMask[i]) continue;

            auto& satellite   = mSatelliteStates[i];
            auto& observation = mObservationStates[satellite.main_observation_id];

            auto delta_position  = satellite.position - current.head<3>();
            auto geometric_range = delta_position.norm();
            auto line_of_sight   = delta_position.normalized();

            design_matrix(i, 0) = line_of_sight.x();
            design_matrix(i, 1) = line_of_sight.y();
            design_matrix(i, 2) = line_of_sight.z();
            design_matrix(i, 3) = 1;

            auto residual   = geometric_range - observation.pseudo_range;
            residuals(i, 0) = residual;

            DEBUGF("%14.4f %14.4f %14.4f %14.4f | %14.4f", line_of_sight.x(), line_of_sight.y(),
                   line_of_sight.z(), 1, residual);
        }

        DEBUGF("H: %s", epf(design_matrix).c_str());
        DEBUGF("R: %s", epf(residuals).c_str());

        // Compute the solution
        auto dTd      = design_matrix.transpose() * design_matrix;
        auto dTr      = design_matrix.transpose() * residuals;
        auto solution = dTd.ldlt().solve(dTr);

        DEBUGF("solution: %s (norm: %f)", epf(solution).c_str(), solution.norm());
        if (solution.norm() < 0.01) {
            break;
        }

        current += solution;
    }

    auto final_position = current.head<3>();
    auto final_bias     = current(3);
    DEBUGF("final position: %14.4f %14.4f %14.4f", final_position.x(), final_position.y(),
           final_position.z());
    DEBUGF("final bias:     %14.4f", final_bias);
}

}  // namespace idokeido

#include "spp.hpp"
#include "eph.hpp"

#include <generator/tokoro/coordinate.hpp>
#include <loglet/loglet.hpp>

#include "coordinates/ecef.hpp"
#include "coordinates/enu.hpp"
#include "coordinates/look_angles.hpp"

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#include <iomanip>

LOGLET_MODULE2(idokeido, spp);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, spp)

namespace idokeido {

SppEngine::SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine) NOEXCEPT
    : mConfiguration(std::move(configuration)),
      mEphemerisEngine(ephemeris_engine) {
    FUNCTION_SCOPE();
    DEBUGF("idokeido single-point positoning");

    mFirstObservationId        = -1;
    mLastObservationId         = -1;
    mEpochObservationCount     = 0;
    mEpochTotalObservationTime = 0;

    mKlobucharModelSet = false;
}

SppEngine::~SppEngine() {
    FUNCTION_SCOPE();
}

void SppEngine::klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT {
    FUNCTION_SCOPE();
    mKlobucharModelSet = true;
    mKlobucharModel    = parameters;
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

    if (mFirstObservationId == -1) {
        mFirstObservationId = signal_id;
    }
    if (mLastObservationId == -1) {
        mLastObservationId = signal_id;
    }
    mEpochObservationCount += 1;
    mEpochTotalObservationTime += raw.time.timestamp().full_seconds();

    VERBOSEF("new observation: %03ld:%04ld %s %s", satellite_id, signal_id, raw.satellite_id.name(),
             raw.signal_id.name());
}

template <typename T>
static std::string epf(T const& t) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << t;
    return ss.str();
}

template <typename T>
static std::string epft(T const& t) {
    return epf(t.transpose());
}

void SppEngine::select_best_observations(ts::Tai const& time) {
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

            auto time_diff = time.difference_seconds(observation.time);
            if (time_diff < -mConfiguration.observation_window * 0.5 ||
                time_diff > mConfiguration.observation_window * 0.5) {
                WARNF("observation time out of window: %03ld:%04ld %s %s: %s %s", satellite_id, j,
                      satellite.id.name(), observation.signal_id.name(),
                      time.rtklib_time_string().c_str(),
                      observation.time.rtklib_time_string().c_str());
                mObservationMask[j] = false;
                continue;
            }

            if (observation.snr < mConfiguration.snr_cutoff) {
                WARNF("snr cutoff: %03ld:%04ld %s %s: %f < %f", satellite_id, j,
                      satellite.id.name(), observation.signal_id.name(), observation.snr,
                      mConfiguration.snr_cutoff);
                mObservationMask[j] = false;
                continue;
            }

            // TODO(ewasjon): REMOVE
            if (observation.signal_id != SignalId::GPS_L1_CA) {
                NOTICEF("ignoring %s", observation.signal_id.name());
                mObservationMask[j] = false;
                continue;
            }

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

void SppEngine::compute_satellite_states(ts::Tai const& time) {
    FUNCTION_SCOPE();

    DEBUGF("computing satellite states");
    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;

        auto& satellite = mSatelliteStates[i];
        auto  state     = mEphemerisEngine.evaluate(satellite.id, time);
        if (!state.is_valid()) {
            mSatelliteMask[i] = false;
            WARNF("invalid state for satellite %03ld %s", satellite.id.absolute_id(),
                  satellite.id.name());
            continue;
        }

        satellite.position = state.position;

        DEBUGF("  %03ld %s: %14.8f %14.8f %14.8f", satellite.id.absolute_id(), satellite.id.name(),
               satellite.position.x(), satellite.position.y(), satellite.position.z());
    }
}

static constexpr double SPEED_OF_LIGHT = 2.99792458e8;

static double geometric_distance(Vector3 const& a, Vector3 const& b) {
    auto delta    = a - b;
    auto distance = delta.norm();

    // correct for rotation ECEF
    auto dot_omega_e = 7.2921151467e-5;
    auto correction  = dot_omega_e * (a.x() * b.y() - a.y() * b.x()) / SPEED_OF_LIGHT;

    return distance + correction;
}

void SppEngine::compute_satellite_state(Satellite& satellite, ts::Tai const& reception_time,
                                        Vector3 const& ground_position) {
    FUNCTION_SCOPE();

    // initial guess is that emission = reception
    auto t_r = reception_time;
    auto t_e = t_r;

    // because the t_e can never equal t_r, the initial guess can be shifted by a small amount
    // (e.g. -0.08 seconds) to help find the correct emission time faster (this is was RTKLIB/CLAS
    // uses)
    t_e = t_e + ts::Timestamp{-0.08};

    for (auto i = 0; i < 10; i++) {
        VERBOSEF("iteration %i: %+f us  %s (GPS %.16f)", i,
                 t_e.difference(t_r).full_seconds() * 1000000.0, t_e.rtklib_time_string().c_str(),
                 ts::Gps{t_e}.time_of_week().full_seconds());

        // ephemeral position at t_e
        auto result = mEphemerisEngine.evaluate(satellite.id, t_e);
        VERBOSEF("    x=%f, y=%f, z=%f", result.position.x(), result.position.y(),
                 result.position.z());
        VERBOSEF("    dx=%f, dy=%f, dz=%f", result.velocity.x(), result.velocity.y(),
                 result.velocity.z());
        VERBOSEF("    clock_bias=%f", result.clock);

        // compute the pseudo-range (this is not the true range, as it contains the satellite orbit
        // error and the satellite clock error)
        auto pseudorange = geometric_distance(result.position, ground_position);
        auto travel_time = pseudorange / constant::c;
        VERBOSEF("    range=%f, time=%f", i, pseudorange, travel_time);

        // the new emission time is the reception time minus the travel time
        auto new_t_e   = t_r + ts::Timestamp{-travel_time};
        auto delta_t_e = new_t_e.difference(t_e);

        t_e = new_t_e;

        // check if the delta has converged
        if (std::abs(delta_t_e.full_seconds()) < 1.0e-12) {
            break;
        }
    }

    auto final_result                 = mEphemerisEngine.evaluate(satellite.id, t_e);
    satellite.position                = final_result.position;
    satellite.velocity                = final_result.velocity;
    satellite.clock                   = final_result.clock;
    satellite.group_delay             = final_result.group_delay;
    satellite.relativistic_correction = final_result.relativistic_correction;

    auto delta_position       = satellite.position - ground_position;
    satellite.geometric_range = delta_position.norm();
    satellite.line_of_sight   = delta_position.normalized();

    auto travel_time            = satellite.geometric_range / constant::c;
    satellite.reception_time    = t_r;
    satellite.transmission_time = t_r + ts::Timestamp{-travel_time};
    auto calculated_time        = satellite.transmission_time + ts::Timestamp{travel_time};

    auto ground_llh = ecef_to_llh(ground_position, ellipsoid::WGS84);
    auto enu        = ecef_to_enu_at_llh(ground_llh, satellite.line_of_sight);

    LookAngles look_angles{};
    if (!compute_look_angles(ground_position, enu, satellite.position, look_angles)) {
        WARNF("failed to compute look angles");
    }

    satellite.azimuth   = look_angles.azimuth;
    satellite.elevation = look_angles.elevation;
    satellite.nadir     = look_angles.nadir;

    DEBUGF("satellite state: %s", satellite.id.name());
    DEBUGF("  position:       (%.14f, %.14f, %.14f)", satellite.position.x(),
           satellite.position.y(), satellite.position.z());
    DEBUGF("  velocity:       (%.14f, %.14f, %.14f)", satellite.velocity.x(),
           satellite.velocity.y(), satellite.velocity.z());
    DEBUGF("  line of sight:  (%.14f, %.14f, %.14f)", satellite.line_of_sight.x(),
           satellite.line_of_sight.y(), satellite.line_of_sight.z());
    DEBUGF("  clock bias:      %.14f", final_result.clock);
    DEBUGF("  group delay:     %.14f", final_result.group_delay);
    DEBUGF("  relativistic:    %.14f", final_result.relativistic_correction);
    DEBUGF("  geometric range: %.14f", satellite.geometric_range);
    DEBUGF("  t_e:               %s (GPS %.16f)", t_e.rtklib_time_string().c_str(),
           ts::Gps{t_e}.time_of_week().full_seconds());
           DEBUGF("  transmission time: %s (GPS %.16f)", t_e.rtklib_time_string().c_str(),
           ts::Gps{t_e}.time_of_week().full_seconds());
           DEBUGF("  reception time:    %s (GPS %.16f)", t_r.rtklib_time_string().c_str(),
                  ts::Gps{t_r}.time_of_week().full_seconds());
    DEBUGF("  calculated time:   %s (GPS %.16f)", calculated_time.rtklib_time_string().c_str(),
           ts::Gps{calculated_time}.time_of_week().full_seconds());
    DEBUGF("  elevation:         %.14f", satellite.elevation * constant::r2d);
    DEBUGF("  azimuth:           %.14f", satellite.azimuth * constant::r2d);
    DEBUGF("  nadir:             %.14f", satellite.nadir * constant::r2d);
}

Solution SppEngine::evaluate() NOEXCEPT {
    FUNCTION_SCOPE();

    Solution solution{};
    if (mConfiguration.epoch_selection == SppConfiguration::EpochSelection::FirstObservation) {
        if (mFirstObservationId >= 0) {
            solution = evaluate(mObservationStates[mFirstObservationId].time);
        } else {
            WARNF("epoch selection: first observation not found");
        }
    } else if (mConfiguration.epoch_selection ==
               SppConfiguration::EpochSelection::LastObservation) {
        if (mLastObservationId >= 0) {
            solution = evaluate(mObservationStates[mLastObservationId].time);
        } else {
            WARNF("epoch selection: last observation not found");
        }
    } else if (mConfiguration.epoch_selection ==
               SppConfiguration::EpochSelection::MeanObservation) {
        if (mEpochObservationCount > 0) {
            auto mean_time =
                ts::Tai{ts::Timestamp{mEpochTotalObservationTime / mEpochObservationCount}};
            solution = evaluate(mean_time);
        } else {
            WARNF("epoch selection: mean observation not found");
        }
    } else {
        ASSERT(false, "invalid epoch selection");
    }

    return solution;
}

Solution SppEngine::evaluate(ts::Tai time) NOEXCEPT {
    FUNCTION_SCOPEF("%s", time.rtklib_time_string().c_str());
    DEBUGF("evaluate: epoch %s", time.rtklib_time_string().c_str());
    DEBUGF("evaluating %zu satellites and %zu observations", mSatelliteMask.count(),
           mObservationMask.count());

    select_best_observations(time);
    compute_satellite_states(time);

    // We can only solve if we have enough satellites
    auto satellite_count = mSatelliteMask.count();
    if (satellite_count < 4) {
        WARNF("not enough satellites: %d < 4", satellite_count);
        return Solution{};
    }

    DEBUGF("satellite available: %zu", satellite_count);

    // Initial guess position: (0,0,0), receiver bias: 0, this will _always_ converge to the
    // correct solution (if possible)
    Vector4 current = {0, 0, 0, 0};
    DEBUGF("initial guess: %14.8f %14.8f %14.8f %14.8f", current(0), current(1), current(2),
           current(3));

    MatrixX residuals{satellite_count, 1};
    MatrixX design_matrix{satellite_count, 4};

    for (size_t it = 0; it < 3; ++it) {
        DEBUGF("iteration %d: %14.8f %14.8f %14.8f %14.8f", it, current(0), current(1), current(2),
               current(3));
        // Compute the geometric range to the guess

        auto ground_position = current.head<3>();
        auto ground_llh      = ecef_to_llh(ground_position, ellipsoid::WGS84);

        size_t j = 0;
        for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
            if (!mSatelliteMask[i]) continue;

            auto& satellite   = mSatelliteStates[i];
            auto& observation = mObservationStates[satellite.main_observation_id];

            compute_satellite_state(satellite, time, ground_position);

            if (satellite.elevation * constant::r2d < mConfiguration.elevation_cutoff) {
                WARNF("reject: %03ld %s: %.2fdeg < %.2fdeg", satellite.id.absolute_id(),
                      satellite.id.name(), satellite.elevation * constant::r2d,
                      mConfiguration.elevation_cutoff);
                continue;
            }

            auto i_delay = 0.0;
            switch (mConfiguration.ionospheric_mode) {
            case SppConfiguration::IonosphericMode::None: break;
            case SppConfiguration::IonosphericMode::Navigation:
                if (mKlobucharModelSet) {
                    i_delay = mKlobucharModel.evaluate(time, satellite.elevation, satellite.azimuth,
                                                       ground_llh);
                } else {
                    WARNF("ionospheric model not available");
                }
                break;
            case SppConfiguration::IonosphericMode::Dual:
                TODOF("implement dual ionospheric model");
                break;
            }

            auto f_mhz  = observation.signal_id.frequency();
            auto i_bias = 40.3e13 * i_delay / (f_mhz * f_mhz);
            auto t_bias = 0.0;

            auto receiver_clock_bias     = constant::c * current(3);
            auto group_delay_bias        = constant::c * satellite.group_delay;
            auto relativistic_clock_bias = constant::c * satellite.relativistic_correction;
            auto satellite_clock_bias    = constant::c * satellite.clock;

            auto epoch_time_delta = time.difference_seconds(observation.time);
            auto pseudo_range_adj = epoch_time_delta * constant::c;
            auto pseudo_range     = satellite.geometric_range + receiver_clock_bias -
                                relativistic_clock_bias + group_delay_bias - satellite_clock_bias +
                                i_bias + t_bias + pseudo_range_adj;

            auto residual = pseudo_range - observation.pseudo_range;
            DEBUGF("%14.4f + [%14.4f + %14.4f - %14.4f] - %14.4f + %14.4f + %14.4f + %14.4f|%14.4f "
                   "- %14.4f "
                   "= %14.4fm|%5.2fdeg %5.2fdeg "
                   "%.14fSTEC| "
                   "%s %s",
                   satellite.geometric_range, receiver_clock_bias, relativistic_clock_bias,
                   group_delay_bias, satellite_clock_bias, i_bias, t_bias, pseudo_range_adj,
                   pseudo_range, observation.pseudo_range, residual,
                   satellite.elevation * constant::r2d, satellite.azimuth * constant::r2d, i_delay,
                   satellite.id.name(), observation.signal_id.name());

            design_matrix(j, 0) = satellite.line_of_sight.x();
            design_matrix(j, 1) = satellite.line_of_sight.y();
            design_matrix(j, 2) = satellite.line_of_sight.z();
            design_matrix(j, 3) = constant::c;

            residuals(j, 0) = residual;

            j++;
        }

        auto h_subset = design_matrix.topRows(j).eval();
        auto r_subset = residuals.topRows(j).eval();

        DEBUGF("H:\n%s", epf(h_subset).c_str());
        DEBUGF("R:\n%s", epf(r_subset).c_str());

        auto h_transpose = h_subset.transpose();

        // Compute the solution
        auto dTd      = h_transpose * h_subset;
        auto dTr      = h_transpose * r_subset;
        auto solution = dTd.ldlt().solve(dTr).eval();
        DEBUGF("solution: %14.8f %14.8f %14.8f %14.8f (norm %14.8f)", solution(0), solution(1),
               solution(2), solution(3), solution.norm());
        current += solution;

        // Compute per satellite residuals
        DEBUGF("residuals:");
        j = 0;
        for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
            if (!mSatelliteMask[i]) continue;

            auto residual = design_matrix.row(j).dot(current) - residuals(j);
            DEBUGF("  %3s: %14.8f", mSatelliteStates[i].id.name(), residual);
            j++;
        }

        // Compute convergence
        auto q   = dTd.inverse();
        auto qxx = q(0, 0);
        auto qyy = q(1, 1);
        auto qzz = q(2, 2);

        auto pod = std::sqrt(qxx + qyy + qzz);
        auto hod = std::sqrt(qxx + qyy);
        auto vod = std::sqrt(qzz);

        DEBUGF("pod: %14.8f", pod);
        DEBUGF("hod: %14.8f", hod);
        DEBUGF("vod: %14.8f", vod);

        if (solution.norm() < 0.001) {
            break;
        }
    }

    datatrace_report();

    auto final_position = current.head<3>();
    auto final_bias     = current(3);
    DEBUGF("final position: %14.4f %14.4f %14.4f", final_position.x(), final_position.y(),
           final_position.z());
    DEBUGF("final bias:     %14.4f", final_bias);

    auto llh = generator::tokoro::ecef_to_llh(
        Float3{final_position.x(), final_position.y(), final_position.z()},
        generator::tokoro::ellipsoid::WGS84);

    DEBUGF("llh: %14.4f %14.4f %14.4f", llh.x * (180.0 / M_PI), llh.y * (180.0 / M_PI), llh.z);

    DEBUGF("bias: %14.4f", final_bias);
    DEBUGF("time: %s", time.rtklib_time_string().c_str());

    Solution solution{
        .time      = time,
        .status    = Solution::Status::Standard,
        .latitude  = llh.x * (180.0 / M_PI),
        .longitude = llh.y * (180.0 / M_PI),
        .altitude  = llh.z,
    };

    mFirstObservationId        = -1;
    mLastObservationId         = -1;
    mEpochObservationCount     = 0;
    mEpochTotalObservationTime = 0;

    return solution;
}

void SppEngine::datatrace_report() NOEXCEPT {
    VSCOPE_FUNCTION();
#ifdef DATA_TRACING
    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;
        auto&                satellite = mSatelliteStates[i];
        datatrace::Satellite dt_sat{};
        dt_sat.position =
            Float3{satellite.position.x(), satellite.position.y(), satellite.position.z()};
        dt_sat.elevation = satellite.elevation * constant::r2d;
        dt_sat.azimuth   = satellite.azimuth * constant::r2d;
        dt_sat.nadir     = satellite.nadir * constant::r2d;
        datatrace::report_satellite(satellite.reception_time, satellite.id.name(), dt_sat);
    }
#endif
}

}  // namespace idokeido

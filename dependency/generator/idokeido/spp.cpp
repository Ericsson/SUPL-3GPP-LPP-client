#include "spp.hpp"
#include "eph.hpp"

#include <cmath>
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
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, spp)

namespace idokeido {

SppEngine::SppEngine(SppConfiguration configuration, EphemerisEngine& ephemeris_engine,
                     CorrectionCache& correction_cache) NOEXCEPT
    : mConfiguration(std::move(configuration)),
      mEphemerisEngine(ephemeris_engine),
      mCorrectionCache(correction_cache) {
    FUNCTION_SCOPE();
    DEBUGF("idokeido single-point positioning");

    mEpochFirstTimeSet         = false;
    mEpochLastTimeSet          = false;
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

void SppEngine::add_measurement(RawMeasurement const& raw) NOEXCEPT {
    FUNCTION_SCOPEF("%s %s", raw.satellite_id.name(), raw.signal_id.name());

    if (!mConfiguration.gnss.gps && raw.satellite_id.is_gps()) return;
    if (!mConfiguration.gnss.glo && raw.satellite_id.is_glonass()) return;
    if (!mConfiguration.gnss.gal && raw.satellite_id.is_galileo()) return;
    if (!mConfiguration.gnss.bds && raw.satellite_id.is_beidou()) return;

    auto satellite_id = raw.satellite_id.absolute_id();
    auto signal_id    = absolute_signal_id(raw.satellite_id, raw.signal_id);
    if (satellite_id < 0 || signal_id < 0) {
        WARNF("invalid satellite or signal id: %s %s", raw.satellite_id.name(),
              raw.signal_id.name());
        return;
    }

    // Mark that the satellite is active
    auto sat_idx              = static_cast<size_t>(satellite_id);
    mObservationMask[sat_idx] = true;

    auto& observation        = mObservations[sat_idx];
    observation.satellite_id = raw.satellite_id;
    observation.add_measurement(Measurment{
        .time          = raw.time,
        .signal_id     = raw.signal_id,
        .pseudo_range  = raw.pseudo_range,
        .carrier_phase = raw.carrier_phase,
        .doppler       = raw.doppler,
        .snr           = raw.snr,
        .lock_time     = raw.lock_time,
    });

    if (!mEpochFirstTimeSet) {
        mEpochFirstTimeSet = true;
        mEpochFirstTime    = raw.time;
    }
    mEpochLastTimeSet = true;
    mEpochLastTime    = raw.time;
    mEpochObservationCount += 1;
    mEpochTotalObservationTime += raw.time.timestamp().full_seconds();
}

template <typename T>
static std::string epf(T const& t) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << t;
    return ss.str();
}

void SppEngine::select_best_observations(ts::Tai const& time) {
    FUNCTION_SCOPE();

    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;
        auto& observation = mObservations[i];
        VERBOSEF("%03ld %s: %d measurments", observation.satellite_id.absolute_id(),
                 observation.satellite_id.name(), observation.measurement_count);
        for (size_t j = 0; j < observation.measurement_mask.size(); ++j) {
            if (!observation.measurement_mask[j]) continue;
            auto& measurement = observation.measurements[j];
            VERBOSEF("  %12s: %s %+16.4f", measurement.signal_id.name(),
                     measurement.time.rtklib_time_string().c_str(), measurement.pseudo_range);
        }
    }

    std::bitset<SIGNAL_ABS_COUNT> measurment_skip;

    DEBUGF("selecting best observation for each satellite");
    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& observation = mObservations[i];
        if (observation.measurement_count == 0) {
            WARNF("reject: %03ld %s: no measurement", observation.satellite_id.absolute_id(),
                  observation.satellite_id.name());
            mObservationMask[i] = false;
            continue;
        }

        // Reset the observation
        observation.selected0 = -1;
        observation.selected1 = -1;
        measurment_skip.reset();

        // Discard measurement that is out of observation window
        for (size_t j = 0; j < observation.measurement_mask.size(); ++j) {
            if (!observation.measurement_mask[j]) continue;

            auto& measurement = observation.measurements[j];
            auto  time_diff   = time.difference_seconds(measurement.time);
            if (time_diff < -mConfiguration.observation_window * 0.5) {
                WARNF("measurement time out of window: %03ld:%04ld %s %s: %s out of %s", i, j,
                      observation.satellite_id.name(), measurement.signal_id.name(),
                      measurement.time.rtklib_time_string().c_str(),
                      time.rtklib_time_string().c_str());
                observation.measurement_mask[j] = false;
                observation.measurement_count -= 1;
                continue;
            }

            if (time_diff > mConfiguration.observation_window * 0.5) {
                measurment_skip[j] = true;  // Skip this measurement, but don't discard it
                continue;
            }

            // If the measurement is within the observation window, we need to align the
            // pseudo-range to the epoch time
            measurement.time = time;
            measurement.pseudo_range += constant::K_C * time_diff;
        }

        // Discard measurement that are of unsupported signal
        for (size_t j = 0; j < observation.measurement_mask.size(); ++j) {
            if (!observation.measurement_mask[j]) continue;
            if (measurment_skip[j]) continue;

            auto& measurement = observation.measurements[j];

            if (measurement.signal_id == SignalId::GPS_L1_CA) continue;

            WARNF("unsupported signal: %03ld:%04ld %s %s: %s %s", i, j,
                  observation.satellite_id.name(), measurement.signal_id.name(),
                  time.rtklib_time_string().c_str(), measurement.time.rtklib_time_string().c_str());

            observation.measurement_mask[j] = false;
            observation.measurement_count -= 1;
        }

        // Select the best measurement based on SNR
        long   best_id  = -1;
        double best_snr = 0.0;

        for (size_t j = 0; j < observation.measurement_mask.size(); ++j) {
            if (!observation.measurement_mask[j]) continue;
            if (measurment_skip[j]) continue;

            auto& measurement = observation.measurements[j];
            if (measurement.snr > best_snr) {
                best_id  = static_cast<long>(j);
                best_snr = measurement.snr;
            }
        }

        if (best_id < 0) {
            mObservationMask[i] = false;
            WARNF("no measurement selected: %03ld %s", observation.satellite_id.absolute_id(),
                  observation.satellite_id.name());
            continue;
        }

        observation.selected0 = best_id;

        // TODO: find secondary observation with highest SNR for ionospheric 1-order estimation

        DEBUGF("  %03ld %s: %04ld", observation.satellite_id.absolute_id(),
               observation.satellite_id.name(), observation.selected0);
    }
}

void SppEngine::compute_satellite_states(ts::Tai const& time) {
    FUNCTION_SCOPE();

    DEBUGF("computing satellite states");
    for (size_t i = 0; i < mObservations.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& observation = mObservations[i];
        if (observation.measurement_count == 0) continue;
        if (observation.selected0 < 0) continue;

        auto& measurement = observation.measurements[static_cast<size_t>(observation.selected0)];

        // Clear group delay
        for (auto& group_delay : observation.group_delay) {
            group_delay = 0.0;
        }

        // TODO(ewasjon): The ephemeris we should use here is the one that corresponds to when the
        // satellite transmitted the signal. It might (very unlikely) not be the one that
        // corresponds to the epoch time. Maybe the error is neglible?
        if (!mEphemerisEngine.find(observation.satellite_id, time, observation.ephemeris)) {
            mObservationMask[i] = false;
            WARNF("reject: %03ld %s: no ephemeris", observation.satellite_id.absolute_id(),
                  observation.satellite_id.name());
            continue;
        }

        OrbitCorrection const* orbit_correction = nullptr;
        auto correction = mCorrectionCache.satellite_correction(observation.satellite_id);
        if (correction) {
            auto it = correction->orbit.find(observation.ephemeris.iod());
            if (it != correction->orbit.end()) {
                orbit_correction = &it->second;
            }
        }

        if (!orbit_correction) {
            NOTICEF("missing orbit correction: %03ld %s iod=%u",
                    observation.satellite_id.absolute_id(), observation.satellite_id.name(),
                    observation.ephemeris.iod());
        }

        SatellitePosition result{};
        if (!satellite_position(observation.satellite_id, time, measurement.pseudo_range,
                                observation.ephemeris, mConfiguration.relativistic_model,
                                orbit_correction, result)) {
            mObservationMask[i] = false;
            WARNF("reject: %03ld %s: no position and velocity",
                  observation.satellite_id.absolute_id(), observation.satellite_id.name());
            continue;
        }

        observation.time           = time;
        observation.eph_position   = result.eph_position;
        observation.eph_velocity   = result.eph_velocity;
        observation.eph_clock_bias = result.eph_clock_bias;
        observation.group_delay[0] = result.group_delay;

        observation.true_position   = result.true_position;
        observation.true_velocity   = result.true_velocity;
        observation.true_clock_bias = result.true_clock_bias;
    }
}

Solution SppEngine::evaluate() NOEXCEPT {
    FUNCTION_SCOPE();

    Solution solution{};
    if (mConfiguration.epoch_selection == EpochSelection::FirstObservation) {
        if (mEpochFirstTimeSet) {
            solution = evaluate(mEpochFirstTime);
        } else {
            WARNF("epoch selection: first observation not found");
        }
    } else if (mConfiguration.epoch_selection == EpochSelection::LastObservation) {
        if (mEpochLastTimeSet) {
            solution = evaluate(mEpochLastTime);
        } else {
            WARNF("epoch selection: last observation not found");
        }
    } else if (mConfiguration.epoch_selection == EpochSelection::MeanObservation) {
        if (mEpochObservationCount > 0) {
            auto mean_time = ts::Tai{ts::Timestamp{mEpochTotalObservationTime /
                                                   static_cast<double>(mEpochObservationCount)}};
            solution       = evaluate(mean_time);
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

    select_best_observations(time);
    compute_satellite_states(time);

    // We can only solve if we have enough satellites
    auto satellite_count = mObservationMask.count();
    if (satellite_count < 4) {
        WARNF("not enough satellites: %d < 4", satellite_count);
        return Solution{};
    }

    DEBUGF("satellite available: %zu", satellite_count);

#if 0
    DEBUGF("      TIME(GPST)       SAT R        P1(m)        P2(m)       L1(cyc)       L2(cyc)  "
           "D1(Hz)  D2(Hz) S1 S2 LLI");
    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;

        auto& satellite = mSatelliteStates[i];
        DEBUGF("%s %3s 1 %12.3f %12.3f %12.3f %12.3f %12.3f %6.3f %6.3f %d %d %d",
               satellite.observation_time.rtklib_time_string(2).c_str(), satellite.id.name(),
               satellite.pseudo_range, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0);
    }

    for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
        if (!mSatelliteMask[i]) continue;

        auto& satellite = mSatelliteStates[i];
        printf("4 %s sat=%2ld rs=%13.3f %13.3f %13.3f dts=%12.3f var=%7.3f svh=00\n",
               satellite.transmit_time.rtklib_time_string(9).c_str(),
               satellite.id.lpp_id().value + 1, satellite.position.x(), satellite.position.y(),
               satellite.position.z(), satellite.clock_bias * 1e9, 0.0);
    }
#endif

    // Initial guess position: (0,0,0), receiver bias: 0, this will _always_ converge to the
    // correct solution (if possible)
    Vector4 current = {0, 0, 0, 0};
    DEBUGF("initial guess: %14.3f %14.3f %14.3f %14.3f", current(0), current(1), current(2),
           current(3));

    MatrixX residuals{satellite_count, 1};
    MatrixX design_matrix{satellite_count, 4};

    for (size_t it = 0; it < 10; ++it) {
        DEBUGF("iteration %d: %14.3f %14.3f %14.3f %14.3f", it, current(0), current(1), current(2),
               current(3));
        // Compute the geometric range to the guess

        auto ground_position = current.head<3>();
        auto ground_llh      = ecef_to_llh(ground_position, ellipsoid::WGS84);
        DEBUGF("ground ecef: %14.3f %14.3f %14.3f", ground_position.x(), ground_position.y(),
               ground_position.z());
        DEBUGF("ground llh:  %14.6f %14.6f %14.3f", ground_llh.x() * constant::K_R2D,
               ground_llh.y() * constant::K_R2D, ground_llh.z());

        auto cps = mCorrectionCache.correction_point_set(ground_llh);

        long j = 0;
        for (size_t i = 0; i < mObservationMask.size(); ++i) {
            if (!mObservationMask[i]) continue;

            auto& observation = mObservations[i];
            if (observation.measurement_count == 0) continue;
            if (observation.selected0 < 0) continue;

            auto geometric_range = geometric_distance(observation.true_position, ground_position);
            auto eph_geometric_range =
                geometric_distance(observation.eph_position, ground_position);
            auto line_of_sight = (observation.true_position - ground_position).normalized();
            auto enu           = ecef_to_enu_at_llh(ground_llh, line_of_sight);

            LookAngles look_angles;
            if (!compute_look_angles(ground_position, enu, observation.true_position,
                                     look_angles)) {
                WARNF("reject: %03ld %s: compute_look_angles failed",
                      observation.satellite_id.absolute_id(), observation.satellite_id.name());
                continue;
            }

            observation.elevation = look_angles.elevation;
            observation.azimuth   = look_angles.azimuth;
            observation.nadir     = look_angles.nadir;

            if (observation.elevation * constant::K_R2D < mConfiguration.elevation_cutoff) {
                WARNF("reject: %03ld %s: %.2fdeg < %.2fdeg", observation.satellite_id.absolute_id(),
                      observation.satellite_id.name(), observation.elevation * constant::K_R2D,
                      mConfiguration.elevation_cutoff);
                continue;
            }

            auto correction = mCorrectionCache.satellite_correction(observation.satellite_id);

            // TODO: We must align the measurement to a common time, otherwise we might use L1 and
            // L2 without correction for code biases
            auto& measurement =
                observation.measurements[static_cast<size_t>(observation.selected0)];

            auto pseudo_range = measurement.pseudo_range;

            Scalar i_residual   = 0.0;
            Scalar i_polynomial = 0.0;
            switch (mConfiguration.ionospheric_mode) {
            case IonosphericMode::None: break;
            case IonosphericMode::Broadcast:
                if (mKlobucharModelSet) {
                    i_polynomial = mKlobucharModel.evaluate(time, observation.elevation,
                                                            observation.azimuth, ground_llh);
                } else {
                    WARNF("ionospheric model not available");
                }
                break;
            case IonosphericMode::Dual: TODO("implement dual ionospheric model"); break;
            case IonosphericMode::Ssr: {
                if (cps) {
                    if (!cps->ionospheric_polynomial(ground_llh, observation.satellite_id,
                                                     i_polynomial)) {
                        DEBUGF("ionospheric polynomial not available");
                    }
                    if (!cps->ionospheric_residual(ground_llh, observation.satellite_id,
                                                   i_residual)) {
                        DEBUGF("ionospheric residual not available");
                    }
                } else {
                    WARNF("ionospheric correction not available");
                }
            } break;
            }

            auto i_delay = i_residual + i_polynomial;

            auto f_khz  = measurement.signal_id.frequency();
            auto f_hz   = f_khz * 1e3;
            auto i_bias = 40.3e16 * i_delay / (f_hz * f_hz);
            auto t_bias = 0.0;

            // clock bias
            auto rc_bias = current(3);
            auto sc_bias = constant::K_C * observation.true_clock_bias;
            auto sc_corr = 0.0;
            if (correction && correction->clock_valid) {
                // NOTE(ewasjon): The clock correction is evaluated at the transmit time - not the
                // epoch time
                sc_corr = correction->clock.evaluate(observation.time);
            }

            // code bias
            auto iod         = observation.ephemeris.iod();
            auto s_code_bias = 0.0;
            if (correction) {
                auto bias_it = correction->code_bias.find(measurement.signal_id);
                if (bias_it != correction->code_bias.end()) {
                    s_code_bias = bias_it->second;
                }
            }

            auto computed_range =
                geometric_range + rc_bias - sc_bias + sc_corr + s_code_bias + i_bias + t_bias;
            auto residual = pseudo_range - computed_range;

            DEBUGF("%+14.4f (%+.4f %+12.4f) %+8.4f %+8.4f %+8.4f %+8.4f|%+14.4f "
                   "- %+14.4f "
                   "= %+14.4fm|%4u %+8.4f|%5.2fdeg %6.2fdeg "
                   "%.4fSTEC| "
                   "%s %s",
                   geometric_range, rc_bias, -sc_bias, sc_corr, s_code_bias, i_bias, t_bias,
                   pseudo_range, computed_range, residual, iod,
                   geometric_range - eph_geometric_range, observation.elevation * constant::K_R2D,
                   observation.azimuth * constant::K_R2D, i_delay, observation.satellite_id.name(),
                   measurement.signal_id.name());
#if 0
            printf("sat=%ld v=%.3f P=%.3f r=%.3f dtr=%.6f dts=%.6f dion=%.3f dtrp=%.3f\n",
                   satellite.id.lpp_id().value + 1, residual,
                   satellite.pseudo_range - group_delay_bias, geometric_range, receiver_clock_bias,
                   satellite.clock_bias, i_bias, 0.0);
            printf("sat=%ld azel=%5.1f %4.1f res=%7.3f sig=%5.3f\n",
                   satellite.id.lpp_id().value + 1, satellite.azimuth * constant::K_R2D,
                   satellite.elevation * constant::K_R2D, residual, 0.0);
#endif
            design_matrix(j, 0) = -line_of_sight.x();
            design_matrix(j, 1) = -line_of_sight.y();
            design_matrix(j, 2) = -line_of_sight.z();
            design_matrix(j, 3) = 1.0;

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
        DEBUGF("solution: %+14.3f %+14.3f %+14.3f %+14.3f (norm %14.8f)", solution(0), solution(1),
               solution(2), solution(3), solution.norm());
        current += solution;
        DEBUGF("current: %+14.3f %+14.3f %+14.3f %+14.3f", current(0), current(1), current(2),
               current(3));

#if 0
        // Compute per satellite residuals
        DEBUGF("residuals:");
        j = 0;
        for (size_t i = 0; i < mSatelliteMask.size(); ++i) {
            if (!mSatelliteMask[i]) continue;

            auto residual = design_matrix.row(j).dot(current) - residuals(j);
            DEBUGF("  %3s: %14.3f", mSatelliteStates[i].id.name(), residual);
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
#endif
        if (solution.norm() < 0.0001) {
            break;
        }
    }

    datatrace_report();

    auto final_position = current.head<3>();
    auto final_bias     = current(3) / constant::K_C;
    DEBUGF("final position: %14.4f %14.4f %14.4f", final_position.x(), final_position.y(),
           final_position.z());
    DEBUGF("final bias:     %14.4f", final_bias);

    auto llh = generator::tokoro::ecef_to_llh(
        Float3{final_position.x(), final_position.y(), final_position.z()},
        generator::tokoro::ellipsoid::gWgs84);

    DEBUGF("llh: %14.8f %14.8f %14.8f", llh.x * constant::K_R2D, llh.y * constant::K_R2D, llh.z);
    DEBUGF("bias: %14.8f", final_bias);
    DEBUGF("time: %s", time.rtklib_time_string().c_str());

    Solution solution{
        .time            = time,
        .status          = Solution::Status::Standard,
        .latitude        = llh.x * constant::K_R2D,
        .longitude       = llh.y * constant::K_R2D,
        .altitude        = llh.z,
        .satellite_count = satellite_count,
    };

    mEpochFirstTimeSet         = false;
    mEpochLastTimeSet          = false;
    mEpochObservationCount     = 0;
    mEpochTotalObservationTime = 0;

    return solution;
}

void SppEngine::datatrace_report() NOEXCEPT {
    VSCOPE_FUNCTION();
#ifdef DATA_TRACING
    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;
        auto&                satellite = mObservations[i];
        datatrace::Satellite dt_sat{};
        dt_sat.position  = Float3{satellite.true_position.x(), satellite.true_position.y(),
                                 satellite.true_position.z()};
        dt_sat.elevation = satellite.elevation * constant::K_R2D;
        dt_sat.azimuth   = satellite.azimuth * constant::K_R2D;
        dt_sat.nadir     = satellite.nadir * constant::K_R2D;
        // TODO(ewasjon): Wrong time?
        datatrace::report_satellite(satellite.time, satellite.satellite_id.name(), dt_sat);
    }
#endif
}

}  // namespace idokeido

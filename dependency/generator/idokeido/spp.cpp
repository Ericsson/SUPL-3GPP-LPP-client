#include "spp.hpp"
#include "eph.hpp"

#include <cmath>
#ifdef INCLUDE_GENERATOR_TOKORO
#include <generator/tokoro/coordinate.hpp>
#endif
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

    mKlobucharModelSet    = false;
    mBdsKlobucharModelSet = false;
    mFilterInitialized    = false;
    mLastEpochTimeSet     = false;

    // Assign ISB state indices. GPS is reference if enabled, else first enabled system.
    // Reference constellation gets index -1 (uses kIdxClk directly).
    auto& g       = mConfiguration.gnss;
    bool  gps_ref = g.gps;
    bool  gal_ref = !gps_ref && g.gal;
    bool  glo_ref = !gps_ref && !gal_ref && g.glo;
    bool  bds_ref = !gps_ref && !gal_ref && !glo_ref && g.bds;

    long next_isb = 4;
    mIsbGps       = (g.gps && !gps_ref) ? next_isb++ : -1;
    mIsbGal       = (g.gal && !gal_ref) ? next_isb++ : -1;
    mIsbGlo       = (g.glo && !glo_ref) ? next_isb++ : -1;
    mIsbBds       = (g.bds && !bds_ref) ? next_isb++ : -1;
    // QZSS uses GPS time and is steered to GPS — treat as GPS-compatible, no ISB needed
    mIsbQzs     = -1;
    mBaseStates = next_isb;
}

SppEngine::~SppEngine() {
    FUNCTION_SCOPE();
}

void SppEngine::klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT {
    FUNCTION_SCOPE();
    mKlobucharModelSet = true;
    mKlobucharModel    = parameters;
}

void SppEngine::bds_klobuchar_model(KlobucharModelParameters const& parameters) NOEXCEPT {
    FUNCTION_SCOPE();
    mBdsKlobucharModelSet = true;
    mBdsKlobucharModel    = parameters;
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
    if (!mConfiguration.gnss.qzs && raw.satellite_id.is_qzss()) return;

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
            if (measurement.signal_id == SignalId::GPS_L1_Z_TRACKING) continue;
            if (measurement.signal_id == SignalId::GALILEO_E1_B_C) continue;
            if (measurement.signal_id == SignalId::GALILEO_E1_C_NO_DATA) continue;
            if (measurement.signal_id == SignalId::BEIDOU_B1_I) continue;
            if (measurement.signal_id == SignalId::GLONASS_G1_CA) continue;
            if (measurement.signal_id == SignalId::QZSS_L1_CA) continue;
            if (measurement.signal_id == SignalId::QZSS_L1C_D) continue;
            if (measurement.signal_id == SignalId::QZSS_L1C_P) continue;
            if (measurement.signal_id == SignalId::QZSS_L1C_D_P) continue;

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

        if (best_id < 0 ||
            (mConfiguration.snr_cutoff > 0.0 && best_snr < mConfiguration.snr_cutoff)) {
            mObservationMask[i] = false;
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
    if (static_cast<long>(satellite_count) < mBaseStates) {
        WARNF("not enough satellites: %zu < %ld", satellite_count, mBaseStates);
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

    // Initial guess: [X, Y, Z, clk, ISB...]
    VectorX current = VectorX::Zero(mBaseStates);

    // If filter is active, predict and use filter state as initial guess
    bool use_filter = mConfiguration.filter_mode != FilterMode::None;
    if (use_filter) {
        if (mFilterInitialized) {
            predict_filter(time);
            for (long k = 0; k < mBaseStates; ++k)
                current(k) = mFilter.state(k);
        }
    }

    MatrixX residuals{satellite_count, 1};
    MatrixX design_matrix{satellite_count, mBaseStates};
    VectorX weights{satellite_count};
    MatrixX last_dTd = MatrixX::Identity(mBaseStates, mBaseStates);  // for DOP

    // Per-epoch outlier exclusion mask (index = observation index in mObservations)
    std::bitset<SATELLITE_ID_MAX> outlier_mask;

    // Chi-squared threshold table: chi2_inv(0.999, df) for df=1..20
    static constexpr double kChi2[21] = {0,     10.83, 13.82, 16.27, 18.47, 20.52, 22.46,
                                         24.32, 26.12, 27.88, 29.59, 31.26, 32.91, 34.53,
                                         36.12, 37.70, 39.25, 40.79, 42.31, 43.82, 45.31};

    // Outer loop: outlier detection and exclusion (RAIM-FDE style)
    for (size_t raim_it = 0; raim_it < 8; ++raim_it) {
        current.setZero();

        // Saved from last LS iteration for RAIM
        std::vector<size_t> row_to_sat;
        VectorX             raw_residuals(satellite_count);

        for (size_t it = 0; it < 10; ++it) {
            // Compute the geometric range to the guess

            auto ground_position = current.head(3);
            auto ground_llh      = ecef_to_llh(ground_position, ellipsoid::WGS84);

            auto cps = mCorrectionCache.correction_point_set(ground_llh);

            row_to_sat.clear();
            long j = 0;
            for (size_t i = 0; i < mObservationMask.size(); ++i) {
                if (!mObservationMask[i]) continue;
                if (outlier_mask[i]) continue;  // skip excluded satellites

                auto& observation = mObservations[i];
                if (observation.measurement_count == 0) continue;
                if (observation.selected0 < 0) continue;
                auto geometric_range =
                    geometric_distance(observation.true_position, ground_position);
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
                    WARNF("reject: %03ld %s: %.2fdeg < %.2fdeg",
                          observation.satellite_id.absolute_id(), observation.satellite_id.name(),
                          observation.elevation * constant::K_R2D, mConfiguration.elevation_cutoff);
                    continue;
                }

                auto correction = mCorrectionCache.satellite_correction(observation.satellite_id);

                // TODO: We must align the measurement to a common time, otherwise we might use L1
                // and L2 without correction for code biases
                auto& measurement =
                    observation.measurements[static_cast<size_t>(observation.selected0)];

                auto pseudo_range = measurement.pseudo_range;

                Scalar i_bias = 0.0;
                switch (mConfiguration.ionospheric_mode) {
                case IonosphericMode::None: break;
                case IonosphericMode::Broadcast: {
                    bool is_bds = observation.satellite_id.is_beidou();
                    if (is_bds && mBdsKlobucharModelSet) {
                        i_bias = mBdsKlobucharModel.evaluate(time, observation.elevation,
                                                             observation.azimuth, ground_llh);
                    } else if (!is_bds && mKlobucharModelSet) {
                        i_bias = mKlobucharModel.evaluate(time, observation.elevation,
                                                          observation.azimuth, ground_llh);
                    } else {
                        WARNF("ionospheric model not available");
                    }
                } break;
                case IonosphericMode::Dual: TODO("implement dual ionospheric model"); break;
                case IonosphericMode::Ssr: {
                    // SSR provides STEC (m²/s²) — convert to meters via 40.3e16/f²
                    Scalar i_polynomial = 0.0, i_residual = 0.0;
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
                    auto f_hz = measurement.signal_id.frequency() * 1e3;
                    i_bias    = 40.3e16 * (i_polynomial + i_residual) / (f_hz * f_hz);
                } break;
                }
                auto t_bias = 0.0;
                if (mConfiguration.tropospheric_mode == TroposphericMode::Saastamoinen &&
                    ground_position.norm() > 1e6) {
                    auto h      = ground_llh.z();
                    auto ztd    = 2.3 * std::exp(-0.116e-3 * h);
                    auto sin_el = std::sin(observation.elevation);
                    t_bias      = ztd / (sin_el > 0.01 ? sin_el : 0.01);
                }

                // clock bias: reference clock + ISB for this constellation
                auto isb_idx = isb_index(observation.satellite_id);
                auto rc_bias = current(kIdxClk) + (isb_idx >= 0 ? current(isb_idx) : 0.0);
                auto sc_bias = constant::K_C * observation.true_clock_bias;
                auto sc_corr = 0.0;
                if (correction && correction->clock_valid) {
                    // NOTE(ewasjon): The clock correction is evaluated at the transmit time - not
                    // the epoch time
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
                       geometric_range - eph_geometric_range,
                       observation.elevation * constant::K_R2D,
                       observation.azimuth * constant::K_R2D, i_bias,
                       observation.satellite_id.name(), measurement.signal_id.name());
#if 0
            printf("sat=%ld v=%.3f P=%.3f r=%.3f dtr=%.6f dts=%.6f dion=%.3f dtrp=%.3f\n",
                   satellite.id.lpp_id().value + 1, residual,
                   satellite.pseudo_range - group_delay_bias, geometric_range, receiver_clock_bias,
                   satellite.clock_bias, i_bias, 0.0);
            printf("sat=%ld azel=%5.1f %4.1f res=%7.3f sig=%5.3f\n",
                   satellite.id.lpp_id().value + 1, satellite.azimuth * constant::K_R2D,
                   satellite.elevation * constant::K_R2D, residual, 0.0);
#endif
                design_matrix.row(j).setZero();
                design_matrix(j, 0) = -line_of_sight.x();
                design_matrix(j, 1) = -line_of_sight.y();
                design_matrix(j, 2) = -line_of_sight.z();
                design_matrix(j, 3) = 1.0;
                if (isb_idx >= 0) design_matrix(j, isb_idx) = 1.0;

                residuals(j, 0) = residual;

                switch (mConfiguration.weight_function) {
                case WeightFunction::None: weights(j) = 1.0; break;
                case WeightFunction::Elevation: {
                    auto s     = std::sin(observation.elevation);
                    weights(j) = s * s;
                } break;
                case WeightFunction::Snr:
                    weights(j) = measurement.snr > 0.0 ? measurement.snr : 1.0;
                    break;
                case WeightFunction::Variance: {
                    auto s = std::sin(observation.elevation);
                    auto sigma =
                        mConfiguration.sigma_a + mConfiguration.sigma_b / (s > 0.01 ? s : 0.01);
                    weights(j) = 1.0 / (sigma * sigma);
                } break;
                }

                raw_residuals(j) = residual;
                row_to_sat.push_back(i);
                j++;
            }

            auto h_subset = design_matrix.topRows(j).eval();
            auto r_subset = residuals.topRows(j).eval();
            auto w_subset = weights.head(j).eval();

            if (j < mBaseStates) break;  // underdetermined, abort

            // Apply weights: scale rows by sqrt(w) so H^T W H = (√W H)^T (√W H)
            for (long k = 0; k < j; ++k) {
                auto sw = std::sqrt(w_subset(k));
                h_subset.row(k) *= sw;
                r_subset.row(k) *= sw;
            }

            auto h_transpose = h_subset.transpose();

            // Compute the solution (use unweighted H for DOP)
            auto H_unweighted  = design_matrix.topRows(j).eval();
            auto Ht_unweighted = H_unweighted.transpose();
            last_dTd           = Ht_unweighted * H_unweighted;

            auto dTd      = h_transpose * h_subset;
            auto dTr      = h_transpose * r_subset;
            auto solution = dTd.ldlt().solve(dTr).eval();
            DEBUGF("solution: %+14.3f %+14.3f %+14.3f %+14.3f (norm %14.8f)", solution(0),
                   solution(1), solution(2), solution(3), solution.norm());
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
        }  // end inner LS loop

        // Chi-squared RAIM using projection matrix post-fit residuals
        if (mConfiguration.reject_outliers) {
            long active = 0;
            for (size_t i = 0; i < mObservationMask.size(); ++i)
                if (mObservationMask[i] && !outlier_mask[i]) active++;
            if (active <= 4) break;

            long m = static_cast<long>(row_to_sat.size());
            if (m <= 4) break;
            long df = m - 4;
            if (df > 20) df = 20;

            // Build weighted H and r for the converged solution
            auto H = design_matrix.topRows(m).eval();
            auto r = raw_residuals.head(m).eval();

            // Use uniform σ=3m for RAIM (typical SPP pseudorange noise)
            // This makes WSSR = Σ r_i²/9 follow chi2(m-4) under null hypothesis
            double sigma2_raim = 9.0;  // 3m²

            // Post-fit residuals via projection: v = r - H(H^TH)^{-1}H^T r
            auto    HtH = (H.transpose() * H).eval();
            auto    Htr = (H.transpose() * r).eval();
            auto    dx  = HtH.ldlt().solve(Htr).eval();
            VectorX v   = r - H * dx;

            double wssr = v.squaredNorm() / sigma2_raim;

            if (wssr <= kChi2[df]) break;

            // FDE: find satellite whose exclusion most reduces WSSR
            double best_wssr = wssr;
            size_t best_sat  = SIZE_MAX;
            for (long k = 0; k < m; ++k) {
                // Recompute WSSR without row k
                MatrixX H2(m - 1, 4);
                VectorX r2(m - 1);
                long    row = 0;
                for (long kk = 0; kk < m; ++kk) {
                    if (kk == k) continue;
                    H2.row(row) = H.row(kk);
                    r2(row)     = r(kk);
                    row++;
                }
                auto dx2 = (H2.transpose() * H2).ldlt().solve((H2.transpose() * r2).eval()).eval();
                VectorX v2    = r2 - H2 * dx2;
                double  wssr2 = v2.squaredNorm() / sigma2_raim;
                if (wssr2 < best_wssr) {
                    best_wssr = wssr2;
                    best_sat  = row_to_sat[k];
                }
            }
            if (best_sat != SIZE_MAX) {
                outlier_mask[best_sat] = true;
                continue;
            }
        }
        break;
    }  // end outer RAIM loop

    datatrace_report();

    // Kalman update: use LS solution as measurement for all base states
    if (use_filter) {
        if (!mFilterInitialized) {
            initialize_filter(current);
        } else {
            long    n = mFilter.size();
            long    m = mBaseStates;
            MatrixX H = MatrixX::Zero(m, n);
            MatrixX R = MatrixX::Zero(m, m);
            VectorX z(m);

            for (long k = 0; k < m; ++k) {
                H(k, k) = 1.0;
                z(k)    = current(k) - mFilter.state(k);
            }
            // Position uncertainty ~100m², clock ~1e6m², ISB ~1e4m²
            R(0, 0) = R(1, 1) = R(2, 2) = 100.0;
            R(3, 3)                     = 1e6;
            for (long k = 4; k < m; ++k)
                R(k, k) = 1e4;

            mFilter.update(H, R, z);
        }
        mLastEpochTime    = time;
        mLastEpochTimeSet = true;

        // Use filter state as final solution
        for (long k = 0; k < mBaseStates; ++k)
            current(k) = mFilter.state(k);
    }

    auto final_position = current.head(3);
    auto final_bias     = current(kIdxClk) / constant::K_C;
    DEBUGF("final position: %14.4f %14.4f %14.4f", final_position.x(), final_position.y(),
           final_position.z());
    DEBUGF("final bias:     %14.4f", final_bias);

#ifdef INCLUDE_GENERATOR_TOKORO
    auto llh = generator::tokoro::ecef_to_llh(
        Float3{final_position.x(), final_position.y(), final_position.z()},
        generator::tokoro::ellipsoid::gWgs84);

    DEBUGF("llh: %14.8f %14.8f %14.8f", llh.x * constant::K_R2D, llh.y * constant::K_R2D, llh.z);
#endif
    DEBUGF("bias: %14.8f", final_bias);
    DEBUGF("time: %s", time.rtklib_time_string().c_str());

    // Compute DOP from Q = (H^T H)^{-1}
    double pdop = 0.0, hdop = 0.0, vdop = 0.0, tdop = 0.0;
    {
        MatrixX Q   = last_dTd.inverse();
        auto    pos = current.head(3);
        double  r   = pos.norm();
        if (r > 1e3) {
            double  lat = std::asin(pos.z() / r), lon = std::atan2(pos.y(), pos.x());
            double  sl = std::sin(lat), cl = std::cos(lat), sn = std::sin(lon), cn = std::cos(lon);
            Matrix3 R;
            R << -sn, cn, 0, -sl * cn, -sl * sn, cl, cl * cn, cl * sn, sl;
            Matrix3 Q_enu = R * Q.topLeftCorner<3, 3>() * R.transpose();
            hdop          = std::sqrt(std::max(0.0, Q_enu(0, 0) + Q_enu(1, 1)));
            vdop          = std::sqrt(std::max(0.0, Q_enu(2, 2)));
        }
        pdop = std::sqrt(std::max(0.0, Q(0, 0) + Q(1, 1) + Q(2, 2)));
        tdop = std::sqrt(std::max(0.0, Q(3, 3)));
    }

    Solution solution{
        .time            = time,
        .status          = Solution::Status::Standard,
        .latitude        = llh.x * constant::K_R2D,
        .longitude       = llh.y * constant::K_R2D,
        .altitude        = llh.z,
        .position_ecef   = Vector3{final_position.x(), final_position.y(), final_position.z()},
        .receiver_clock  = final_bias,
        .pdop            = pdop,
        .hdop            = hdop,
        .vdop            = vdop,
        .tdop            = tdop,
        .satellite_count = satellite_count,
    };

    // Populate per-satellite info for logging/comparison
    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i] || outlier_mask[i]) continue;
        auto& obs = mObservations[i];
        if (obs.selected0 < 0) continue;
        if (obs.elevation * constant::K_R2D < mConfiguration.elevation_cutoff) continue;
        auto&                   m  = obs.measurements[static_cast<size_t>(obs.selected0)];
        auto                    gr = geometric_distance(obs.true_position, final_position);
        auto                    sc = constant::K_C * obs.true_clock_bias;
        Solution::SatelliteInfo info{};
        info.id          = obs.satellite_id;
        info.azimuth     = obs.azimuth;
        info.elevation   = obs.elevation;
        auto isb_i       = isb_index(obs.satellite_id);
        auto rc_i        = current(kIdxClk) + (isb_i >= 0 ? current(isb_i) : 0.0);
        info.residual    = m.pseudo_range - (gr + rc_i - sc);
        info.sat_pos     = obs.true_position;
        info.sat_clock   = obs.true_clock_bias;
        info.pseudorange = m.pseudo_range;
        solution.satellites.push_back(info);
    }

    mEpochFirstTimeSet         = false;
    mEpochLastTimeSet          = false;
    mEpochObservationCount     = 0;
    mEpochTotalObservationTime = 0;

    return solution;
}

void SppEngine::initialize_filter(VectorX const& ls_solution) NOEXCEPT {
    bool is_velocity = mConfiguration.filter_mode == FilterMode::KinematicVelocity;
    long n           = is_velocity ? (mBaseStates + 4) : mBaseStates;
    // For KinematicVelocity: [X,Y,Z,clk,ISB...,Vx,Vy,Vz,clk_dot]

    VectorX x0 = VectorX::Zero(n);
    MatrixX P0 = MatrixX::Zero(n, n);

    for (long k = 0; k < mBaseStates; ++k)
        x0(k) = ls_solution(k);

    P0(kIdxX, kIdxX)     = 1e4;
    P0(kIdxY, kIdxY)     = 1e4;
    P0(kIdxZ, kIdxZ)     = 1e4;
    P0(kIdxClk, kIdxClk) = 1e10;
    for (long k = 4; k < mBaseStates; ++k)
        P0(k, k) = 1e6;  // ISB: unknown initially

    if (is_velocity) {
        long vx = mBaseStates, vy = mBaseStates + 1, vz = mBaseStates + 2, cd = mBaseStates + 3;
        P0(vx, vx) = P0(vy, vy) = P0(vz, vz) = 100.0;
        P0(cd, cd)                           = 1e6;
    }

    mFilter.initialize(x0, P0);
    mFilterInitialized = true;
}

void SppEngine::predict_filter(ts::Tai const& time) NOEXCEPT {
    double dt = mLastEpochTimeSet ? time.difference_seconds(mLastEpochTime) : 0.0;
    if (dt < 0.0 || dt > 300.0) dt = 0.0;

    long    n = mFilter.size();
    MatrixX F = MatrixX::Identity(n, n);
    MatrixX Q = MatrixX::Zero(n, n);

    // Build position process noise in ENU then rotate to ECEF
    auto pos = mFilter.state().head<3>();
    auto r   = pos.norm();
    if (r < 1e3) {
        Q(kIdxX, kIdxX) = mConfiguration.process_noise.position_horizontal;
        Q(kIdxY, kIdxY) = mConfiguration.process_noise.position_horizontal;
        Q(kIdxZ, kIdxZ) = mConfiguration.process_noise.position_vertical;
    } else {
        double  lat = std::asin(pos.z() / r);
        double  lon = std::atan2(pos.y(), pos.x());
        double  sl = std::sin(lat), cl = std::cos(lat);
        double  sn = std::sin(lon), cn = std::cos(lon);
        Vector3 e_east  = {-sn, cn, 0.0};
        Vector3 e_north = {-sl * cn, -sl * sn, cl};
        Vector3 e_up    = {cl * cn, cl * sn, sl};
        double  qh      = mConfiguration.process_noise.position_horizontal;
        double  qv      = mConfiguration.process_noise.position_vertical;
        Matrix3 Q3      = qh * (e_east * e_east.transpose() + e_north * e_north.transpose()) +
                     qv * (e_up * e_up.transpose());
        Q.block<3, 3>(0, 0) = Q3;
    }

    Q(kIdxClk, kIdxClk) = mConfiguration.process_noise.clock;
    // ISB: very small process noise (hardware bias changes slowly)
    for (long k = 4; k < mBaseStates; ++k)
        Q(k, k) = 1.0;  // ~1m²/s — allows slow drift

    if (mConfiguration.filter_mode == FilterMode::KinematicVelocity && dt > 0.0) {
        long vx = mBaseStates, vy = mBaseStates + 1, vz = mBaseStates + 2, cd = mBaseStates + 3;
        F(kIdxX, vx)   = dt;
        F(kIdxY, vy)   = dt;
        F(kIdxZ, vz)   = dt;
        F(kIdxClk, cd) = dt;
        double qvel    = mConfiguration.process_noise.velocity;
        Q(vx, vx) = Q(vy, vy) = Q(vz, vz) = qvel;
        Q(cd, cd)                         = mConfiguration.process_noise.clock * 1e-6;
    }

    mFilter.predict(F, Q);
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

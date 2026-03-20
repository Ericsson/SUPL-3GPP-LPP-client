#include "ppp.hpp"
#include "correction.hpp"
#include "eph.hpp"
#include "satellite.hpp"

#include "coordinates/ecef.hpp"
#include "coordinates/enu.hpp"
#include "coordinates/look_angles.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, ppp);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, ppp)

namespace idokeido {

PppEngine::PppEngine(PppConfiguration configuration, EphemerisEngine& ephemeris_engine,
                     CorrectionCache& correction_cache) NOEXCEPT
    : mConfiguration(std::move(configuration)),
      mEphemerisEngine(ephemeris_engine),
      mCorrectionCache(correction_cache),
      mFilterInitialized(false),
      mLastEpochTimeSet(false),
      mEpochFirstTimeSet(false),
      mEpochLastTimeSet(false),
      mEpochObservationCount(0),
      mEpochTotalObservationTime(0.0) {
    FUNCTION_SCOPE();

    // Assign fixed state indices
    mIdxZtdDry     = -1;
    mIdxZtdWet     = -1;
    mBaseStateSize = 4;  // X, Y, Z, clk

    if (mConfiguration.troposphere_dry_mode == PppTroposphereMode::Estimated) {
        mIdxZtdDry = mBaseStateSize++;
    }
    if (mConfiguration.troposphere_wet_mode == PppTroposphereMode::Estimated) {
        mIdxZtdWet = mBaseStateSize++;
    }
}

PppEngine::~PppEngine() = default;

void PppEngine::add_measurement(RawMeasurement const& raw) NOEXCEPT {
    FUNCTION_SCOPEF("%s %s", raw.satellite_id.name(), raw.signal_id.name());

    if (!mConfiguration.gnss.gps && raw.satellite_id.is_gps()) return;
    if (!mConfiguration.gnss.glo && raw.satellite_id.is_glonass()) return;
    if (!mConfiguration.gnss.gal && raw.satellite_id.is_galileo()) return;
    if (!mConfiguration.gnss.bds && raw.satellite_id.is_beidou()) return;

    auto satellite_id = raw.satellite_id.absolute_id();
    if (satellite_id < 0) return;

    auto sat_idx              = static_cast<size_t>(satellite_id);
    mObservationMask[sat_idx] = true;

    auto& obs        = mObservations[sat_idx];
    obs.satellite_id = raw.satellite_id;
    obs.add_measurement(Measurement{
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

void PppEngine::select_best_observations(ts::Tai const& time) NOEXCEPT {
    FUNCTION_SCOPE();

    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& obs = mObservations[i];
        if (obs.measurement_count == 0) {
            mObservationMask[i] = false;
            continue;
        }

        obs.selected0 = -1;
        obs.selected1 = -1;

        // Discard out-of-window measurements
        for (size_t j = 0; j < obs.measurement_mask.size(); ++j) {
            if (!obs.measurement_mask[j]) continue;
            auto& m         = obs.measurements[j];
            auto  time_diff = time.difference_seconds(m.time);
            if (time_diff < -mConfiguration.observation_window * 0.5) {
                obs.measurement_mask[j] = false;
                obs.measurement_count -= 1;
                continue;
            }
            if (std::fabs(time_diff) <= mConfiguration.observation_window * 0.5) {
                m.time = time;
                m.pseudo_range += constant::K_C * time_diff;
            }
        }

        // Select best by SNR (GPS L1 CA only for now)
        long   best_id  = -1;
        double best_snr = 0.0;
        for (size_t j = 0; j < obs.measurement_mask.size(); ++j) {
            if (!obs.measurement_mask[j]) continue;
            auto& m = obs.measurements[j];
            if (m.signal_id != SignalId::GPS_L1_CA) continue;
            if (m.snr > best_snr) {
                best_id  = static_cast<long>(j);
                best_snr = m.snr;
            }
        }

        if (best_id < 0) {
            mObservationMask[i] = false;
            continue;
        }
        obs.selected0 = best_id;
    }
}

void PppEngine::compute_satellite_states(ts::Tai const& time) NOEXCEPT {
    FUNCTION_SCOPE();

    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& obs = mObservations[i];
        if (obs.selected0 < 0) continue;

        auto& m = obs.measurements[static_cast<size_t>(obs.selected0)];

        if (!mEphemerisEngine.find(obs.satellite_id, time, obs.ephemeris)) {
            mObservationMask[i] = false;
            WARNF("reject: %s: no ephemeris", obs.satellite_id.name());
            continue;
        }

        OrbitCorrection const* orbit_correction = nullptr;
        auto                   correction = mCorrectionCache.satellite_correction(obs.satellite_id);
        if (correction) {
            auto it = correction->orbit.find(obs.ephemeris.iod());
            if (it != correction->orbit.end()) orbit_correction = &it->second;
        }

        SatellitePosition result{};
        if (!satellite_position(obs.satellite_id, time, m.pseudo_range, obs.ephemeris,
                                mConfiguration.relativistic_model, orbit_correction, result)) {
            mObservationMask[i] = false;
            WARNF("reject: %s: no satellite position", obs.satellite_id.name());
            continue;
        }

        obs.time            = time;
        obs.eph_position    = result.eph_position;
        obs.eph_velocity    = result.eph_velocity;
        obs.eph_clock_bias  = result.eph_clock_bias;
        obs.group_delay     = result.group_delay;
        obs.true_position   = result.true_position;
        obs.true_velocity   = result.true_velocity;
        obs.true_clock_bias = result.true_clock_bias;
    }
}

void PppEngine::seed(Solution const& spp) NOEXCEPT {
    if (spp.status != Solution::Status::Standard) return;

    VectorX x0 = VectorX::Zero(mBaseStateSize);
    MatrixX P0 = MatrixX::Zero(mBaseStateSize, mBaseStateSize);

    x0(kIdxX)   = spp.position_ecef.x();
    x0(kIdxY)   = spp.position_ecef.y();
    x0(kIdxZ)   = spp.position_ecef.z();
    x0(kIdxClk) = spp.receiver_clock * constant::K_C;

    P0(kIdxX, kIdxX)     = 1e4;  // ~100m 1-sigma from SPP
    P0(kIdxY, kIdxY)     = 1e4;
    P0(kIdxZ, kIdxZ)     = 1e4;
    P0(kIdxClk, kIdxClk) = 1e6;  // ~1km clock uncertainty

    if (mIdxZtdDry >= 0) {
        x0(mIdxZtdDry)             = 2.3;
        P0(mIdxZtdDry, mIdxZtdDry) = 1.0;
    }
    if (mIdxZtdWet >= 0) {
        x0(mIdxZtdWet)             = 0.1;
        P0(mIdxZtdWet, mIdxZtdWet) = 1.0;
    }

    mFilter.initialize(x0, P0);
    mFilterInitialized = true;

    for (auto& obs : mObservations) {
        obs.ambiguity_state_idx = -1;
        obs.iono_state_idx      = -1;
    }
}

void PppEngine::initialize_filter() NOEXCEPT {
    FUNCTION_SCOPE();

    VectorX x0 = VectorX::Zero(mBaseStateSize);
    MatrixX P0 = MatrixX::Zero(mBaseStateSize, mBaseStateSize);

    // Position: large initial uncertainty
    P0(kIdxX, kIdxX) = 1e8;
    P0(kIdxY, kIdxY) = 1e8;
    P0(kIdxZ, kIdxZ) = 1e8;
    // Clock: very large (unknown)
    P0(kIdxClk, kIdxClk) = 1e10;

    if (mIdxZtdDry >= 0) {
        x0(mIdxZtdDry)             = 2.3;  // ~2.3m a priori zenith dry delay
        P0(mIdxZtdDry, mIdxZtdDry) = 1.0;
    }
    if (mIdxZtdWet >= 0) {
        x0(mIdxZtdWet)             = 0.1;
        P0(mIdxZtdWet, mIdxZtdWet) = 1.0;
    }

    mFilter.initialize(x0, P0);
    mFilterInitialized = true;

    // Reset per-satellite state tracking
    for (auto& obs : mObservations) {
        obs.ambiguity_state_idx = -1;
        obs.iono_state_idx      = -1;
    }
}

void PppEngine::predict_filter() NOEXCEPT {
    FUNCTION_SCOPE();

    auto    n = mFilter.size();
    MatrixX F = MatrixX::Identity(n, n);
    MatrixX Q = MatrixX::Zero(n, n);

    // Position process noise (0 = static, small = kinematic)
    Q(kIdxX, kIdxX) = mConfiguration.process_noise_position;
    Q(kIdxY, kIdxY) = mConfiguration.process_noise_position;
    Q(kIdxZ, kIdxZ) = mConfiguration.process_noise_position;

    // Clock: white noise — reset each epoch
    Q(kIdxClk, kIdxClk) = mConfiguration.process_noise_clock;

    if (mIdxZtdDry >= 0) Q(mIdxZtdDry, mIdxZtdDry) = mConfiguration.process_noise_ztd_dry;
    if (mIdxZtdWet >= 0) Q(mIdxZtdWet, mIdxZtdWet) = mConfiguration.process_noise_ztd_wet;

    // Ambiguity and iono states: zero process noise (constant)
    // (they are at indices >= mBaseStateSize, Q is already zero there)

    mFilter.predict(F, Q);
}

void PppEngine::manage_ambiguity_states(ts::Tai const& time) NOEXCEPT {
    FUNCTION_SCOPE();

    // Remove states for satellites no longer visible
    // Iterate in reverse so indices remain valid after removal
    for (long i = static_cast<long>(mObservations.size()) - 1; i >= 0; --i) {
        auto& obs = mObservations[static_cast<size_t>(i)];
        if (obs.ambiguity_state_idx < 0) continue;

        bool still_visible = mObservationMask[static_cast<size_t>(i)];
        if (!still_visible) {
            // Remove iono state first (higher index) then ambiguity
            if (obs.iono_state_idx >= 0) {
                // Adjust other indices that are above this one
                for (auto& other : mObservations) {
                    if (other.iono_state_idx > obs.iono_state_idx) other.iono_state_idx--;
                    if (other.ambiguity_state_idx > obs.iono_state_idx) other.ambiguity_state_idx--;
                }
                mFilter.remove_state(obs.iono_state_idx);
                obs.iono_state_idx = -1;
            }
            if (obs.ambiguity_state_idx >= 0) {
                for (auto& other : mObservations) {
                    if (other.iono_state_idx > obs.ambiguity_state_idx) other.iono_state_idx--;
                    if (other.ambiguity_state_idx > obs.ambiguity_state_idx)
                        other.ambiguity_state_idx--;
                }
                mFilter.remove_state(obs.ambiguity_state_idx);
                obs.ambiguity_state_idx = -1;
            }
        }
    }

    // Add states for newly visible satellites or cycle slips
    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& obs = mObservations[i];
        if (obs.selected0 < 0) continue;

        auto& m = obs.measurements[static_cast<size_t>(obs.selected0)];

        bool cycle_slip = mConfiguration.reject_cycle_slip && obs.prev_lock_time > 0.0 &&
                          m.lock_time < obs.prev_lock_time;

        if (obs.ambiguity_state_idx < 0 || cycle_slip) {
            if (cycle_slip && obs.ambiguity_state_idx >= 0) {
                DEBUGF("cycle slip: %s, resetting ambiguity", obs.satellite_id.name());
                auto f_khz      = m.signal_id.frequency();
                auto wavelength = constant::K_C / (f_khz * 1e3);
                mFilter.state(obs.ambiguity_state_idx) =
                    (m.carrier_phase != 0.0) ? (m.carrier_phase - m.pseudo_range / wavelength) :
                                               0.0;
                mFilter.covariance()(obs.ambiguity_state_idx, obs.ambiguity_state_idx) = 1e6;
            } else if (obs.ambiguity_state_idx < 0) {
                // Add iono state if needed
                if (mConfiguration.ionosphere_mode == PppIonosphereMode::Estimated) {
                    obs.iono_state_idx = mFilter.size();
                    mFilter.add_state(0.0, 1e4);
                }
                // Add ambiguity state — initialize from phase-pseudorange difference
                obs.ambiguity_state_idx = mFilter.size();
                auto f_khz              = m.signal_id.frequency();
                auto wavelength         = constant::K_C / (f_khz * 1e3);
                // N ≈ φ - P/λ  (float ambiguity in cycles, ignoring iono)
                auto n0 = (m.carrier_phase != 0.0) ?
                              (m.carrier_phase - m.pseudo_range / wavelength) :
                              0.0;
                mFilter.add_state(n0, 1e6);
            }
        }

        obs.prev_lock_time = m.lock_time;
    }
}

PppSolution PppEngine::evaluate() NOEXCEPT {
    FUNCTION_SCOPE();

    PppSolution solution{};
    if (mConfiguration.epoch_selection == EpochSelection::FirstObservation) {
        if (mEpochFirstTimeSet) solution = evaluate(mEpochFirstTime);
    } else if (mConfiguration.epoch_selection == EpochSelection::LastObservation) {
        if (mEpochLastTimeSet) solution = evaluate(mEpochLastTime);
    } else if (mConfiguration.epoch_selection == EpochSelection::MeanObservation) {
        if (mEpochObservationCount > 0) {
            auto mean_time = ts::Tai{ts::Timestamp{mEpochTotalObservationTime /
                                                   static_cast<double>(mEpochObservationCount)}};
            solution       = evaluate(mean_time);
        }
    }
    return solution;
}

PppSolution PppEngine::evaluate(ts::Tai time) NOEXCEPT {
    FUNCTION_SCOPEF("%s", time.rtklib_time_string().c_str());

    select_best_observations(time);
    compute_satellite_states(time);

    auto satellite_count = mObservationMask.count();
    if (satellite_count < 4) {
        WARNF("not enough satellites: %zu < 4", satellite_count);
        return PppSolution{};
    }

    if (!mFilterInitialized) {
        initialize_filter();
    } else {
        predict_filter();
    }

    if (mConfiguration.use_carrier_phase) {
        manage_ambiguity_states(time);
    }

    // Build observation vector and design matrix
    auto                             n = mFilter.size();
    std::vector<Scalar>              z_vec, r_diag;
    std::vector<std::vector<Scalar>> H_rows;

    auto ground_position = mFilter.state().head<3>();
    auto ground_llh      = ecef_to_llh(ground_position, ellipsoid::WGS84);
    auto cps             = mCorrectionCache.correction_point_set(ground_llh);

    std::vector<PppSolution::SatelliteDetail> sat_details;

    for (size_t i = 0; i < mObservationMask.size(); ++i) {
        if (!mObservationMask[i]) continue;

        auto& obs = mObservations[i];
        if (obs.selected0 < 0) continue;

        auto& m = obs.measurements[static_cast<size_t>(obs.selected0)];

        auto geometric_range = geometric_distance(obs.true_position, ground_position);
        auto line_of_sight   = (obs.true_position - ground_position).normalized();
        auto enu             = ecef_to_enu_at_llh(ground_llh, line_of_sight);

        LookAngles look_angles;
        if (!compute_look_angles(ground_position, enu, obs.true_position, look_angles)) continue;

        obs.elevation = look_angles.elevation;
        obs.azimuth   = look_angles.azimuth;
        obs.nadir     = look_angles.nadir;

        if (obs.elevation * constant::K_R2D < mConfiguration.elevation_cutoff) continue;

        auto correction = mCorrectionCache.satellite_correction(obs.satellite_id);

        // Satellite clock
        auto sc_bias = constant::K_C * obs.true_clock_bias;
        auto sc_corr = 0.0;
        if (correction && correction->clock_valid) {
            sc_corr = correction->clock.evaluate(obs.time);
        }

        // Code bias
        auto code_bias = 0.0;
        if (correction) {
            auto it = correction->code_bias.find(m.signal_id);
            if (it != correction->code_bias.end()) code_bias = it->second;
        }

        // Ionosphere
        auto f_khz  = m.signal_id.frequency();
        auto f_hz   = f_khz * 1e3;
        auto i_bias = 0.0;
        if (mConfiguration.ionosphere_mode == PppIonosphereMode::None) {
            i_bias = 0.0;
        } else if (mConfiguration.ionosphere_mode == PppIonosphereMode::Ssr && cps) {
            Scalar poly = 0.0, resid = 0.0;
            cps->ionospheric_polynomial(ground_llh, obs.satellite_id, poly);
            cps->ionospheric_residual(ground_llh, obs.satellite_id, resid);
            i_bias = 40.3e16 * (poly + resid) / (f_hz * f_hz);
        }

        // Troposphere mapping (naive: 1/sin(el))
        auto sin_el  = std::sin(obs.elevation);
        auto mapping = (sin_el > 0.01) ? (1.0 / sin_el) : 100.0;

        auto ztd_dry_val = (mIdxZtdDry >= 0) ? mFilter.state(mIdxZtdDry) : 0.0;
        auto ztd_wet_val = (mIdxZtdWet >= 0) ? mFilter.state(mIdxZtdWet) : 0.0;
        auto t_bias      = mapping * (ztd_dry_val + ztd_wet_val);

        // Receiver clock
        auto rc_bias = mFilter.state(kIdxClk);

        auto computed_range =
            geometric_range + rc_bias - sc_bias + sc_corr + code_bias + i_bias + t_bias;
        auto residual = m.pseudo_range - computed_range;

        // Build H row
        std::vector<Scalar> h_row(static_cast<size_t>(n), 0.0);
        h_row[static_cast<size_t>(kIdxX)]   = -line_of_sight.x();
        h_row[static_cast<size_t>(kIdxY)]   = -line_of_sight.y();
        h_row[static_cast<size_t>(kIdxZ)]   = -line_of_sight.z();
        h_row[static_cast<size_t>(kIdxClk)] = 1.0;
        if (mIdxZtdDry >= 0) h_row[static_cast<size_t>(mIdxZtdDry)] = mapping;
        if (mIdxZtdWet >= 0) h_row[static_cast<size_t>(mIdxZtdWet)] = mapping;

        H_rows.push_back(h_row);
        z_vec.push_back(residual);
        r_diag.push_back(mConfiguration.sigma_pseudorange * mConfiguration.sigma_pseudorange);

        // Carrier phase observation: λφ = ρ + rc - sc + T - I + λN
        if (mConfiguration.use_carrier_phase && obs.ambiguity_state_idx >= 0 &&
            m.carrier_phase != 0.0) {
            auto f_khz          = m.signal_id.frequency();
            auto wavelength     = constant::K_C / (f_khz * 1e3);
            auto phase_range    = m.carrier_phase * wavelength;
            auto N_state        = mFilter.state(obs.ambiguity_state_idx);
            auto phase_computed = geometric_range + rc_bias - sc_bias + sc_corr + t_bias - i_bias +
                                  wavelength * N_state;
            auto phase_residual = phase_range - phase_computed;

            std::vector<Scalar> h_phase(static_cast<size_t>(n), 0.0);
            h_phase[static_cast<size_t>(kIdxX)]   = -line_of_sight.x();
            h_phase[static_cast<size_t>(kIdxY)]   = -line_of_sight.y();
            h_phase[static_cast<size_t>(kIdxZ)]   = -line_of_sight.z();
            h_phase[static_cast<size_t>(kIdxClk)] = 1.0;
            if (mIdxZtdDry >= 0) h_phase[static_cast<size_t>(mIdxZtdDry)] = mapping;
            if (mIdxZtdWet >= 0) h_phase[static_cast<size_t>(mIdxZtdWet)] = mapping;
            h_phase[static_cast<size_t>(obs.ambiguity_state_idx)] = wavelength;

            H_rows.push_back(h_phase);
            z_vec.push_back(phase_residual);
            r_diag.push_back(mConfiguration.sigma_carrier_phase *
                             mConfiguration.sigma_carrier_phase);
        }

        PppSolution::SatelliteDetail detail{};
        detail.id                   = obs.satellite_id;
        detail.elevation            = obs.elevation;
        detail.azimuth              = obs.azimuth;
        detail.residual_pseudorange = residual;
        detail.geometric_range      = geometric_range;
        detail.satellite_clock      = sc_bias - sc_corr;
        detail.ionosphere           = i_bias;
        detail.troposphere_dry      = mapping * ztd_dry_val;
        detail.troposphere_wet      = mapping * ztd_wet_val;
        detail.code_bias            = code_bias;
        sat_details.push_back(detail);
    }

    auto m_obs = static_cast<long>(z_vec.size());
    if (m_obs < 4) {
        WARNF("not enough observations after filtering: %ld < 4", m_obs);
        return PppSolution{};
    }

    MatrixX H = MatrixX::Zero(m_obs, n);
    VectorX z = VectorX::Zero(m_obs);
    MatrixX R = MatrixX::Zero(m_obs, m_obs);

    for (long row = 0; row < m_obs; ++row) {
        for (long col = 0; col < n; ++col) {
            H(row, col) = H_rows[static_cast<size_t>(row)][static_cast<size_t>(col)];
        }
        z(row)      = z_vec[static_cast<size_t>(row)];
        R(row, row) = r_diag[static_cast<size_t>(row)];
    }

    mFilter.update(H, R, z);

    // Reset epoch tracking
    mEpochFirstTimeSet         = false;
    mEpochLastTimeSet          = false;
    mEpochObservationCount     = 0;
    mEpochTotalObservationTime = 0.0;
    mLastEpochTime             = time;
    mLastEpochTimeSet          = true;

    auto final_position = mFilter.state().head<3>();
    auto final_llh      = ecef_to_llh(final_position, ellipsoid::WGS84);

    PppSolution solution{};
    solution.time            = time;
    solution.status          = PppSolution::Status::Standard;
    solution.position_ecef   = final_position;
    solution.latitude        = final_llh.x() * constant::K_R2D;
    solution.longitude       = final_llh.y() * constant::K_R2D;
    solution.altitude        = final_llh.z();
    solution.receiver_clock  = mFilter.state(kIdxClk) / constant::K_C;
    solution.ztd_wet         = (mIdxZtdWet >= 0) ? mFilter.state(mIdxZtdWet) : 0.0;
    solution.ztd_dry         = (mIdxZtdDry >= 0) ? mFilter.state(mIdxZtdDry) : 0.0;
    solution.covariance      = mFilter.covariance().topLeftCorner(mBaseStateSize, mBaseStateSize);
    solution.satellites      = std::move(sat_details);
    solution.satellite_count = satellite_count;

    return solution;
}

}  // namespace idokeido

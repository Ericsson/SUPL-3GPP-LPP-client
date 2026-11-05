#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <time/utc.hpp>
#include "ephemeris/bds.hpp"
#include "ephemeris/gal.hpp"
#include "time/bdt.hpp"
#if defined(INCLUDE_GENERATOR_TOKORO)
#include "agnss.hpp"
#include "tokoro.hpp"

#ifdef INCLUDE_FORMAT_ANTEX
#include <format/antex/antex.hpp>
#endif
#include <format/rinex/nav_reader.hpp>
#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>
#include <lpp/session.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-SSR-CorrectionPoints-r16.h>
#include <ProvideAssistanceData-r9-IEs.h>
EXTERNAL_WARNINGS_POP

LOGLET_MODULE2(p, tkr);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, tkr)

static uint32_t tokoro_rtcm_msm_type(TokoroConfig::MsmType msm_type) {
    switch (msm_type) {
    case TokoroConfig::MsmType::MSM4: return 4;
    case TokoroConfig::MsmType::MSM5: return 5;
    case TokoroConfig::MsmType::MSM6: return 6;
    case TokoroConfig::MsmType::MSM7: return 7;
    }

    return 5;
}

//
//
//

void TokoroLocation::inspect(streamline::System&, DataType const& location, uint64_t) {
    VSCOPE_FUNCTION();
    mTokoro.update_location_information(location);
}

//
//
//

Tokoro::Tokoro(ProgramOutput const& output, TokoroConfig const& config,
               scheduler::Scheduler& scheduler)
    : mOutput(output), mConfig(config), mScheduler(scheduler), mSystem(nullptr) {
    VSCOPE_FUNCTION();
    mGenerator = std::unique_ptr<generator::tokoro::Generator>(new generator::tokoro::Generator{});
    mReferenceStation = nullptr;
    mPeriodicTask     = nullptr;
    mOutputTag        = tags::get(mConfig.output_tag).value;

    mGenerator->set_iod_consistency_check(mConfig.iod_consistency_check);
    mGenerator->set_ssr_generation_consistency_check(mConfig.ssr_generation_consistency_check);
    mGenerator->set_rtoc(mConfig.rtoc);
    mGenerator->set_ocit(mConfig.ocit);
    mGenerator->set_ignore_bitmask(mConfig.ignore_bitmask);
    mGenerator->set_iono_polynomial_only(mConfig.iono_polynomial_only);
    if (!mConfig.orbit_frame.empty()) {
        if (!mGenerator->set_orbit_frame(mConfig.orbit_frame.c_str())) {
            ERRORF("invalid --tkr-orbit-frame \"%s\"", mConfig.orbit_frame.c_str());
            exit(1);
        }
        for (auto const& value : mConfig.orbit_source_frames) {
            auto              separator = value.find('=');
            auto              gnss_name = value.substr(0, separator);
            auto              frame     = value.substr(separator + 1);
            SatelliteId::Gnss gnss{};
            if (!generator::tokoro::gnss_from_name(gnss_name.c_str(), gnss) ||
                !mGenerator->set_orbit_source_frame(gnss, frame.c_str())) {
                ERRORF("invalid --tkr-orbit-source-frame \"%s\"", value.c_str());
                exit(1);
            }
        }
    }
    mGenerator->set_ephemeris_max_cache(mConfig.ephemeris_max_cache);
    mGenerator->set_max_orbit_age(mConfig.max_orbit_age);
    mGenerator->set_max_clock_age(mConfig.max_clock_age);
    mGenerator->set_max_bias_age(mConfig.max_bias_age);
    mGenerator->set_max_iono_age(mConfig.max_iono_age);
    mGenerator->set_max_tropo_age(mConfig.max_tropo_age);

    if (mConfig.fake_correction_point_set) {
        auto const& f = *mConfig.fake_correction_point_set;
        mGenerator->set_fake_correction_point_set(
            f.set_id, f.reference_point_latitude, f.reference_point_longitude,
            f.number_of_steps_latitude, f.number_of_steps_longitude, f.step_of_latitude,
            f.step_of_longitude);
    }

    if (!config.gpt2_grid.empty()) {
        if (!mGenerator->troposphere_data().load_gpt2_grid(config.gpt2_grid)) {
            ERRORF("failed to load GPT2 grid file: \"%s\"", config.gpt2_grid.c_str());
        }
    }
    for (auto const& file : config.vmf3_grids) {
        if (!mGenerator->troposphere_data().load_vmf3_grid(file)) {
            ERRORF("failed to load VMF3 grid file: \"%s\"", file.c_str());
        }
    }
    if (!config.vmf3_orography.empty()) {
        if (!mGenerator->troposphere_data().load_vmf3_orography(config.vmf3_orography)) {
            ERRORF("failed to load VMF3 orography file: \"%s\"", config.vmf3_orography.c_str());
        }
    }

#ifdef INCLUDE_FORMAT_ANTEX
    if (!config.antex_file.empty()) {
        auto result = format::antex::Antex::from_file(config.antex_file);
        if (!result) {
            WARNF("failed to load antex file: \"%s\"", config.antex_file.c_str());
        } else {
            mGenerator->set_antex(std::move(result));
        }
    }
#endif

    if (!config.nav_files.empty()) {
        for (auto const& nav_file : config.nav_files) {
            auto nav = format::rinex::parse_nav_file(nav_file);
            for (auto& eph : nav.gps)
                mGenerator->process_ephemeris(eph);
            for (auto& eph : nav.gal)
                mGenerator->process_ephemeris(eph);
            for (auto& eph : nav.bds)
                mGenerator->process_ephemeris(eph);
            for (auto& eph : nav.glo)
                mGenerator->process_ephemeris(eph);
            for (auto& eph : nav.qzs)
                mGenerator->process_ephemeris(eph);
        }
    }

    if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::AssistanceData) {
        // Nothing to do, the generator will generate when assistance data is received
    } else if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::TimeStep ||
               mConfig.generation_strategy == TokoroConfig::GenerationStrategy::TimeStepAligned ||
               mConfig.generation_strategy == TokoroConfig::GenerationStrategy::TimeStepLast) {
        // Setup a periodic timer to generate every time step
        auto interval = std::chrono::milliseconds(static_cast<int>(mConfig.time_step * 1000.0));
        mPeriodicTask =
            std::unique_ptr<scheduler::PeriodicTask>(new scheduler::PeriodicTask(interval));
        mPeriodicTask->callback = [this]() {
            ASSERT(mGenerator, "generator is null");
            if (!mSystem) return;
            if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::TimeStepLast) {
                generate(mGenerator->last_correction_data_time());
            } else if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::TimeStep) {
                generate(ts::Tai::now());
            } else if (mConfig.generation_strategy ==
                       TokoroConfig::GenerationStrategy::TimeStepAligned) {
                auto now          = ts::Tai::now();
                auto full_seconds = now.timestamp().full_seconds();
                auto aligned_full_seconds =
                    std::floor(full_seconds / mConfig.time_step) * mConfig.time_step;
                auto aligned      = ts::Timestamp{aligned_full_seconds};
                auto aligned_time = ts::Tai{aligned};
                generate(aligned_time);
            } else {
                WARNF("unsupported generation strategy");
            }

            auto missing = mGenerator->missing_ephemeris();
            for (auto& entry : missing) {
                MissingEphemeris msg{entry.first, entry.second};
                mSystem->push(std::move(msg));
            }
        };

        if (!mPeriodicTask->schedule(mScheduler)) {
            WARNF("failed to schedule periodic task");
        }
    } else {
        WARNF("unsupported generation strategy");
    }
}

Tokoro::~Tokoro() {
    VSCOPE_FUNCTION();
}

bool Tokoro::is_gps_enabled() {
    return mConfig.generate_gps;
}

bool Tokoro::is_gal_enabled() {
    return mConfig.generate_galileo;
}

bool Tokoro::is_bds_enabled() {
    return mConfig.generate_beidou;
}

void Tokoro::update_location_information(lpp::LocationInformation location_information) NOEXCEPT {
    VSCOPE_FUNCTION();
    mLastLocation = std::move(location_information.location);
}

void Tokoro::process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    mGenerator->process_ephemeris(ephemeris);
}

void Tokoro::process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    mGenerator->process_ephemeris(ephemeris);
}

void Tokoro::process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    mGenerator->process_ephemeris(ephemeris);
}

void Tokoro::process_ephemeris(ephemeris::GloEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    mGenerator->process_ephemeris(ephemeris);
}

void Tokoro::process_ephemeris(ephemeris::QzsEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    mGenerator->process_ephemeris(ephemeris);
}

void Tokoro::vrs_mode_fixed() {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    if (!mReferenceStation) {
        Float3 itrf_position{mConfig.fixed_itrf_x, mConfig.fixed_itrf_y, mConfig.fixed_itrf_z};
        Float3 rtcm_position{mConfig.fixed_rtcm_x, mConfig.fixed_rtcm_y, mConfig.fixed_rtcm_z};

        mReferenceStation =
            mGenerator->define_reference_station(generator::tokoro::ReferenceStationConfig{
                itrf_position,
                rtcm_position,
                mConfig.generate_gps,
                mConfig.generate_glonass,
                mConfig.generate_galileo,
                mConfig.generate_beidou,
                false,
            });

        if (mConfig.fixed_physical_station_set) {
            Float3 physical_position{mConfig.fixed_physical_x, mConfig.fixed_physical_y,
                                     mConfig.fixed_physical_z};
            mReferenceStation->set_physical_ground_position(physical_position);
        }
    }
}

void Tokoro::vrs_mode_grid() {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    if (!mReferenceStation) {
        double lat, lon;
        int    east  = mConfig.vrs_grid_position->first;
        int    north = mConfig.vrs_grid_position->second;

        if (!mGenerator->get_grid_cell_center_position(east, north, &lat, &lon)) {
            WARNF("no correction point set available for grid mode");
            return;
        }

        Float3 llh{lat * generator::tokoro::constant::DEG2RAD,
                   lon * generator::tokoro::constant::DEG2RAD, 0.0};
        Float3 position = generator::tokoro::llh_to_ecef(llh, generator::tokoro::ellipsoid::gWgs84);

        INFOF("VRS grid cell center (%d,%d): lat=%.6f lon=%.6f", east, north, lat, lon);

        mReferenceStation =
            mGenerator->define_reference_station(generator::tokoro::ReferenceStationConfig{
                position,
                position,
                mConfig.generate_gps,
                mConfig.generate_glonass,
                mConfig.generate_galileo,
                mConfig.generate_beidou,
                false,
            });
    }
}

static Float3 llh_from_shape(lpp::LocationShape const& shape) {
    auto latitude  = shape.latitude();
    auto longitude = shape.longitude();
    auto altitude  = shape.altitude();

    return Float3{
        latitude * generator::tokoro::constant::DEG2RAD,
        longitude * generator::tokoro::constant::DEG2RAD,
        altitude,
    };
}

void Tokoro::vrs_mode_dynamic() {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");

    if (mConfig.dynamic_distance_threshold <= 0) {
        // No threshold, generate a new VRS every time
        mReferenceStation.reset();
    } else if (mLastUsedLocation.has_value() && mLastLocation.has_value()) {
        auto const& current_shape = mLastLocation.const_value();
        auto const& last_shape    = mLastUsedLocation.const_value();

        auto current_llh = llh_from_shape(current_shape);
        auto last_llh    = llh_from_shape(last_shape);

        auto current_position =
            generator::tokoro::llh_to_ecef(current_llh, generator::tokoro::ellipsoid::gWgs84);
        auto last_position =
            generator::tokoro::llh_to_ecef(last_llh, generator::tokoro::ellipsoid::gWgs84);

        auto distance_km = (current_position - last_position).length() / 1000.0;
        DEBUGF("distance to last used location: %.6fkm (%.6fkm threshold)", distance_km,
               mConfig.dynamic_distance_threshold);
        if (distance_km > mConfig.dynamic_distance_threshold) {
            // We have moved far enough to generate a new VRS, discard the last VRS and let the code
            // below generate a new one.
            mReferenceStation.reset();
        }
    }

    if (!mReferenceStation) {
        if (!mLastLocation.has_value()) {
            WARNF("must have a rough location to generate a VRS");
            return;
        }

        // We don't track which coordinate system the location is in, however, that doesn't matter
        // because (1) we only need a rough location and (2) the it will probably be in gWgs84,
        // which is aligned with ITRF current, and that's what we use for the reference station.
        auto const& last_location_shape = mLastLocation.const_value();
        auto        last_location_llh   = llh_from_shape(last_location_shape);
        INFOF("last location (llh): (%.14f,%.14f,%.6f)",
              last_location_llh.x * generator::tokoro::constant::RAD2DEG,
              last_location_llh.y * generator::tokoro::constant::RAD2DEG, last_location_llh.z);

        auto last_location =
            generator::tokoro::llh_to_ecef(last_location_llh, generator::tokoro::ellipsoid::gWgs84);
        INFOF("last location (ecef): (%.14f,%.14f,%.14f)", last_location.x, last_location.y,
              last_location.z);

        mReferenceStation =
            mGenerator->define_reference_station(generator::tokoro::ReferenceStationConfig{
                last_location,
                last_location,
                mConfig.generate_gps,
                mConfig.generate_glonass,
                mConfig.generate_galileo,
                mConfig.generate_beidou,
                false,
            });

        mLastUsedLocation = mLastLocation;
    }
}

void Tokoro::generate(ts::Tai const& generation_time) {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");

    if (generation_time.timestamp().full_seconds() == 0) return;

    if (mConfig.deduplicate_epochs && generation_time == mLastGenerationTime) return;
    mLastGenerationTime = generation_time;

    auto vrs_start = std::chrono::steady_clock::now();
    if (mConfig.vrs_mode == TokoroConfig::VrsMode::Fixed) {
        vrs_mode_fixed();
    } else if (mConfig.vrs_mode == TokoroConfig::VrsMode::Grid) {
        vrs_mode_grid();
    } else if (mConfig.vrs_mode == TokoroConfig::VrsMode::Dynamic) {
        vrs_mode_dynamic();
    } else {
        WARNF("unsupported VRS mode");
        return;
    }
    auto vrs_end = std::chrono::steady_clock::now();
    auto vrs_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(vrs_end - vrs_start).count();
    VERBOSEF("VRS mode setup took %lld ms", vrs_ms);

    if (!mReferenceStation) {
        WARNF("reference station is null");
        return;
    }

    mReferenceStation->set_shapiro_correction(mConfig.shapiro_correction);
    mReferenceStation->set_antenna_phase_variation_correction(
        mConfig.antenna_phase_variation_correction);
    mReferenceStation->set_earth_solid_tides_correction(mConfig.earth_solid_tides_correction);
    mReferenceStation->set_phase_windup_correction(mConfig.phase_windup_correction);
    mReferenceStation->set_tropospheric_height_correction(mConfig.tropospheric_height_correction);
    mReferenceStation->set_tropospheric_hydrostatic_delta(mConfig.tropospheric_hydrostatic_delta);
    mReferenceStation->set_elevation_mask(mConfig.elevation_mask);
    mReferenceStation->set_msm_type(tokoro_rtcm_msm_type(mConfig.msm_type));
    mReferenceStation->set_negative_phase_windup(mConfig.negative_phase_windup);
    mReferenceStation->set_negative_correction_sign(mConfig.negative_correction_sign);
    mReferenceStation->set_phase_alignment(mConfig.phase_alignment);
    mReferenceStation->set_tropospheric_model(mConfig.tropospheric_model);
    mReferenceStation->set_tropospheric_mapping(mConfig.tropospheric_mapping);
    mReferenceStation->set_tropospheric_gradient(mConfig.tropospheric_gradient,
                                                 mConfig.tropospheric_gradient_mapping);
    if (mConfig.tropospheric_gradient_constant_set) {
        mReferenceStation->set_tropospheric_gradient_constant(mConfig.tropospheric_gradient_north,
                                                              mConfig.tropospheric_gradient_east);
    }
    mReferenceStation->set_lock_start(mConfig.lock_start == TokoroConfig::LockStart::Max ?
                                          generator::tokoro::LockStart::Max :
                                          generator::tokoro::LockStart::Reset);
    mReferenceStation->set_lock_rise(mConfig.lock_rise == TokoroConfig::LockStart::Max ?
                                         generator::tokoro::LockStart::Max :
                                         generator::tokoro::LockStart::Reset);
    switch (mConfig.ambiguity_randomization) {
    case TokoroConfig::AmbiguityRandomization::Off:
        mReferenceStation->set_ambiguity_randomization(
            generator::tokoro::AmbiguityRandomization::Off);
        break;
    case TokoroConfig::AmbiguityRandomization::Startup:
        mReferenceStation->set_ambiguity_randomization(
            generator::tokoro::AmbiguityRandomization::Startup);
        break;
    case TokoroConfig::AmbiguityRandomization::Rise:
        mReferenceStation->set_ambiguity_randomization(
            generator::tokoro::AmbiguityRandomization::Rise);
        break;
    case TokoroConfig::AmbiguityRandomization::Both:
        mReferenceStation->set_ambiguity_randomization(
            generator::tokoro::AmbiguityRandomization::Both);
        break;
    }
    mReferenceStation->set_ambiguity_seed(mConfig.ambiguity_seed);
    mReferenceStation->set_antenna_descriptor(mConfig.antenna_descriptor);
    mReferenceStation->set_receiver_descriptor(mConfig.receiver_descriptor);
#ifdef INCLUDE_FORMAT_RINEX
    mReferenceStation->set_generate_rinex(mConfig.generate_rinex);
#endif
    mReferenceStation->set_require_code_bias(mConfig.require_code_bias);
    mReferenceStation->set_require_phase_bias(mConfig.require_phase_bias);
    mReferenceStation->set_require_tropo(mConfig.require_tropo);
    mReferenceStation->set_require_iono(mConfig.require_iono);
    mReferenceStation->set_use_tropospheric_model(mConfig.use_tropospheric_model);
    mReferenceStation->set_use_ionospheric_height_correction(
        mConfig.use_ionospheric_height_correction);
    mReferenceStation->set_generate_residuals(mConfig.generate_residuals);

    if (!mConfig.diag_output_dir.empty()) {
        mReferenceStation->set_diag_output(mConfig.diag_output_dir);
        mReferenceStation->set_diag_tropospheric_all(mConfig.diag_tropospheric_all);
    }

#if defined(ENABLE_TOKORO_SNAPSHOT)
    if (mRecorder && mRecorder->should_record()) {
        auto snapshot = mReferenceStation->snapshot(generation_time);
        mRecorder->record(snapshot);
    }
#endif

    auto gen_start = std::chrono::steady_clock::now();
    mReferenceStation->generate(generation_time);
    auto gen_end = std::chrono::steady_clock::now();
    auto gen_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(gen_end - gen_start).count();
    VERBOSEF("reference station generate took %lld ms", gen_ms);

    auto messages = mReferenceStation->produce();
    if (!messages.empty()) {
        std::string ids;
        for (auto& m : messages) {
            if (!ids.empty()) ids += ',';
            ids += std::to_string(m.id());
        }
        INFOF("generated %zu RTCM message(s) @ %s: [%s]", messages.size(),
              ts::Utc(generation_time).rtklib_time_string().c_str(), ids.c_str());
    }
    DEBUG_INDENT_SCOPE();
    for (auto& submessage : messages) {
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        DEBUGF("message: %4u: %zu bytes", submessage.id(), size);

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOutput.outputs) {
            if (!output.rtcm_support()) continue;
            if (!output.accept_tag(mOutputTag)) {
                XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %s not accepted: %s",
                          output.tag_name(mOutputTag).c_str(),
                          output.reject_reason(mOutputTag).c_str());
                continue;
            }
            XDEBUGF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes) tag=%llX", submessage.id(), size,
                    mOutputTag);

            ASSERT(output.stage, "stage is null");
            output.stage->write(OUTPUT_FORMAT_RTCM, buffer, size);
        }
    }
}

void Tokoro::inspect(streamline::System& system, DataType const& message, uint64_t) {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");

    mSystem = &system;

    if (!message) {
        return;
    }

    auto process_start       = std::chrono::steady_clock::now();
    auto new_assistance_data = mGenerator->process_lpp(*message.get());
    auto process_end         = std::chrono::steady_clock::now();
    auto process_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(process_end - process_start).count();
    VERBOSEF("process_lpp took %lld ms", process_ms);

    if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::AssistanceData) {
        if (new_assistance_data) {
            auto gen_start = std::chrono::steady_clock::now();
            generate(mGenerator->last_correction_data_time());
            auto gen_end = std::chrono::steady_clock::now();
            auto gen_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(gen_end - gen_start).count();
            VERBOSEF("generate took %lld ms", gen_ms);
        } else {
            DEBUGF("skipping generation, no new assistance data");
        }
    }

    auto missing = mGenerator->missing_ephemeris();
    for (auto& entry : missing) {
        MissingEphemeris msg{entry.first, entry.second};
        system.push(std::move(msg));
    }
}

#endif

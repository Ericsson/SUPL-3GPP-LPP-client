#include <cstdint>
#include "ephemeris/bds.hpp"
#include "ephemeris/gal.hpp"
#include "time/bdt.hpp"
#if defined(INCLUDE_GENERATOR_TOKORO)
#include "agnss.hpp"
#include "tokoro.hpp"

#include <format/antex/antex.hpp>
#include <format/rtcm/datafields.hpp>
#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, tkr);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, tkr)

void TokoroEphemerisUbx::handle_gps_lnav(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());

    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) {
        WARNF("failed to decode GPS LNAV subframe");
        return;
    }

    ephemeris::GpsEphemeris ephemeris{};
    if (!mGpsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) {
        return;
    }

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisUbx::handle_gps(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gps_enabled()) return;
    if (sfrbx->sig_id() == 0) {
        handle_gps_lnav(sfrbx);
    } else {
        VERBOSEF("unsupported GPS signal id %d", sfrbx->sig_id());
    }
}

void TokoroEphemerisUbx::handle_gal_inav(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());

    format::nav::gal::InavWord word{};
    if (!format::nav::gal::InavWord::decode(words, word)) {
        WARNF("failed to decode GAL INAV word");
        return;
    }

    ephemeris::GalEphemeris ephemeris{};
    if (!mGalCollector.process(sfrbx->sv_id(), word, ephemeris)) return;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisUbx::handle_gal(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gal_enabled()) return;
    if (sfrbx->sig_id() == 5) {
        handle_gal_inav(sfrbx);
    } else {
        VERBOSEF("unsupported GAL signal id %d", sfrbx->sig_id());
    }
}

void TokoroEphemerisUbx::handle_bds_d1(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());

    format::nav::D1Subframe subframe{};
    if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) {
        WARNF("failed to decode BDS D1 subframe");
        return;
    }

    ephemeris::BdsEphemeris ephemeris{};
    if (!mBdsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisUbx::handle_bds(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_bds_enabled()) return;
    if (sfrbx->sig_id() == 0) {
        handle_bds_d1(sfrbx);
    } else {
        VERBOSEF("unsupported BDS signal id %d", sfrbx->sig_id());
    }
}

void TokoroEphemerisUbx::inspect(streamline::System&, DataType const& message, uint64_t) {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto sfrbx = dynamic_cast<format::ubx::RxmSfrbx*>(ptr);
    if (!sfrbx) return;

    if (sfrbx->gnss_id() == 0) {
        handle_gps(sfrbx);
    } else if (sfrbx->gnss_id() == 2) {
        handle_gal(sfrbx);
    } else if (sfrbx->gnss_id() == 3) {
        handle_bds(sfrbx);
    }
}

//
//
//

void TokoroEphemerisRtcm::handle_gps_lnav(format::rtcm::Rtcm1019* rtcm) {
    VSCOPE_FUNCTION();

    ephemeris::GpsEphemeris ephemeris{};
    ephemeris.prn               = rtcm->prn;
    ephemeris.week_number       = rtcm->week;
    ephemeris.ca_or_p_on_l2     = rtcm->code_on_l2;
    ephemeris.ura_index         = rtcm->SV_ACCURACY;
    ephemeris.sv_health         = rtcm->SV_HEALTH;
    ephemeris.lpp_iod           = rtcm->iode;
    ephemeris.iodc              = rtcm->iodc;
    ephemeris.iode              = rtcm->iode;
    ephemeris.aodo              = 0;
    ephemeris.toc               = rtcm->t_oc;
    ephemeris.toe               = rtcm->t_oe;
    ephemeris.tgd               = rtcm->t_GD;
    ephemeris.af2               = rtcm->a_f2;
    ephemeris.af1               = rtcm->a_f1;
    ephemeris.af0               = rtcm->a_f0;
    ephemeris.crc               = rtcm->C_rc;
    ephemeris.crs               = rtcm->C_rs;
    ephemeris.cuc               = rtcm->C_uc;
    ephemeris.cus               = rtcm->C_us;
    ephemeris.cic               = rtcm->C_ic;
    ephemeris.cis               = rtcm->C_is;
    ephemeris.e                 = rtcm->e;
    ephemeris.m0                = rtcm->M_0;
    ephemeris.delta_n           = rtcm->delta_n;
    ephemeris.a                 = rtcm->sqrt_A * rtcm->sqrt_A;
    ephemeris.i0                = rtcm->i_0;
    ephemeris.omega0            = rtcm->OMEGA_0;
    ephemeris.omega             = rtcm->omega;
    ephemeris.omega_dot         = rtcm->OMEGADOT;
    ephemeris.idot              = rtcm->idot;
    ephemeris.fit_interval_flag = rtcm->fit;
    ephemeris.l2_p_data_flag    = rtcm->L2_P_data_flag;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_gps(format::rtcm::Rtcm1019* rtcm) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gps_enabled()) return;
    if (rtcm->type() == 1019) {
        handle_gps_lnav(rtcm);
    } else {
        VERBOSEF("not rtcm 1019 but rtcm %d", rtcm->type());
    }
}

void TokoroEphemerisRtcm::handle_bds_d1(format::rtcm::Rtcm1042* rtcm) {
    VSCOPE_FUNCTION();

    ephemeris::BdsEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;  // Could have something to do with sow???
    ephemeris.sv_health   = rtcm->sv_health;
    ephemeris.lpp_iod     = static_cast<uint16_t>(static_cast<uint32_t>(rtcm->toe) >> 9);
    ephemeris.iodc        = (static_cast<uint32_t>(rtcm->toc) / 720) % 240;
    ephemeris.iode        = (static_cast<uint32_t>(rtcm->toe) / 720) % 240;
    ephemeris.aode        = rtcm->aode;
    ephemeris.aodc        = rtcm->aodc;
    ephemeris.toc_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toc), 0.0);
    ephemeris.toe_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toe), 0.0);
    ephemeris.toc       = rtcm->toc;
    ephemeris.toe       = rtcm->toe;
    ephemeris.af2       = rtcm->af2;
    ephemeris.af1       = rtcm->af1;
    ephemeris.af0       = rtcm->af0;
    ephemeris.crc       = rtcm->crc;
    ephemeris.crs       = rtcm->crs;
    ephemeris.cuc       = rtcm->cuc;
    ephemeris.cus       = rtcm->cus;
    ephemeris.cic       = rtcm->cic;
    ephemeris.cis       = rtcm->cis;
    ephemeris.e         = rtcm->e;
    ephemeris.m0        = rtcm->m0;
    ephemeris.delta_n   = rtcm->delta_n;
    ephemeris.a         = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0        = rtcm->i0;
    ephemeris.omega0    = rtcm->omega0;
    ephemeris.omega     = rtcm->omega;
    ephemeris.omega_dot = rtcm->omega_dot;
    ephemeris.idot      = rtcm->idot;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_bds(format::rtcm::Rtcm1042* rtcm) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gal_enabled()) return;
    if (rtcm->type() == 1042) {
        handle_bds_d1(rtcm);
    } else {
        VERBOSEF("not rtcm 1042 but rtcm %d", rtcm->type());
    }
}

void TokoroEphemerisRtcm::handle_gal_inav(format::rtcm::Rtcm1046* rtcm) {
    VSCOPE_FUNCTION();

    ephemeris::GalEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;
    ephemeris.lpp_iod     = rtcm->iod_nav;
    ephemeris.iod_nav     = rtcm->iod_nav;

    ephemeris.toc = rtcm->toc;
    ephemeris.toe = rtcm->toe;

    ephemeris.af2 = rtcm->af2;
    ephemeris.af1 = rtcm->af1;
    ephemeris.af0 = rtcm->af0;

    ephemeris.crc = rtcm->crc;
    ephemeris.crs = rtcm->crs;
    ephemeris.cuc = rtcm->cuc;
    ephemeris.cus = rtcm->cus;
    ephemeris.cic = rtcm->cic;
    ephemeris.cis = rtcm->cis;

    ephemeris.e       = rtcm->e;
    ephemeris.m0      = rtcm->m0;
    ephemeris.delta_n = rtcm->delta_n;
    ephemeris.a       = rtcm->sqrt_a * rtcm->sqrt_a;

    ephemeris.i0        = rtcm->i0;
    ephemeris.omega0    = rtcm->omega0;
    ephemeris.omega     = rtcm->omega;
    ephemeris.omega_dot = rtcm->omega_dot;
    ephemeris.idot      = rtcm->idot;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_gal(format::rtcm::Rtcm1046* rtcm) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gal_enabled()) return;
    if (rtcm->type() == 1046) {
        handle_gal_inav(rtcm);
    } else {
        VERBOSEF("not rtcm 1046 but rtcm %d", rtcm->type());
    }
}

void TokoroEphemerisRtcm::inspect(streamline::System&, DataType const& message, uint64_t) {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto rtcm1019 = dynamic_cast<format::rtcm::Rtcm1019*>(ptr);
    if (rtcm1019) return handle_gps(rtcm1019);
    auto rtcm1042 = dynamic_cast<format::rtcm::Rtcm1042*>(ptr);
    if (rtcm1042) return handle_bds(rtcm1042);
    auto rtcm1046 = dynamic_cast<format::rtcm::Rtcm1046*>(ptr);
    if (rtcm1046) return handle_gal(rtcm1046);
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

Tokoro::Tokoro(OutputConfig const& output, TokoroConfig const& config,
               scheduler::Scheduler& scheduler)
    : mOutput(output), mConfig(config), mScheduler(scheduler), mSystem(nullptr) {
    VSCOPE_FUNCTION();
    mGenerator = std::unique_ptr<generator::tokoro::Generator>(new generator::tokoro::Generator{});
    mReferenceStation = nullptr;
    mPeriodicTask     = nullptr;
    mOutputTag        = 0;

    mGenerator->set_iod_consistency_check(mConfig.iod_consistency_check);
    mGenerator->set_rtoc(mConfig.rtoc);
    mGenerator->set_ocit(mConfig.ocit);
    mGenerator->set_ignore_bitmask(mConfig.ignore_bitmask);

    if (!config.antex_file.empty()) {
        auto result = format::antex::Antex::from_file(config.antex_file);
        if (!result) {
            WARNF("failed to load antex file: \"%s\"", config.antex_file.c_str());
        } else {
            mGenerator->set_antex(std::move(result));
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
            ASSERT(mSystem, "system is null");
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
            });
    }
}

void Tokoro::vrs_mode_grid() {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
    if (!mReferenceStation) {
        double lat, lon;
        int    east  = mConfig.vrs_grid_position->first;
        int    north = mConfig.vrs_grid_position->second;

        if (!mGenerator->get_grid_position(east, north, &lat, &lon)) {
            WARNF("no correction point set available for grid mode");
            return;
        }

        Float3 llh{lat * generator::tokoro::constant::DEG2RAD,
                   lon * generator::tokoro::constant::DEG2RAD, 0.0};
        Float3 position = generator::tokoro::llh_to_ecef(llh, generator::tokoro::ellipsoid::WGS84);

        INFOF("VRS grid position (%d,%d): lat=%.6f lon=%.6f", east, north, lat, lon);

        mReferenceStation =
            mGenerator->define_reference_station(generator::tokoro::ReferenceStationConfig{
                position,
                position,
                mConfig.generate_gps,
                mConfig.generate_glonass,
                mConfig.generate_galileo,
                mConfig.generate_beidou,
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
            generator::tokoro::llh_to_ecef(current_llh, generator::tokoro::ellipsoid::WGS84);
        auto last_position =
            generator::tokoro::llh_to_ecef(last_llh, generator::tokoro::ellipsoid::WGS84);

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
        // because (1) we only need a rough location and (2) the it will probably be in WGS84, which
        // is aligned with ITRF current, and that's what we use for the reference station.
        auto const& last_location_shape = mLastLocation.const_value();
        auto        last_location_llh   = llh_from_shape(last_location_shape);
        INFOF("last location (llh): (%.14f,%.14f,%.6f)",
              last_location_llh.x * generator::tokoro::constant::RAD2DEG,
              last_location_llh.y * generator::tokoro::constant::RAD2DEG, last_location_llh.z);

        auto last_location =
            generator::tokoro::llh_to_ecef(last_location_llh, generator::tokoro::ellipsoid::WGS84);
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
            });

        mLastUsedLocation = mLastLocation;
    }
}

void Tokoro::generate(ts::Tai const& generation_time) {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");
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

    if (!mReferenceStation) {
        WARNF("reference station is null");
        return;
    }

    mReferenceStation->set_shaprio_correction(mConfig.shapiro_correction);
    mReferenceStation->set_antenna_phase_variation_correction(
        mConfig.antenna_phase_variation_correction);
    mReferenceStation->set_earth_solid_tides_correction(mConfig.earth_solid_tides_correction);
    mReferenceStation->set_phase_windup_correction(mConfig.phase_windup_correction);
    mReferenceStation->set_tropospheric_height_correction(mConfig.tropospheric_height_correction);
    mReferenceStation->set_negative_phase_windup(mConfig.negative_phase_windup);
    mReferenceStation->set_generate_rinex(mConfig.generate_rinex);
    mReferenceStation->set_require_code_bias(mConfig.require_code_bias);
    mReferenceStation->set_require_phase_bias(mConfig.require_phase_bias);
    mReferenceStation->set_require_tropo(mConfig.require_tropo);
    mReferenceStation->set_require_iono(mConfig.require_iono);
    mReferenceStation->set_use_tropospheric_model(mConfig.use_tropospheric_model);
    mReferenceStation->set_use_ionospheric_height_correction(
        mConfig.use_ionospheric_height_correction);

    mReferenceStation->generate(generation_time);

    auto messages = mReferenceStation->produce();
    INFOF("generated %d RTCM messages", messages.size());
    DEBUG_INDENT_SCOPE();
    for (auto& submessage : messages) {
        auto buffer = submessage.data().data();
        auto size   = submessage.data().size();
        DEBUGF("message: %4u: %zu bytes", submessage.id(), size);

        // TODO(ewasjon): These message should be passed back into the system
        for (auto const& output : mOutput.outputs) {
            if (!output.rtcm_support()) continue;
            if (!output.accept_tag(mOutputTag)) {
                XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", mOutputTag);
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

    auto new_assistance_data = mGenerator->process_lpp(*message.get());
    if (mConfig.generation_strategy == TokoroConfig::GenerationStrategy::AssistanceData) {
        if (new_assistance_data) {
            generate(mGenerator->last_correction_data_time());
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

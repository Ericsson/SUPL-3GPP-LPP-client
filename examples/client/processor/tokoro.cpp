#include "ephemeris/bds.hpp"
#include "ephemeris/gal.hpp"
#if defined(INCLUDE_GENERATOR_TOKORO)
#include "tokoro.hpp"

#include <format/antex/antex.hpp>
#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>
#include <format/rtcm/datafields.hpp>

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

void TokoroEphemerisUbx::inspect(streamline::System&, DataType const& message) {
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

void TokoroEphemerisRtcm::handle_gps_lnav(format::rtcm::Rtcm1019Message* rtcm_message) {
    VSCOPE_FUNCTION();

    ephemeris::GpsEphemeris ephemeris{};
    ephemeris.prn = rtcm_message->prn;
    ephemeris.week_number = rtcm_message->week;
    ephemeris.ca_or_p_on_l2 = rtcm_message->code_on_l2;
    ephemeris.ura_index = rtcm_message->SV_ACCURACY;
    ephemeris.sv_health = rtcm_message->SV_HEALTH;
    ephemeris.lpp_iod = rtcm_message->iode;
    ephemeris.iodc = rtcm_message->iodc;
    ephemeris.iode = rtcm_message->iode;
    ephemeris.aodo = 0;
    ephemeris.toc = rtcm_message->t_oc;
    ephemeris.toe = rtcm_message->t_oe;
    ephemeris.tgd = rtcm_message->t_GD;
    ephemeris.af2 = rtcm_message->a_f2;
    ephemeris.af1 = rtcm_message->a_f1;
    ephemeris.af0 = rtcm_message->a_f0;
    ephemeris.crc = rtcm_message->C_rc;
    ephemeris.crs = rtcm_message->C_rs;
    ephemeris.cuc = rtcm_message->C_uc;
    ephemeris.cus = rtcm_message->C_us;
    ephemeris.cic = rtcm_message->C_ic;
    ephemeris.cis = rtcm_message->C_is;
    ephemeris.e = rtcm_message->e;
    ephemeris.m0 = rtcm_message->M_0;
    ephemeris.delta_n = rtcm_message->dn; // Really should rename this one
    ephemeris.a = rtcm_message->sqrt_A * rtcm_message->sqrt_A;
    ephemeris.i0 = rtcm_message->i_0;
    ephemeris.omega0 = rtcm_message->OMEGA_0;
    ephemeris.omega = rtcm_message->omega;
    ephemeris.omega_dot = rtcm_message->OMEGADOT;
    ephemeris.idot = rtcm_message->idot;
    ephemeris.fit_interval_flag = rtcm_message->fit;
    ephemeris.l2_p_data_flag = rtcm_message->L2_P_data_flag;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_gps(format::rtcm::Rtcm1019Message* rtcm_message) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gps_enabled()) return;
    if (rtcm_message->type() == 1019) {
        handle_gps_lnav(rtcm_message);
    } else {
        VERBOSEF("not rtcm 1019 but rtcm %d", rtcm_message-> type());
    }
}

void TokoroEphemerisRtcm::handle_bds_d1(format::rtcm::Rtcm1042Message* rtcm_message) {
    VSCOPE_FUNCTION();

    ephemeris::BdsEphemeris ephemeris{};
    ephemeris.prn = rtcm_message->prn;
    ephemeris.week_number = rtcm_message->week;
    ephemeris.ca_or_p_on_l2 = rtcm_message->code_on_l2;
    ephemeris.ura_index = rtcm_message->SV_ACCURACY;
    ephemeris.sv_health = rtcm_message->SV_HEALTH;
    ephemeris.lpp_iod = rtcm_message->iode;
    ephemeris.iodc = rtcm_message->iodc;
    ephemeris.iode = rtcm_message->iode;
    ephemeris.aodo = 0;
    ephemeris.toc = rtcm_message->t_oc;
    ephemeris.toe = rtcm_message->t_oe;
    ephemeris.tgd = rtcm_message->t_GD;
    ephemeris.af2 = rtcm_message->a_f2;
    ephemeris.af1 = rtcm_message->a_f1;
    ephemeris.af0 = rtcm_message->a_f0;
    ephemeris.crc = rtcm_message->C_rc;
    ephemeris.crs = rtcm_message->C_rs;
    ephemeris.cuc = rtcm_message->C_uc;
    ephemeris.cus = rtcm_message->C_us;
    ephemeris.cic = rtcm_message->C_ic;
    ephemeris.cis = rtcm_message->C_is;
    ephemeris.e = rtcm_message->e;
    ephemeris.m0 = rtcm_message->M_0;
    ephemeris.delta_n = rtcm_message->dn; // Really should rename this one
    ephemeris.a = rtcm_message->sqrt_A * rtcm_message->sqrt_A;
    ephemeris.i0 = rtcm_message->i_0;
    ephemeris.omega0 = rtcm_message->OMEGA_0;
    ephemeris.omega = rtcm_message->omega;
    ephemeris.omega_dot = rtcm_message->OMEGADOT;
    ephemeris.idot = rtcm_message->idot;
    ephemeris.fit_interval_flag = rtcm_message->fit;
    ephemeris.l2_p_data_flag = rtcm_message->L2_P_data_flag;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_bds(format::rtcm::Rtcm1042Message* rtcm_message) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gal_enabled()) return;
    if (rtcm_message->type() == 1042) {
        handle_bds_d1(rtcm_message);
    } else {
        VERBOSEF("not rtcm 1042 but rtcm %d", rtcm_message-> type());
    }
}

void TokoroEphemerisRtcm::handle_gal_inav(format::rtcm::Rtcm1046Message* rtcm_message) {
    VSCOPE_FUNCTION();

    ephemeris::GalEphemeris ephemeris{};
    ephemeris.prn = rtcm_message->prn;
    ephemeris.week_number = rtcm_message->week;
    ephemeris.lpp_iod = rtcm_message->iode;
    ephemeris.iod_nav = rtcm_message->iod_nav;
    ephemeris.lpp_iod = rtcm_message->iod_nav;
    ephemeris.toc = rtcm_message->t_oc;
    ephemeris.toe = rtcm_message->t_oe;
    ephemeris.af2 = rtcm_message->a_f2;
    ephemeris.af1 = rtcm_message->a_f1;
    ephemeris.af0 = rtcm_message->a_f0;
    ephemeris.crc = rtcm_message->C_rc;
    ephemeris.crs = rtcm_message->C_rs;
    ephemeris.cuc = rtcm_message->C_uc;
    ephemeris.cus = rtcm_message->C_us;
    ephemeris.cic = rtcm_message->C_ic;
    ephemeris.cis = rtcm_message->C_is;
    ephemeris.e = rtcm_message->e;
    ephemeris.m0 = rtcm_message->M_0;
    ephemeris.delta_n = rtcm_message->dn; // Really should rename this one
    ephemeris.a = rtcm_message->sqrt_A * rtcm_message->sqrt_A;
    ephemeris.i0 = rtcm_message->i_0;
    ephemeris.omega0 = rtcm_message->OMEGA_0;
    ephemeris.omega = rtcm_message->omega;
    ephemeris.omega_dot = rtcm_message->OMEGADOT;
    ephemeris.idot = rtcm_message->idot;

    mTokoro.process_ephemeris(ephemeris);
}

void TokoroEphemerisRtcm::handle_gal(format::rtcm::Rtcm1046Message* rtcm_message) {
    VSCOPE_FUNCTION();
    if (!mTokoro.is_gal_enabled()) return;
    if (rtcm_message->type() == 1046) {
        handle_gal_inav(rtcm_message);
    } else {
        VERBOSEF("not rtcm 1046 but rtcm %d", rtcm_message-> type());
    }
}

void TokoroEphemerisRtcm::inspect(streamline::System&, DataType const& message) {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto rtcm1019 = dynamic_cast<format::rtcm::Rtcm1019Message*>(ptr);
    if (rtcm1019) return handle_gps(rtcm1019);
    auto rtcm1042 = dynamic_cast<format::rtcm::Rtcm1042Message*>(ptr);
    if (rtcm1042) return handle_bds(rtcm1042);
    auto rtcm1046 = dynamic_cast<format::rtcm::Rtcm1046Message*>(ptr);
    if (rtcm1046) return handle_gal(rtcm1046);
}

//
//
//

void TokoroLocation::inspect(streamline::System&, DataType const& location) {
    VSCOPE_FUNCTION();
    mTokoro.update_location_information(location);
}

//
//
//

Tokoro::Tokoro(OutputConfig const& output, TokoroConfig const& config,
               scheduler::Scheduler& scheduler)
    : mOutput(output), mConfig(config), mScheduler(scheduler) {
    VSCOPE_FUNCTION();
    mGenerator = std::unique_ptr<generator::tokoro::Generator>(new generator::tokoro::Generator{});
    mReferenceStation = nullptr;
    mPeriodicTask     = nullptr;

    mGenerator->set_iod_consistency_check(mConfig.iod_consistency_check);
    mGenerator->set_rtoc(mConfig.rtoc);
    mGenerator->set_ocit(mConfig.ocit);

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
        mReferenceStation =
            mGenerator->define_reference_station(generator::tokoro::ReferenceStationConfig{
                Float3{
                    mConfig.fixed_itrf_x,
                    mConfig.fixed_itrf_y,
                    mConfig.fixed_itrf_z,
                },
                Float3{
                    mConfig.fixed_rtcm_x,
                    mConfig.fixed_rtcm_y,
                    mConfig.fixed_rtcm_z,
                },
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
            if (output.print) {
                XINFOF(OUTPUT_PRINT_MODULE, "rtcm: %04d (%zd bytes)", submessage.id(), size);
            }

            output.interface->write(buffer, size);
        }
    }
}

void Tokoro::inspect(streamline::System&, DataType const& message) {
    VSCOPE_FUNCTION();
    ASSERT(mGenerator, "generator is null");

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
}

#endif

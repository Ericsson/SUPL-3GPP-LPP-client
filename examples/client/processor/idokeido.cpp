#if defined(INCLUDE_GENERATOR_IDOKEIDO)
#include "idokeido.hpp"

#include <generator/rtcm/generator.hpp>
#include <loglet/loglet.hpp>

#include <fstream>
#include <iostream>
#include <ostream>

LOGLET_MODULE2(p, ido);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, ido)

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_gps_lnav(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());

    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) {
        WARNF("failed to decode GPS LNAV subframe");
        return;
    }

    // Extract ionospheric parameters
    if (subframe.how.subframe_id == 4) {
        if (subframe.subframe4.sv_id == 56) {
            idokeido::KlobucharModelParameters params{
                .a = {subframe.subframe4.page18.a[0], subframe.subframe4.page18.a[1],
                      subframe.subframe4.page18.a[2], subframe.subframe4.page18.a[3]},
                .b = {subframe.subframe4.page18.b[0], subframe.subframe4.page18.b[1],
                      subframe.subframe4.page18.b[2], subframe.subframe4.page18.b[3]},
            };
            // TODO(ewaison): Change to a better system
            {
                std::ofstream f("klobuchar_params.txt");
                f << params.a[0] << std::endl;
                f << params.a[1] << std::endl;
                f << params.a[2] << std::endl;
                f << params.a[3] << std::endl;
                f << params.b[0] << std::endl;
                f << params.b[1] << std::endl;
                f << params.b[2] << std::endl;
                f << params.b[3] << std::endl;
                f.close();
            }
            process_klobuchar(params);
        }
    }

    ephemeris::GpsEphemeris ephemeris{};
    if (!mGpsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) {
        return;
    }

    process_ephemeris(ephemeris);
}

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_gps(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (sfrbx->sig_id() == 0) {
        handle_gps_lnav(sfrbx);
    } else {
        VERBOSEF("unsupported GPS signal id %d", sfrbx->sig_id());
    }
}

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_gal_inav(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());

    format::nav::gal::InavWord word{};
    if (!format::nav::gal::InavWord::decode(words, word)) {
        WARNF("failed to decode GAL INAV word");
        return;
    }

    ephemeris::GalEphemeris ephemeris{};
    if (!mGalCollector.process(sfrbx->sv_id(), word, ephemeris)) return;

    process_ephemeris(ephemeris);
}

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_gal(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (sfrbx->sig_id() == 5) {
        handle_gal_inav(sfrbx);
    } else {
        VERBOSEF("unsupported GAL signal id %d", sfrbx->sig_id());
    }
}

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_bds_d1(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    auto words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());

    format::nav::D1Subframe subframe{};
    if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) {
        WARNF("failed to decode BDS D1 subframe");
        return;
    }

    ephemeris::BdsEphemeris ephemeris{};
    if (!mBdsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    process_ephemeris(ephemeris);
}

template <typename T>
void IdokeidoEphemerisUbx<T>::handle_bds(format::ubx::RxmSfrbx* sfrbx) {
    VSCOPE_FUNCTION();
    if (sfrbx->sig_id() == 0) {
        handle_bds_d1(sfrbx);
    } else {
        VERBOSEF("unsupported BDS signal id %d", sfrbx->sig_id());
    }
}

template <typename T>
void IdokeidoEphemerisUbx<T>::inspect(streamline::System&, DataType const& message, uint64_t) {
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

static SatelliteId satellite_id_from_ubx(uint8_t gnss_id, uint8_t sv_id) {
    if (gnss_id == 0) {
        return SatelliteId::from_gps_prn(sv_id);
    } else if (gnss_id == 2) {
        return SatelliteId::from_gal_prn(sv_id);
    } else if (gnss_id == 3) {
        return SatelliteId::from_bds_prn(sv_id);
    } else {
        return {};
    }
}

/*
A summary of all the signal identification schemes used in the NMEA protocol and the UBX protocol
is provided in the following table. (Only a subset of the signals is supported by each product.) In
the NMEA protocol, system and signal identifiers are in hexadecimal format. An unknown signal
identifier is presented as 0 in the NMEA protocol.
UBX Protocol NMEA Protocol 4.10 NMEA Protocol 4.11
Signal gnssId sigId System ID Signal ID System ID Signal ID
GPS L1C/A 2 0 0 1 1 1 1
GPS L2 CL 0 3 1 6 1 6
GPS L2 CM 0 4 1 5 1 5
GPS L5 I 0 6 1 7 1 7
GPS L5 Q 0 7 1 8 1 8
SBAS L1C/A 2 1 0 1 1 1 1
Galileo E1 C2 2 0 3 7 3 7
Galileo E1 B2 2 1 3 7 3 7
Galileo E5 aI 2 3 3 1 3 1
Galileo E5 aQ 2 4 3 1 3 1
Galileo E5 bI 2 5 3 2 3 2
Galileo E5 bQ 2 6 3 2 3 2
Galileo E6 B 2 8 3 5 3 5
Galileo E6 C 2 9 3 5 3 5
Galileo E6 A 2 10 3 4 3 4
BeiDou B1I D1 2 3 0 (4)3 (1)4 4 1
BeiDou B1I D2 2 3 1 (4)3 (1)4 4 1
BeiDou B2I D1 3 2 (4)3 (3)4 4 B
BeiDou B2I D2 3 3 (4)3 (3)4 4 B
BeiDou B3I D1 3 4 (4)3 N/A 4 8
BeiDou B3I D2 3 10 (4)3 N/A 4 8
BeiDou B1 Cp (pilot) 3 5 (4)3 N/A 4 3
BeiDou B1 Cd (data) 3 6 (4)3 N/A 4 3
BeiDou B2 ap (pilot) 3 7 (4)3 N/A 4 5
BeiDou B2 ad (data) 3 8 (4)3 N/A 4 5
QZSS L1C/A 2 5 0 (1)3 (1)4 5 1
QZSS L1S 5 1 (1)3 (4)4 5 4
QZSS L2 CM 5 4 (1)3 (5)4 5 5
QZSS L2 CL 5 5 (1)3 (6)4 5 6
QZSS L5 I 5 8 (1)3 N/A 5 7
QZSS L5 Q 5 9 (1)3 N/A 5 8
QZSS L1C/B 2 5 12 (1)3 N/A 5 N/A
GLONASS L1 OF 2 6 0 2 1 2 1
GLONASS L2 OF 6 2 2 3 2 3
NavIC L5 A 2 7 0 N/A N/A 6 1
Table 4: Signal identifier

2 This signal belongs to the group of signals reported in the UBX messages that do not have an
explicit sigId field. 3 While not defined by NMEA 4.10, in this mode, u-blox receivers use system ID
4 for BeiDou and, if extended satellite numbering is enabled, system ID 1 for QZSS. 4 BeiDou and
QZSS signal ID are not defined in the NMEA protocol version 4.10. Values shown in the table are only
valid for u-blox products and, for QZSS signal ID, if extended satellite numbering is enabled.
UBXDOC-304424225-19967 - R01 1 General information Page 16 of 224
C1-Public
u-blox F20 HPG 2.02 - Interface description
UBX Protocol NMEA Protocol 4.10 NMEA Protocol 4.11
Signal gnssId sigId System ID Signal ID System ID Signal ID
*/

static SignalId signal_id_from_ubx(uint8_t gnss_id, uint8_t sv_id, uint8_t sig_id) {
    if (gnss_id == 0) {
        if (sig_id == 0) return SignalId::GPS_L1_CA;
        if (sig_id == 3) return SignalId::GPS_L2_L2C_L;
        if (sig_id == 4) return SignalId::GPS_L2_L2C_M;
        if (sig_id == 6) return SignalId::GPS_L5_I;
        if (sig_id == 7) return SignalId::GPS_L5_Q;
        return {};
    } else if (gnss_id == 1) {
        if (sig_id == 0) return SignalId::GLONASS_G1_CA;
        if (sig_id == 2) return SignalId::GLONASS_G2_CA;
        return {};
    } else if (gnss_id == 2) {
        if (sig_id == 0) return SignalId::GALILEO_E1_C_NO_DATA;
        if (sig_id == 1) return SignalId::GALILEO_E1_B_I_NAV_OS_CS_SOL;
        if (sig_id == 3) return SignalId::GALILEO_E5A_I;
        if (sig_id == 4) return SignalId::GALILEO_E5A_Q;
        if (sig_id == 5) return SignalId::GALILEO_E5B_I;
        if (sig_id == 6) return SignalId::GALILEO_E5B_Q;
        if (sig_id == 8) return SignalId::GALILEO_E6_B;
        if (sig_id == 9) return SignalId::GALILEO_E6_C;
        if (sig_id == 10) return SignalId::GALILEO_E6_A;
        return {};
    } else if (gnss_id == 3) {
        if (sig_id == 0) return SignalId::BEIDOU_B1_I;
        if (sig_id == 1) return SignalId::BEIDOU_B1_Q;
        if (sig_id == 2) return SignalId::BEIDOU_B2_I;
        if (sig_id == 3) return SignalId::BEIDOU_B2_Q;
        if (sig_id == 4) return SignalId::BEIDOU_B3_I;
        if (sig_id == 10) return SignalId::BEIDOU_B3_Q;
        if (sig_id == 5) return SignalId::BEIDOU_B1C_P;
        if (sig_id == 6) return SignalId::BEIDOU_B1C_D;
        if (sig_id == 7) return SignalId::BEIDOU_B2A_P;
        if (sig_id == 8) return SignalId::BEIDOU_B2A_D;
        return {};
    } else {
        return {};
    }
}

template <typename T>
void IdokeidoMeasurmentUbx<T>::handle(format::ubx::UbxRxmRawx* rawx) {
    VSCOPE_FUNCTION();
    if (!rawx) return;

    for (auto& m : rawx->measurements()) {
        if (!m.trk_stat.pr_valid) continue;
        if (!m.trk_stat.cp_valid) continue;

        auto satellite_id = satellite_id_from_ubx(m.gnss_id, m.sv_id);
        if (!satellite_id.is_valid()) continue;

        auto signal_id = signal_id_from_ubx(m.gnss_id, m.sv_id, m.sig_id);
        if (!signal_id.is_valid()) continue;

        auto tow          = rawx->rcv_tow();
        auto week         = rawx->week();
        auto tow_integer  = static_cast<int64_t>(tow);
        auto tow_fraction = tow - tow_integer;
        auto time         = ts::Gps::from_week_tow(week, tow_integer, tow_fraction);

        idokeido::RawMeasurement observation{
            .time          = ts::Tai{time},
            .satellite_id  = satellite_id,
            .signal_id     = signal_id,
            .pseudo_range  = m.pr_mes,
            .carrier_phase = m.cp_mes,
            .doppler       = m.do_mes,
            .snr           = m.cno * 1.0,
            .lock_time     = m.locktime * 1e-3,
        };

        measurement(observation);
    }
}

template <typename T>
void IdokeidoMeasurmentUbx<T>::inspect(streamline::System&, DataType const& message, uint64_t tag) {
    VSCOPE_FUNCTION();
    auto ptr = message.get();
    if (!ptr) return;

    auto rawx = dynamic_cast<format::ubx::UbxRxmRawx*>(ptr);
    if (!rawx) return;

    handle(rawx);
}

//
//
//

template class IdokeidoEphemerisUbx<IdokeidoSpp>;
template class IdokeidoMeasurmentUbx<IdokeidoSpp>;

//
//
//

IdokeidoSpp::IdokeidoSpp(OutputConfig const& output, IdokeidoConfig const& config,
                         scheduler::Scheduler& scheduler, streamline::System& system)
    : mOutput(output), mConfig(std::move(config)), mScheduler(scheduler), mSystem(system) {
    VSCOPE_FUNCTION();
    mComputeTask = nullptr;

    idokeido::SppConfiguration configuration{
        .relativistic_model    = config.relativistic_model,
        .ionospheric_mode      = config.ionospheric_mode,
        .weight_function       = config.weight_function,
        .epoch_selection       = config.epoch_selection,
        .gnss                  = {.gps = config.gps,
                                  .glo = config.glonass,
                                  .gal = config.galileo,
                                  .bds = config.beidou},
        .observation_window    = config.observation_window,
        .elevation_cutoff      = 15,
        .snr_cutoff            = 30,
        .outlier_cutoff        = 10,
        .reject_cycle_slip     = true,
        .reject_halfcycle_slip = true,
        .reject_outliers       = true,
    };

    mEphemerisEngine = std::unique_ptr<idokeido::EphemerisEngine>(new idokeido::EphemerisEngine{});
    if (!mConfig.ephemeris_cache.empty()) {
        VERBOSEF("using ephemeris cache: %s", mConfig.ephemeris_cache.c_str());
        mEphemerisEngine->load_or_create_cache(mConfig.ephemeris_cache);
    }

    mCorrectionCache = std::unique_ptr<idokeido::CorrectionCache>(new idokeido::CorrectionCache{});

    mEngine = std::unique_ptr<idokeido::SppEngine>(
        new idokeido::SppEngine{configuration, *mEphemerisEngine, *mCorrectionCache});
    mOutputTag = 0;

    // TODO(ewasjon): Change to a better system
    {
        std::ifstream file("klobuchar_params.txt");
        if (file.is_open()) {
            idokeido::KlobucharModelParameters params{};
            file >> params.a[0];
            file >> params.a[1];
            file >> params.a[2];
            file >> params.a[3];
            file >> params.b[0];
            file >> params.b[1];
            file >> params.b[2];
            file >> params.b[3];
            process_klobuchar(params);
        }
    }

    // Setup a periodic timer to generate every time step
    auto interval = std::chrono::milliseconds(static_cast<int>(mConfig.update_rate * 1000.0));
    mComputeTask  = std::unique_ptr<scheduler::PeriodicTask>(new scheduler::PeriodicTask(interval));
    mComputeTask->callback = [this]() {
        compute();
    };

    if (!mComputeTask->schedule(mScheduler)) {
        WARNF("failed to schedule periodic task");
    }
}

IdokeidoSpp::~IdokeidoSpp() {
    VSCOPE_FUNCTION();
}

void IdokeidoSpp::process_klobuchar(idokeido::KlobucharModelParameters const& params) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEngine, "engine is null");
    mEngine->klobuchar_model(params);
}

void IdokeidoSpp::process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEphemerisEngine, "engine is null");
    mEphemerisEngine->add(ephemeris);
}

void IdokeidoSpp::process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEphemerisEngine, "engine is null");
    mEphemerisEngine->add(ephemeris);
}

void IdokeidoSpp::process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEphemerisEngine, "engine is null");
    mEphemerisEngine->add(ephemeris);
}

void IdokeidoSpp::measurement(idokeido::RawMeasurement const& observation) NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEngine, "engine is null");
    VERBOSEF("measurement: %s %s %s", observation.satellite_id.name(), observation.signal_id.name(),
             observation.time.rtklib_time_string().c_str());

    if (observation.satellite_id.is_gps() && !mConfig.gps) return;
    if (observation.satellite_id.is_glonass() && !mConfig.glonass) return;
    if (observation.satellite_id.is_galileo() && !mConfig.galileo) return;
    if (observation.satellite_id.is_beidou() && !mConfig.beidou) return;

    mEngine->add_measurement(observation);
}

void IdokeidoSpp::compute() NOEXCEPT {
    VSCOPE_FUNCTION();
    ASSERT(mEngine, "engine is null");
    auto solution = mEngine->evaluate();

    if (solution.status == idokeido::Solution::Status::None) {
        return;
    }

    auto semi_major = 1.0;
    auto semi_minor = 1.0;

    auto velocity   = lpp::VelocityShape::horizontal(0.0, 0.0);
    auto horizontal = lpp::HorizontalAccuracy::to_ellipse_39(semi_major, semi_minor, 0.0);
    auto vertical   = lpp::VerticalAccuracy::from_1sigma(1.0);

    lpp::LocationInformation location{};
    location.time     = solution.time;
    location.location = lpp::LocationShape::ha_ellipsoid_altitude_with_uncertainty(
        solution.latitude, solution.longitude, solution.altitude, horizontal, vertical);
    location.velocity = velocity;
    mSystem.push(std ::move(location));

    lpp::HaGnssMetrics metrics{};
    metrics.fix_quality          = lpp::FixQuality::INVALID;
    metrics.number_of_satellites = static_cast<long>(solution.satellite_count);
    mSystem.push(std ::move(metrics));
}

void IdokeidoSpp::inspect(streamline::System&, DataType const& message, uint64_t) {
    VSCOPE_FUNCTION();
    ASSERT(mEngine, "engine is null");

    if (message) {
        mCorrectionCache->process(*message);
    }
}

#endif

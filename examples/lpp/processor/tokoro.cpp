#include "tokoro.hpp"
#include "ssr_example.h"
#include "ubx.hpp"

#ifdef INCLUDE_GENERATOR_TOKORO
#include <math.h>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>

#include <generator/rtcm/generator.hpp>
#include <generator/tokoro/coordinate.hpp>
#include <generator/tokoro/generator.hpp>
#include <loglet/loglet.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "p/tokoro"

using namespace streamline;
using namespace generator::tokoro;
using namespace format::ubx;

class EphemerisExtractor : public Inspector<UbxMessage> {
public:
    EphemerisExtractor(Generator& generator) : mGenerator(generator) {}

    void handle_gps_lnav(RxmSfrbx* sfrbx);
    void handle_gps(RxmSfrbx* sfrbx);

    void handle_gal_inav(RxmSfrbx* sfrbx);
    void handle_gal(RxmSfrbx* sfrbx);

    void handle_bds_d1(RxmSfrbx* sfrbx);
    void handle_bds(RxmSfrbx* sfrbx);

    void inspect(System&, DataType const& message) override;

private:
    Generator&                                 mGenerator;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};

void EphemerisExtractor::handle_gps_lnav(RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());

    format::nav::gps::lnav::Subframe subframe{};
    if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) return;

    ephemeris::GpsEphemeris ephemeris{};
    if (!mGpsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_gps(RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 0) {
        handle_gps_lnav(sfrbx);
    }
}

void EphemerisExtractor::handle_gal_inav(format::ubx::RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());

    format::nav::gal::InavWord word{};
    if (!format::nav::gal::InavWord::decode(words, word)) return;

    ephemeris::GalEphemeris ephemeris{};
    if (!mGalCollector.process(sfrbx->sv_id(), word, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_gal(format::ubx::RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 5) {
        handle_gal_inav(sfrbx);
    }
}

void EphemerisExtractor::handle_bds_d1(format::ubx::RxmSfrbx* sfrbx) {
    auto words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());

    format::nav::D1Subframe subframe{};
    if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) return;

    ephemeris::BdsEphemeris ephemeris{};
    if (!mBdsCollector.process(sfrbx->sv_id(), subframe, ephemeris)) return;

    mGenerator.process_ephemeris(ephemeris);
}

void EphemerisExtractor::handle_bds(format::ubx::RxmSfrbx* sfrbx) {
    if (sfrbx->sig_id() == 0) {
        handle_bds_d1(sfrbx);
    } else if (sfrbx->sig_id() == 1) {
        // handle_bds_d2(sfrbx);
    }
}

void EphemerisExtractor::inspect(streamline::System&, DataType const& message) {
    auto ptr = message.get();
    if (!ptr) return;

    auto sfrbx = dynamic_cast<RxmSfrbx*>(ptr);
    if (!sfrbx) return;

#if 1
    if (sfrbx->gnss_id() == 0) {
        handle_gps(sfrbx);
    } else if (sfrbx->gnss_id() == 2) {
        handle_gal(sfrbx);
    } else if (sfrbx->gnss_id() == 3) {
        handle_bds(sfrbx);
    }
#endif
}

#define TRIMBLE_TEST 1

class SsrEvaluator : public Inspector<LppMessage> {
public:
    SsrEvaluator(OutputOptions const& options) : mGenerator(), mOptions(options) {}

    Generator& generator() { return mGenerator; }

    void inspect(System&, DataType const& message) override {
        if (!message) return;

#if 1
        mGenerator.process_lpp(*message.get());

        // auto r_x = 3226.697e3;
        // auto r_y = 902.44e3;
        // auto r_z = 5409.136e3;

        auto t = mGenerator.last_correction_data_time();
        printf("time: %s %24li %24f\n", ts::Utc(t).rtklib_time_string().c_str(),
               t.timestamp().seconds(), ts::Gps{t}.time_of_week().full_seconds());
#if 0
        auto p   = EcefPosition{r_x, r_y, r_z};
        auto wgs = generator::tokoro::ecef_to_wgs84(p);

        printf("ground position: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        printf("wgs84  position: %.12f, %.12f, %.12f\n", wgs.x, wgs.y, wgs.z);
#endif

#if TRIMBLE_TEST
        printf("%li: %s %f\n", t.timestamp().seconds(), t.rtklib_time_string().c_str(),
               ts::Gps{t}.time_of_week().full_seconds());

        // 2024/09/09 12:12:46.000000000000 1725883929
        // 2024/10/01 12:10:42.000000000000 1727784605
        // 2024/12/05 07:24:27.000000000000 1733383430
        if (t.timestamp().seconds() != 1733383430) {
            printf("----------- SKIPING ----------------\n");
            return;
        }

#endif

        mReferenceStation->generate(t);
        auto messages = mReferenceStation->produce();

        // auto messages = mGenerator.generate(t, p);
        for (auto& submessage : messages) {
            printf("message: %4d: %zu bytes\n", submessage.id(), submessage.data().size());
        }

        for (auto& submessage : messages) {
            auto buffer = submessage.data().data();
            auto size   = submessage.data().size();

            // TODO(ewasjon): These message should be passed back into the system
            for (auto const& output : mOptions.outputs) {
                if ((output.format & OUTPUT_FORMAT_RTCM) != 0) {
                    output.interface->write(buffer, size);
                }
            }
        }

#if TRIMBLE_TEST
        for (;;) {
            sleep(1);
        }
#endif
#endif
    }

    void set_reference_station(std::shared_ptr<ReferenceStation> rs) {
        mReferenceStation = std::move(rs);
    }

private:
    Generator                         mGenerator;
    OutputOptions const&              mOptions;
    std::shared_ptr<ReferenceStation> mReferenceStation;
};

void tokoro_initialize(System& system, ssr_example::SsrGlobals const& globals,
                       OutputOptions const& options) {
    auto  evaluator = system.add_inspector<SsrEvaluator>(options);
    auto& generator = evaluator->generator();

    generator.set_iod_consistency_check(globals.iod_consistency_check);

#if TRIMBLE_TEST
    // Trimble Test Reference
    auto location_itrf2020 = Float3{
        3233520.957,
        859415.096,
        5412047.363,
    };
    auto location_output = location_itrf2020;
    auto rs              = generator.define_reference_station(ReferenceStationConfig{
        location_itrf2020,
        location_output,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
#elif 0
    // SE_Lin_Office ETRF89
    auto location_etrf89 = Float3{
        3227560.90670000016689,
        898383.43410000007134,
        5409177.11160000041127,
    };
    // SE_Lin_Office ITRF2020
    auto location_itrf2020_2024 = itrf_transform(Itrf::ITRF1989, Itrf::ITRF2020, 2024.0,
                                                 etrf89_to_itrf89(2024.0, location_etrf89));

    auto physical_location = Float3{
        3219441.0553999999538064,
        927415.5111000000033528,
        5409116.6926000006496906,
    };

    printf("etrf89:   %+14.4f %+14.4f %+14.4f (----.--)\n", location_etrf89.x, location_etrf89.y,
           location_etrf89.z);
    printf("itrf2020: %+14.4f %+14.4f %+14.4f (2024.00)\n", location_itrf2020_2024.x,
           location_itrf2020_2024.y, location_itrf2020_2024.z);

    auto from_epoch             = 2005.0;
    auto to_epoch               = 2024.9;
    auto location_itrf2020_1991 = itrf_transform(Itrf::ITRF1989, Itrf::ITRF2020, to_epoch,
                                                 etrf89_to_itrf89(from_epoch, location_etrf89));
    printf("itrf2020: %+14.4f %+14.4f %+14.4f (%.2f)\n", location_itrf2020_1991.x,
           location_itrf2020_1991.y, location_itrf2020_1991.z, to_epoch);
    printf("from tri: %+14.4f %+14.4f %+14.4f\n", 3227560.2384, 898383.8764, 5409177.4438);

    auto location_itrf2020 = Float3{3227560.2384, 898383.8764, 5409177.4438};
    auto location_output   = location_etrf89;

    auto rs = generator.define_reference_station(ReferenceStationConfig{
        location_itrf2020,
        location_output,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
    rs->set_physical_ground_position(physical_location);
#elif 1
    // SE_Lin_Office (new)
    auto vrs_location = Float3{
        3228520.2226,
        898382.7298,
        5408690.7539,
    };

    auto physical_location = Float3{
        3220403.2762000001966953,
        927414.9581000000471249,
        5408630.0054000001400709,
    };

    auto generate_location = Float3{
        3227560.2384,
        898383.8764,
        5409177.4438,
    };

    auto rs = generator.define_reference_station(ReferenceStationConfig{
        generate_location,
        vrs_location,
        globals.generate_gps,
        globals.generate_glonass,
        globals.generate_galileo,
        globals.generate_beidou,
    });
    rs->set_physical_ground_position(physical_location);
#endif

    rs->set_shaprio_correction(globals.shapiro_correction);
    rs->set_earth_solid_tides_correction(globals.earth_solid_tides_correction);
    rs->set_phase_windup_correction(globals.phase_windup_correction);
    rs->set_antenna_phase_variation_correction(globals.antenna_phase_variation_correction);
    rs->set_tropospheric_height_correction(globals.tropospheric_height_correction);

#if TRIMBLE_TEST
#if 0
    rs->include_satellite(SatelliteId::from_gps_prn(7));
    rs->include_satellite(SatelliteId::from_gps_prn(9));
    //rs->include_satellite(SatelliteId::from_gps_prn(13));
    rs->include_signal(SignalId::GPS_L1_CA);
#endif

#if 0
    rs->include_satellite(SatelliteId::from_gal_prn(4));
    rs->include_satellite(SatelliteId::from_gal_prn(10));
    rs->include_signal(SignalId::GALILEO_E1_B_C);
#endif

#if 0
    rs->include_satellite(SatelliteId::from_bds_prn(19));
    //rs->include_satellite(SatelliteId::from_gal_prn(10));
    //rs->include_signal(SignalId::GALILEO_E1_B_C);
#endif
#endif

    evaluator->set_reference_station(rs);

    system.add_inspector<EphemerisExtractor>(generator);
}

#endif

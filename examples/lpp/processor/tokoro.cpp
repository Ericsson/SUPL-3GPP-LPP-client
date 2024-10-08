#include "tokoro.hpp"
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

    if (sfrbx->gnss_id() == 0) {
        handle_gps(sfrbx);
    } else if (sfrbx->gnss_id() == 2) {
        handle_gal(sfrbx);
    } else if (sfrbx->gnss_id() == 3) {
        handle_bds(sfrbx);
    }
}

class SsrEvaluator : public Inspector<LppMessage> {
public:
    SsrEvaluator(OutputOptions const& options) : mGenerator(), mOptions(options) {}

    Generator& generator() { return mGenerator; }

    void inspect(System&, DataType const& message) override {
        if (!message) return;

        mGenerator.process_lpp(*message.get());

        auto r_x = 3233520.957;
        auto r_y = 859415.096;
        auto r_z = 5412047.363;

        // auto r_x = 3226.697e3;
        // auto r_y = 902.44e3;
        // auto r_z = 5409.136e3;

        auto t   = mGenerator.last_correction_data_time();
        auto p   = EcefPosition{r_x, r_y, r_z};
        auto wgs = generator::tokoro::ecef_to_wgs84(p);

        printf("ground position: %.3f, %.3f, %.3f\n", p.x, p.y, p.z);
        printf("wgs84  position: %.12f, %.12f, %.12f\n", wgs.x, wgs.y, wgs.z);

#if 0
        printf("%li: %s\n", t.timestamp().seconds(), t.rtklib_time_string().c_str());

        // 2024/10/01 12:10:43.000000000000 1727784605
        // 2024/09/09 12:12:46.000000000000 1725883929
        if (t.timestamp().seconds() != 1727784605) {
            printf("----------- SKIPING ----------------\n");
            return;
        }

#if 0
        mGenerator.include_satellite(SatelliteId::from_gps_prn(9));
        mGenerator.include_signal(SignalId::GPS_L1_CA);
#endif

#if 1
        mGenerator.include_satellite(SatelliteId::from_gal_prn(10));
        mGenerator.include_signal(SignalId::GALILEO_E1_B_C);
#endif
#endif

        auto messages = mGenerator.generate(t, p);
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
#if 1
        for (;;) {
            sleep(1);
        }
#endif
    }

private:
    Generator            mGenerator;
    OutputOptions const& mOptions;
};

void tokoro_initialize(System& system, OutputOptions const& options) {
    auto  evaluator = system.add_inspector<SsrEvaluator>(options);
    auto& generator = evaluator->generator();
    system.add_inspector<EphemerisExtractor>(generator);
}

#endif

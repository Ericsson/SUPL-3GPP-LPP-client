#include "tokoro.hpp"
#include "ubx.hpp"

#ifdef INCLUDE_GENERATOR_TOKORO
#include <math.h>

#include <ephemeris/gps.hpp>
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
    void inspect(System&, DataType const& message) override;

private:
    Generator&                                 mGenerator;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
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

void EphemerisExtractor::inspect(streamline::System&, DataType const& message) {
    auto ptr = message.get();
    if (!ptr) return;

    auto sfrbx = dynamic_cast<RxmSfrbx*>(ptr);
    if (!sfrbx) return;

    if (sfrbx->gnss_id() == 0) {
        handle_gps(sfrbx);
    }
}

class SsrEvaluator : public Inspector<LppMessage> {
public:
    SsrEvaluator(OutputOptions const& options) : mGenerator(), mOptions(options) {}

    Generator& generator() { return mGenerator; }

    void inspect(System&, DataType const& message) override {
        if (!message) return;

        mGenerator.process_lpp(*message.get());

        auto r_x = 3234.241e3;
        auto r_y = 854.055e3;
        auto r_z = 5412.345e3;

        auto t = mGenerator.last_correction_data_time();
        auto p = EcefPosition{r_x, r_y, r_z};

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

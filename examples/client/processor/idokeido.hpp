
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_rawx.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <generator/idokeido/spp.hpp>
#include <generator/idokeido/eph.hpp>
#include <lpp/location_information.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include "config.hpp"
#include "lpp.hpp"
#include "ubx.hpp"

class IdokeidoSpp : public streamline::Inspector<lpp::Message> {
public:
    IdokeidoSpp(OutputConfig const& output, IdokeidoConfig const& config,
                scheduler::Scheduler& scheduler, streamline::System& system);
    ~IdokeidoSpp() override;

    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    void measurement(idokeido::RawObservation const& observation) NOEXCEPT;

    void compute() NOEXCEPT;

    char const* name() const NOEXCEPT override { return "IdokeidoSpp"; }
    void        inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    OutputConfig const&                        mOutput;
    IdokeidoConfig const&                      mConfig;
    scheduler::Scheduler&                      mScheduler;
    streamline::System&                        mSystem;
    std::unique_ptr<idokeido::SppEngine>       mEngine;
    std::unique_ptr<idokeido::EphemerisEngine> mEphemerisEngine;
    uint64_t                                   mOutputTag;
    std::unique_ptr<scheduler::PeriodicTask>   mComputeTask;
};

template <typename Base>
class IdokeidoEphemerisUbx : public streamline::Inspector<UbxMessage> {
public:
    IdokeidoEphemerisUbx(Base& base) : mBase(base) {}

    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
        mBase.process_ephemeris(ephemeris);
    }
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
        mBase.process_ephemeris(ephemeris);
    }
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
        mBase.process_ephemeris(ephemeris);
    }

    void handle_gps_lnav(format::ubx::RxmSfrbx* sfrbx);
    void handle_gps(format::ubx::RxmSfrbx* sfrbx);

    void handle_gal_inav(format::ubx::RxmSfrbx* sfrbx);
    void handle_gal(format::ubx::RxmSfrbx* sfrbx);

    void handle_bds_d1(format::ubx::RxmSfrbx* sfrbx);
    void handle_bds(format::ubx::RxmSfrbx* sfrbx);

    char const* name() const NOEXCEPT override { return "IdokeidoEphemerisUbx"; }
    void        inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    Base&                                      mBase;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};

template <typename Base>
class IdokeidoMeasurmentUbx : public streamline::Inspector<UbxMessage> {
public:
    IdokeidoMeasurmentUbx(Base& base) : mBase(base) {}

    void measurement(idokeido::RawObservation const& observation) NOEXCEPT {
        mBase.measurement(observation);
    }

    void handle(format::ubx::UbxRxmRawx* rawx);

    char const* name() const NOEXCEPT override { return "IdokeidoMeasurmentUbx"; }
    void        inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    Base& mBase;
};

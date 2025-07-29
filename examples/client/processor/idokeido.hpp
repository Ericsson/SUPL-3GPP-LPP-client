
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_rawx.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <generator/idokeido/spp.hpp>
#include <lpp/location_information.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include "config.hpp"
#include "lpp.hpp"
#include "ubx.hpp"

class IdokeidoSpp : public streamline::Inspector<lpp::Message> {
public:
    IdokeidoSpp(OutputConfig const& output, IdokeidoConfig const& config,
                scheduler::Scheduler& scheduler);
    ~IdokeidoSpp() override;

    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    void measurement(idokeido::RawObservation const& observation) NOEXCEPT;

    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    OutputConfig const&                        mOutput;
    IdokeidoConfig const&                      mConfig;
    scheduler::Scheduler&                      mScheduler;
    std::unique_ptr<idokeido::SppEngine>       mEngine;
    std::unique_ptr<idokeido::EphemerisEngine> mEphemerisEngine;
    uint64_t                                   mOutputTag;
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

    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

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

    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    Base& mBase;
};

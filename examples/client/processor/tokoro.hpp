#pragma once

#if !defined(INCLUDE_GENERATOR_TOKORO)
#error "INCLUDE_GENERATOR_TOKORO must be defined"
#endif

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <generator/tokoro/constant.hpp>
#include <format/rtcm/1019.hpp>
#include <format/rtcm/1042.hpp>
#include <format/rtcm/1046.hpp>
#include <generator/tokoro/coordinate.hpp>
#include <generator/tokoro/generator.hpp>
#include <lpp/location_information.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include "config.hpp"
#include "lpp.hpp"
#include "ubx.hpp"
#include "rtcm.hpp"

class Tokoro : public streamline::Inspector<lpp::Message> {
public:
    Tokoro(OutputConfig const& output, TokoroConfig const& config, scheduler::Scheduler& scheduler);
    ~Tokoro() override;

    void update_location_information(lpp::LocationInformation location_information) NOEXCEPT;

    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;

    char const* name() const NOEXCEPT override { return "Tokoro"; }
    void        inspect(streamline::System&, DataType const& message, uint64_t tag) override;

    void vrs_mode_fixed();
    void vrs_mode_dynamic();
    void generate(ts::Tai const& generation_time);

    bool is_gps_enabled();
    bool is_gal_enabled();
    bool is_bds_enabled();

private:
    OutputConfig const&                                  mOutput;
    TokoroConfig const&                                  mConfig;
    scheduler::Scheduler&                                mScheduler;
    lpp::Optional<lpp::LocationShape>                    mLastUsedLocation;
    lpp::Optional<lpp::LocationShape>                    mLastLocation;
    std::unique_ptr<generator::tokoro::Generator>        mGenerator;
    std::shared_ptr<generator::tokoro::ReferenceStation> mReferenceStation;
    std::unique_ptr<scheduler::PeriodicTask>             mPeriodicTask;
    uint64_t                                             mOutputTag;
};

class TokoroEphemerisUbx : public streamline::Inspector<UbxMessage> {
public:
    TokoroEphemerisUbx(Tokoro& tokoro) : mTokoro(tokoro) {}

    void handle_gps_lnav(format::ubx::RxmSfrbx* sfrbx);
    void handle_gps(format::ubx::RxmSfrbx* sfrbx);

    void handle_gal_inav(format::ubx::RxmSfrbx* sfrbx);
    void handle_gal(format::ubx::RxmSfrbx* sfrbx);

    void handle_bds_d1(format::ubx::RxmSfrbx* sfrbx);
    void handle_bds(format::ubx::RxmSfrbx* sfrbx);

    char const* name() const NOEXCEPT override { return "TokoroEphemerisUbx"; }
    void        inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    Tokoro&                                    mTokoro;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};

class TokoroEphemerisRtcm : public streamline::Inspector<RtcmMessage> {
public:
    TokoroEphemerisRtcm(Tokoro& tokoro) : mTokoro(tokoro) {}

    void handle_gps_lnav(format::rtcm::Rtcm1019* rtcm);
    void handle_gps(format::rtcm::Rtcm1019* rtcm);

    void handle_gal_inav(format::rtcm::Rtcm1046* rtcm);
    void handle_gal(format::rtcm::Rtcm1046* rtcm);

    void handle_bds_d1(format::rtcm::Rtcm1042* rtcm);
    void handle_bds(format::rtcm::Rtcm1042* rtcm);

    const char* name() const NOEXCEPT override { return "TokoroEphemerisRtcm"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

private:
    Tokoro&                                    mTokoro;
    format::nav::gps::lnav::EphemerisCollector mGpsCollector;
    format::nav::gal::InavEphemerisCollector   mGalCollector;
    format::nav::D1Collector                   mBdsCollector;
};

class TokoroLocation : public streamline::Inspector<lpp::LocationInformation> {
public:
    TokoroLocation(Tokoro& tokoro) : mTokoro(tokoro) {}

    char const* name() const NOEXCEPT override { return "TokoroLocation"; }
    void        inspect(streamline::System&, DataType const& location, uint64_t tag) override;

private:
    Tokoro& mTokoro;
};

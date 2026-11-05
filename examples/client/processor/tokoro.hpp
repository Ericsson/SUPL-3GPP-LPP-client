#pragma once
#include "../program_io.hpp"

#if !defined(INCLUDE_GENERATOR_TOKORO)
#error "INCLUDE_GENERATOR_TOKORO must be defined"
#endif

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/glo.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#include <generator/tokoro/constant.hpp>
#include <generator/tokoro/coordinate.hpp>
#include <generator/tokoro/generator.hpp>
#include <lpp/location_information.hpp>

namespace generator {
namespace tokoro {
struct CorrectionPointSet;
}
}  // namespace generator
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#include "config.hpp"
#include "lpp.hpp"
#include "rtcm.hpp"
#include "ubx.hpp"

#ifdef ENABLE_TOKORO_SNAPSHOT
#include "snapshot/tokoro_snapshot.hpp"
#endif

class Tokoro : public streamline::Inspector<lpp::Message> {
public:
    Tokoro(ProgramOutput const& output, TokoroConfig const& config,
           scheduler::Scheduler& scheduler);
    ~Tokoro() override;

    void update_location_information(lpp::LocationInformation location_information) NOEXCEPT;

    void process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::GloEphemeris const& ephemeris) NOEXCEPT;
    void process_ephemeris(ephemeris::QzsEphemeris const& ephemeris) NOEXCEPT;

    NODISCARD char const* name() const NOEXCEPT override { return "Tokoro"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) override;

    void vrs_mode_fixed();
    void vrs_mode_grid();
    void vrs_mode_dynamic();
    void generate(ts::Tai const& generation_time);

    void set_output_tag(uint64_t tag) { mOutputTag = tag; }

    bool is_gps_enabled();
    bool is_gal_enabled();
    bool is_bds_enabled();

#ifdef ENABLE_TOKORO_SNAPSHOT
    void set_recorder(std::shared_ptr<TokoroSnapshot> recorder) { mRecorder = recorder; }
#endif

private:
    ProgramOutput const&                                 mOutput;
    TokoroConfig const&                                  mConfig;
    scheduler::Scheduler&                                mScheduler;
    streamline::System*                                  mSystem;
    lpp::Optional<lpp::LocationShape>                    mLastUsedLocation;
    lpp::Optional<lpp::LocationShape>                    mLastLocation;
    std::unique_ptr<generator::tokoro::Generator>        mGenerator;
    std::shared_ptr<generator::tokoro::ReferenceStation> mReferenceStation;
    std::unique_ptr<scheduler::PeriodicTask>             mPeriodicTask;
    ts::Tai                                              mLastGenerationTime;
    uint64_t                                             mOutputTag;
#ifdef ENABLE_TOKORO_SNAPSHOT
    std::shared_ptr<TokoroSnapshot> mRecorder;
#endif
};

class TokoroEphemerisGps : public streamline::Inspector<ephemeris::GpsEphemeris> {
public:
    TokoroEphemerisGps(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroEphemerisGps"; }
    void                  inspect(streamline::System&, DataType const& eph, uint64_t) override {
        mTokoro.process_ephemeris(eph);
    }

private:
    Tokoro& mTokoro;
};

class TokoroEphemerisGal : public streamline::Inspector<ephemeris::GalEphemeris> {
public:
    TokoroEphemerisGal(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroEphemerisGal"; }
    void                  inspect(streamline::System&, DataType const& eph, uint64_t) override {
        mTokoro.process_ephemeris(eph);
    }

private:
    Tokoro& mTokoro;
};

class TokoroEphemerisBds : public streamline::Inspector<ephemeris::BdsEphemeris> {
public:
    TokoroEphemerisBds(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroEphemerisBds"; }
    void                  inspect(streamline::System&, DataType const& eph, uint64_t) override {
        mTokoro.process_ephemeris(eph);
    }

private:
    Tokoro& mTokoro;
};

class TokoroEphemerisGlo : public streamline::Inspector<ephemeris::GloEphemeris> {
public:
    TokoroEphemerisGlo(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroEphemerisGlo"; }
    void                  inspect(streamline::System&, DataType const& eph, uint64_t) override {
        mTokoro.process_ephemeris(eph);
    }

private:
    Tokoro& mTokoro;
};

class TokoroEphemerisQzs : public streamline::Inspector<ephemeris::QzsEphemeris> {
public:
    TokoroEphemerisQzs(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroEphemerisQzs"; }
    void                  inspect(streamline::System&, DataType const& eph, uint64_t) override {
        mTokoro.process_ephemeris(eph);
    }

private:
    Tokoro& mTokoro;
};

class TokoroLocation : public streamline::Inspector<lpp::LocationInformation> {
public:
    TokoroLocation(Tokoro& tokoro) : mTokoro(tokoro) {}

    NODISCARD char const* name() const NOEXCEPT override { return "TokoroLocation"; }
    void inspect(streamline::System&, DataType const& location, uint64_t tag) override;

private:
    Tokoro& mTokoro;
};

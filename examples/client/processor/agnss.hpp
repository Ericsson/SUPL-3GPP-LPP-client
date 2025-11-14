#pragma once
#include <generator/rtcm/satellite_id.hpp>
#include <lpp/client.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/timeout.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>
#include <supl/cell.hpp>
#include <supl/identity.hpp>

#include <chrono>
#include <unordered_map>

#include "config.hpp"

struct MissingEphemeris {
    SatelliteId satellite_id;
    uint32_t    iod;
};

class MissingEphemerisPrint : public streamline::Inspector<MissingEphemeris> {
public:
    MissingEphemerisPrint(PrintConfig const& config) : mConfig(config) {}

    NODISCARD char const* name() const NOEXCEPT override { return "MissingEphemerisPrint"; }
    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

private:
    PrintConfig const& mConfig;
};

class AGnssProcessor : public streamline::Inspector<MissingEphemeris> {
public:
    AGnssProcessor(AGnssConfig const& config, supl::Identity const& identity,
                   supl::Cell const& cell, scheduler::Scheduler& scheduler,
                   streamline::System& stream);
    ~AGnssProcessor() override;

    char const* name() const NOEXCEPT override { return "AGnssProcessor"; }
    void        inspect(streamline::System& system, DataType const& message,
                        uint64_t tag) NOEXCEPT override;

private:
    void request_agnss(streamline::System& system);
    void schedule_triggered_request(streamline::System& system);

    AGnssConfig const&                                                  mConfig;
    supl::Identity                                                      mIdentity;
    supl::Cell                                                          mCell;
    scheduler::Scheduler&                                               mScheduler;
    streamline::System*                                                 mSystem;
    std::unique_ptr<lpp::Client>                                        mClient;
    std::unique_ptr<scheduler::PeriodicTask>                            mPeriodicTask;
    std::unique_ptr<scheduler::TimeoutTask>                             mTriggeredDelayTask;
    std::unordered_map<uint64_t, std::chrono::steady_clock::time_point> mRequestHistory;
    bool                                                                mTriggeredRequestPending;
};

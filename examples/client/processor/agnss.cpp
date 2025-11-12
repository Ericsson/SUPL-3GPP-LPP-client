#include "agnss.hpp"

#include <cxx11_compat.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(p, agnss);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, agnss)

void MissingEphemerisPrint::inspect(streamline::System&, DataType const& message,
                                    uint64_t tag) NOEXCEPT {
    for (auto const& print : mConfig.prints) {
        if (!print.agnss_support()) continue;
        if (!print.accept_tag(tag)) continue;
        INFOF("missing ephemeris: %s iod=%u", message.satellite_id.name(), message.iod);
        return;
    }
}

AGnssProcessor::AGnssProcessor(AGnssConfig const& config, supl::Identity const& identity,
                               supl::Cell const& cell, scheduler::Scheduler& scheduler,
                               streamline::System& stream)
    : mConfig(config), mIdentity(identity), mCell(cell), mScheduler(scheduler), mSystem(&stream),
      mTriggeredRequestPending(false) {
    FUNCTION_SCOPE();
    if (mConfig.mode == AGnssMode::Periodic || mConfig.mode == AGnssMode::Both) {
        mPeriodicTask.reset(
            new scheduler::PeriodicTask{std::chrono::seconds(mConfig.interval_seconds)});
        mPeriodicTask->set_event_name("agnss-request");
        mPeriodicTask->callback = [this]() {
            DEBUGF("periodic A-GNSS request");
            if (mSystem) request_agnss(*mSystem);
        };
        if (!mPeriodicTask->schedule(mScheduler)) {
            ERRORF("failed to schedule A-GNSS periodic task");
        }
    }
}

AGnssProcessor::~AGnssProcessor() {
    FUNCTION_SCOPE();
    if (mPeriodicTask) {
        mPeriodicTask->cancel();
    }
    if (mTriggeredDelayTask) {
        mTriggeredDelayTask->cancel();
    }
}

void AGnssProcessor::schedule_triggered_request(streamline::System& system) {
    FUNCTION_SCOPE();
    if (mTriggeredRequestPending) return;
    mTriggeredRequestPending = true;
    mSystem                  = &system;

    mTriggeredDelayTask.reset(new scheduler::TimeoutTask{std::chrono::seconds(2)});
    mTriggeredDelayTask->callback = [this]() {
        mTriggeredRequestPending = false;
        if (mSystem) request_agnss(*mSystem);
    };
    mTriggeredDelayTask->schedule(mScheduler);
}

void AGnssProcessor::inspect(streamline::System& system, DataType const& message,
                             uint64_t) NOEXCEPT {
    FUNCTION_SCOPE();
    mSystem = &system;
    if (mConfig.mode == AGnssMode::Periodic) return;

    uint64_t key = (static_cast<uint64_t>(message.satellite_id.absolute_id()) << 32) | message.iod;
    auto     now = std::chrono::steady_clock::now();
    auto     it  = mRequestHistory.find(key);
    if (it != mRequestHistory.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
        if (elapsed < mConfig.triggered_cooldown_seconds) {
            return;
        }
    }

    mRequestHistory[key] = now;
    schedule_triggered_request(system);
}

void AGnssProcessor::request_agnss(streamline::System& system) {
    FUNCTION_SCOPE();
    DEBUGF("requesting A-GNSS");

    if (mClient) {
        WARNF("A-GNSS request already in progress");
        return;
    }

    mClient = std::make_unique<lpp::Client>(mIdentity, mCell, mConfig.host, mConfig.port);
    if (mConfig.interface) {
        mClient->set_interface(*mConfig.interface);
    }

    mClient->on_connected = [](lpp::Client&) {
        DEBUGF("A-GNSS connected");
    };
    mClient->on_disconnected = [this](lpp::Client&) {
        DEBUGF("A-GNSS disconnected");
        mScheduler.defer([this]() {
            mClient.reset();
        });
    };

    if (!mClient->request_assistance_data({
            lpp::SingleRequestAssistanceData::Type::AGNSS,
            mCell,
            {mConfig.gps, mConfig.glonass, mConfig.galileo, mConfig.beidou},
            [&system](lpp::Client&, lpp::Message message) {
                DEBUGF("A-GNSS received assistance data");
                system.push(std::move(message));
            },
            [this](lpp::Client&) {
                ERRORF("A-GNSS request failed");
                mScheduler.defer([this]() {
                    mClient.reset();
                });
            },
        })) {
        WARNF("failed to request A-GNSS assistance data");
        mClient.reset();
        return;
    }

    mClient->schedule(&mScheduler);
}

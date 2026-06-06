#pragma once
#include <lpp/message.hpp>
#include <scheduler/periodic.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include <chrono>
#include <memory>
#include <vector>

/// Watches the LPP stream for messages containing static assistance data
/// (RTK reference station info or SSR correction point set). Caches only
/// those two fields and re-pushes a synthetic LPP message containing only
/// the cached data every `interval`. Cache is updated when the server sends
/// new values.
class LppStaticDataRepeat : public streamline::Inspector<lpp::Message> {
public:
    LppStaticDataRepeat(streamline::System& system, uint64_t tag,
                        std::chrono::seconds interval = std::chrono::seconds{30})
        : mSystem(system), mTag(tag), mInterval(interval) {}

    NODISCARD char const* name() const NOEXCEPT override { return "LppStaticDataRepeat"; }

    void inspect(streamline::System&, DataType const& message, uint64_t tag) NOEXCEPT override;

protected:
    bool do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT override;

private:
    streamline::System&                      mSystem;
    uint64_t                                 mTag;
    std::chrono::seconds                     mInterval;
    std::vector<uint8_t>                     mCachedUper;  // UPER of GNSS_CommonAssistData
    std::unique_ptr<scheduler::PeriodicTask> mTimer;
};

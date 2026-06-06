#pragma once
#include <lpp/message.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>
#include <streamline/inspector.hpp>
#include <streamline/system.hpp>

#include <chrono>
#include <memory>
#include <vector>

/// Watches the LPP stream for messages containing RTK reference station info or SSR
/// correction point set, caches only those fields, and re-pushes a synthetic LPP
/// message containing only the cached data every `interval`. Cache is updated when
/// the server sends new values.
class LppStaticDataRepeat : public streamline::Inspector<lpp::Message> {
public:
    LppStaticDataRepeat(streamline::System& system, uint64_t tag, scheduler::Scheduler& scheduler,
                        std::chrono::seconds interval = std::chrono::seconds{30});

    NODISCARD char const* name() const NOEXCEPT override { return "LppStaticDataRepeat"; }
    void inspect(streamline::System&, DataType const& message, uint64_t) NOEXCEPT override;

private:
    streamline::System&                      mSystem;
    uint64_t                                 mTag;
    std::vector<uint8_t>                     mCachedUper;
    std::unique_ptr<scheduler::PeriodicTask> mTimer;
};

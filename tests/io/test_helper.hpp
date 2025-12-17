#pragma once
#include <loglet/loglet.hpp>
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

struct LogletTesting {
    LogletTesting(loglet::Level level = loglet::Level::Trace) {
        loglet::initialize();
        loglet::set_level(level);
        loglet::set_color_enable(true);
    }

    ~LogletTesting() {
        loglet::set_level(loglet::Level::Disabled);
        loglet::uninitialize();
    }
};

inline bool
run_until_or_timeout(scheduler::Scheduler& sched, std::function<bool()> pred,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(250)) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (pred()) return true;
        sched.execute_timeout(std::chrono::milliseconds(25));
    }
    return false;
}

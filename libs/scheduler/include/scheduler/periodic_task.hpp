#pragma once

#include <scheduler/task.hpp>

class PeriodicTask : public Task {
public:
    PeriodicTask(std::chrono::steady_clock::duration interval);
    virtual ~PeriodicTask() = default;

    virtual void register_task(Scheduler* scheduler) override;
    virtual void event(std::chrono::steady_clock::duration difference) = 0;

protected:
    std::chrono::steady_clock::time_point mBeginning;
    std::chrono::steady_clock::time_point mNext;
    std::chrono::steady_clock::duration   mInterval;

    TimeoutEvent mTimeout;
};

class PeriodicCallbackTask : public PeriodicTask {
public:
    PeriodicCallbackTask(std::chrono::steady_clock::duration                      interval,
                         std::function<void(std::chrono::steady_clock::duration)> callback);
    virtual ~PeriodicCallbackTask() = default;

    virtual void event(std::chrono::steady_clock::duration difference) override;

protected:
    std::function<void(std::chrono::steady_clock::duration)> mCallback;
};

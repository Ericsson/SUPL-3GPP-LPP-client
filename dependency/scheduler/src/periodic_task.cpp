#include "scheduler/periodic_task.hpp"

PeriodicTask::PeriodicTask(std::chrono::steady_clock::duration interval) : mInterval(interval) {
    mBeginning = std::chrono::steady_clock::now();
    mNext      = mBeginning + mInterval;

    mTimeout.time  = mNext;
    mTimeout.event = [this]() {
        auto now        = std::chrono::steady_clock::now();
        auto difference = now - mNext;
        event(difference);
        mNext += mInterval;
        mTimeout.time = mNext;
        mScheduler->add_timeout(&mTimeout);
    };
}

void PeriodicTask::register_task(Scheduler* scheduler) {
    Task::register_task(scheduler);
    mScheduler->add_timeout(&mTimeout);
}

void PeriodicTask::unregister_task(Scheduler* scheduler) {
    Task::unregister_task(scheduler);
    mScheduler->remove_timeout(&mTimeout);
}

PeriodicCallbackTask::PeriodicCallbackTask(
    std::chrono::steady_clock::duration                      interval,
    std::function<void(std::chrono::steady_clock::duration)> callback)
    : PeriodicTask(interval), mCallback(callback) {}

void PeriodicCallbackTask::event(std::chrono::steady_clock::duration difference) {
    mCallback(difference);
}

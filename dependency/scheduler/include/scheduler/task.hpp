#pragma once
#include <scheduler/scheduler.hpp>

class Task {
public:
    virtual ~Task() = default;

    virtual void register_task(Scheduler* scheduler) { mScheduler = scheduler; }
    virtual void unregister_task(Scheduler* scheduler) { mScheduler = nullptr; }

protected:
    Scheduler* mScheduler;
};

class IoTask : public Task {
public:
    IoTask(int fd);
    virtual ~IoTask() = default;

    virtual void register_task(Scheduler* scheduler) override;
    virtual void unregister_task(Scheduler* scheduler) override;

    void event(struct epoll_event* event);

    std::function<void(int)> on_read;
    std::function<void(int)> on_write;
    std::function<void(int)> on_error;

protected:
    int        mFd;
    EpollEvent mEvent;
    bool mActive;
};

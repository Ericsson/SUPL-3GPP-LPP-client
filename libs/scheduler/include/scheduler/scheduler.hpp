#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <sys/epoll.h>
#include <vector>

class Task;

struct EpollEvent {
    std::function<void(struct epoll_event*)> event;
};

struct TimeoutEvent {
    std::chrono::steady_clock::time_point time;
    std::function<void()>                 event;
};

struct CompareTimeouts {
    bool operator()(const TimeoutEvent* lhs, const TimeoutEvent* rhs) const {
        return lhs->time > rhs->time;
    }
};

class Scheduler {
public:
    Scheduler();
    ~Scheduler();

    void schedule(Task* task);
    void cancel(Task* task);
    void execute_forever();

    void add_epoll_fd(int fd, uint32_t events, EpollEvent* event);
    void remove_epoll_fd(int fd);
    void add_timeout(TimeoutEvent* event);

protected:
    TimeoutEvent* next_timeout() const;

private:
    int mEpollFd;
    int mInterruptFd;

    std::priority_queue<TimeoutEvent*, std::vector<TimeoutEvent*>, CompareTimeouts> mTimeouts;
};

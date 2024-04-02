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

    // Schedule a task to run.
    void schedule(Task* task);
    // Cancel a task from running.
    void cancel(Task* task);
    // Run the scheduler until there are no more tasks to run.
    void execute();

    // Add, remove, and update file descriptors in the epoll instance.
    bool add_epoll_fd(int fd, uint32_t events, EpollEvent* event);
    bool update_epoll_fd(int fd, uint32_t events, EpollEvent* event);
    bool remove_epoll_fd(int fd);

    // Add and remove timeouts.
    void add_timeout(TimeoutEvent* event);
    void remove_timeout(TimeoutEvent* event);

protected:
    TimeoutEvent* next_timeout() const;

private:
    int mEpollFd;
    int mInterruptFd;
    int mEpollCount;

    std::priority_queue<TimeoutEvent*, std::vector<TimeoutEvent*>, CompareTimeouts> mTimeouts;
};

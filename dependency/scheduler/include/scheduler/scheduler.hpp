#pragma once
#include <core/core.hpp>

#include <chrono>
#include <functional>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

namespace scheduler {
struct EpollEvent {
    char const*                              name = nullptr;
    std::function<void(struct epoll_event*)> event;
};

class Scheduler {
public:
    Scheduler() NOEXCEPT;
    ~Scheduler() NOEXCEPT;

    void execute() NOEXCEPT;
    void execute_timeout(std::chrono::steady_clock::duration duration) NOEXCEPT;
    void execute_while(std::function<bool()> condition) NOEXCEPT;
    void execute_once() NOEXCEPT;
    void yield() NOEXCEPT;
    void interrupt() NOEXCEPT;

    void register_tick(void* unique_ptr, std::function<void()> callback) NOEXCEPT;
    void unregister_tick(void* unique_ptr) NOEXCEPT;

    void defer(std::function<void(Scheduler&)> callback) NOEXCEPT;

    // Add, remove, and update file descriptors in the epoll instance.
    NODISCARD bool add_epoll_fd(int fd, uint32_t events, EpollEvent* event) NOEXCEPT;
    NODISCARD bool update_epoll_fd(int fd, uint32_t events, EpollEvent* event) NOEXCEPT;
    NODISCARD bool remove_epoll_fd(int fd) NOEXCEPT;

    void set_max_events_per_wait(int max_events) NOEXCEPT { mMaxEventsPerWait = max_events; }
    int  max_events_per_wait() const NOEXCEPT { return mMaxEventsPerWait; }

private:
    void process_event(struct epoll_event& event) NOEXCEPT;
    void tick_callbacks();
    void process_deferred();

    int  mEpollFd;
    int  mInterruptFd;
    int  mEpollCount;
    int  mMaxEventsPerWait;
    bool mInterrupted;

    struct epoll_event mEvents[32];

    std::unordered_map<void*, std::function<void()>>        mTickCallbacks;
    std::vector<std::function<void(scheduler::Scheduler&)>> mDeferredCallbacks;
    std::unordered_map<int, EpollEvent*>                    mFdToEvent;
    std::unordered_map<EpollEvent*, int>                    mActiveEvents;
};
}  // namespace scheduler

#pragma once
#include <core/core.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

namespace scheduler {

enum class ExecuteResult {
    Interrupted,
    Timeout,
    ConditionMet,
    NoWork,
    Error,
};

enum class EventInterest : uint32_t {
    None   = 0,
    Read   = 1 << 0,
    Write  = 1 << 1,
    Error  = 1 << 2,
    Hangup = 1 << 3,
};

inline EventInterest operator|(EventInterest a, EventInterest b) {
    return static_cast<EventInterest>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline EventInterest& operator|=(EventInterest& a, EventInterest b) {
    a = a | b;
    return a;
}

inline bool operator&(EventInterest a, EventInterest b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

struct ScheduledEvent {
    uint16_t index;
    uint16_t generation;

    ScheduledEvent() : index(UINT16_MAX), generation(0) {}
    ScheduledEvent(uint16_t idx, uint16_t gen) : index(idx), generation(gen) {}

    bool valid() const { return index != UINT16_MAX; }
    void invalidate() { index = UINT16_MAX; }

    void interests(EventInterest interests);
    void callback(std::function<void(EventInterest)> callback);
    void unregister();

    static ScheduledEvent invalid() { return ScheduledEvent(); }
};

struct EventSlot {
    std::function<void(EventInterest)> callback;
    std::string                        name;
    int                                fd           = -1;
    uint16_t                           generation   = 0;
    bool                               in_use       = false;
    bool                               pending_free = false;
};

class Scheduler {
public:
    static constexpr int MAX_EVENT_SLOTS = 256;

    Scheduler() NOEXCEPT;
    ~Scheduler() NOEXCEPT;

    ExecuteResult execute() NOEXCEPT;
    ExecuteResult execute_timeout(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ExecuteResult execute_while(std::function<bool()> condition) NOEXCEPT;
    ExecuteResult execute_once() NOEXCEPT;
    void          yield() NOEXCEPT;
    void          interrupt() NOEXCEPT;

    void defer(std::function<void(Scheduler&)> callback) NOEXCEPT;

    NODISCARD ScheduledEvent register_fd(int fd, EventInterest interests,
                                         std::function<void(EventInterest)> callback,
                                         std::string                        name) NOEXCEPT;
    void update_interests(ScheduledEvent event, EventInterest interests) NOEXCEPT;
    void update_callback(ScheduledEvent                     event,
                         std::function<void(EventInterest)> callback) NOEXCEPT;
    void unregister(ScheduledEvent event) NOEXCEPT;

    void set_max_events_per_wait(int max_events) NOEXCEPT { mMaxEventsPerWait = max_events; }
    int  max_events_per_wait() const NOEXCEPT { return mMaxEventsPerWait; }

private:
    void process_event(struct epoll_event& event) NOEXCEPT;
    void process_deferred();

    NODISCARD EventSlot* get_slot(ScheduledEvent handle) NOEXCEPT;
    NODISCARD bool       is_stale(ScheduledEvent handle) NOEXCEPT;

    static uint64_t       encode_handle(ScheduledEvent h) NOEXCEPT;
    static ScheduledEvent decode_handle(uint64_t v) NOEXCEPT;
    static uint32_t       interests_to_epoll(EventInterest i) NOEXCEPT;
    static EventInterest  epoll_to_interests(uint32_t e) NOEXCEPT;

    int  mEpollFd;
    int  mInterruptFd;
    int  mEpollCount;
    int  mMaxEventsPerWait;
    bool mInterrupted;

    struct epoll_event mEvents[32];

    EventSlot mEventPool[MAX_EVENT_SLOTS];

    std::vector<std::function<void(scheduler::Scheduler&)>> mDeferredCallbacks;
};

namespace detail {
extern Scheduler* current_scheduler;
}

inline Scheduler& current() {
    CORE_ASSERT(detail::current_scheduler != nullptr, "no scheduler set");
    return *detail::current_scheduler;
}

inline void set_current(Scheduler* scheduler) {
    detail::current_scheduler = scheduler;
}

inline bool has_current() {
    return detail::current_scheduler != nullptr;
}

template <typename T>
inline void defer(T&& callback) {
    current().defer(std::forward<T>(callback));
}

inline void interrupt() {
    current().interrupt();
}

class ScopedScheduler : public Scheduler {
public:
    ScopedScheduler() NOEXCEPT { set_current(this); }
    ~ScopedScheduler() NOEXCEPT { set_current(nullptr); }

    ScopedScheduler(ScopedScheduler const&)            = delete;
    ScopedScheduler& operator=(ScopedScheduler const&) = delete;
};

class SchedulerGuard {
public:
    EXPLICIT SchedulerGuard(Scheduler& scheduler) NOEXCEPT { set_current(&scheduler); }
    ~SchedulerGuard() NOEXCEPT { set_current(nullptr); }

    SchedulerGuard(SchedulerGuard const&)            = delete;
    SchedulerGuard& operator=(SchedulerGuard const&) = delete;
};

}  // namespace scheduler

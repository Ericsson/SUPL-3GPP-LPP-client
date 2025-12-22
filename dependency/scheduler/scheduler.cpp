#include "scheduler/scheduler.hpp"

#include <cstring>
#include <sched.h>
#include <stdexcept>
#include <sys/eventfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE(sched);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(sched)

namespace scheduler {

void ScheduledEvent::interests(EventInterest interests) {
    if (valid()) current().update_interests(*this, interests);
}

void ScheduledEvent::callback(std::function<void(EventInterest)> cb) {
    if (valid()) current().update_callback(*this, std::move(cb));
}

void ScheduledEvent::unregister() {
    if (valid()) {
        current().unregister(*this);
        invalidate();
    }
}

Scheduler::Scheduler() NOEXCEPT : mEpollFd(-1),
                                  mInterruptFd(-1),
                                  mEpollCount(0),
                                  mMaxEventsPerWait(1),
                                  mInterrupted(false) {
    VSCOPE_FUNCTION();

    mEpollFd = ::epoll_create1(0);
    VERBOSEF("::epoll_create1(0) = %d", mEpollFd);
    if (mEpollFd == -1) {
        ERRORF("failed to create epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        return;
    }

    mInterruptFd = ::eventfd(0, EFD_NONBLOCK);
    VERBOSEF("::eventfd(0, EFD_NONBLOCK) = %d", mInterruptFd);
    if (mInterruptFd == -1) {
        ERRORF("failed to create eventfd instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        close(mEpollFd);
        return;
    }

    struct epoll_event event;
    event.events  = EPOLLIN;
    event.data.fd = mInterruptFd;
    auto result   = ::epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mInterruptFd, &event);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_ADD, %d, %p) = %d", mEpollFd, mInterruptFd, &event, result);
    if (result == -1) {
        ERRORF("failed to add eventfd to epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        close(mEpollFd);
        close(mInterruptFd);
        return;
    }

    VERBOSEF("epoll_fd: %d", mEpollFd);
    VERBOSEF("interrupt_fd: %d", mInterruptFd);
}

Scheduler::~Scheduler() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd != -1) {
        auto reslut = ::close(mEpollFd);
        VERBOSEF("::close(%d) = %d", mEpollFd, reslut);
        mEpollFd = -1;
    }
    if (mInterruptFd != -1) {
        auto result = ::close(mInterruptFd);
        VERBOSEF("::close(%d) = %d", mInterruptFd, result);
        mInterruptFd = -1;
    }
}

#define EVENT_COUNT 32

ExecuteResult Scheduler::execute() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return ExecuteResult::Error;
    }

    // We must process the deferred events directly if they were added before the event loop started
    process_deferred();

    for (;;) {
        if (mEpollCount == 0) {
            DEBUGF("no file descriptors to wait for");
            return ExecuteResult::NoWork;
        }

        if (mInterrupted) {
            DEBUGF("interrupted");
            return ExecuteResult::Interrupted;
        }

        // Wait for a file descriptor to become ready.
        VERBOSEF("waiting for events (%d file descriptors)", mEpollCount);
        auto nfds = ::epoll_pwait(mEpollFd, mEvents, EVENT_COUNT, -1, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, -1, nullptr) = %d", mEpollFd, mEvents, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return ExecuteResult::Error;
        }

        VERBOSEF("%d file descriptors are ready, processing %d", nfds, mMaxEventsPerWait);
        auto to_process = nfds < mMaxEventsPerWait ? nfds : mMaxEventsPerWait;
        for (int i = 0; i < to_process; i++) {
            process_event(mEvents[i]);
        }

        process_deferred();
    }
}

ExecuteResult Scheduler::execute_timeout(std::chrono::steady_clock::duration duration) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return ExecuteResult::Error;
    }

    process_deferred();

    auto now = std::chrono::steady_clock::now();
    auto end = now + duration;

    for (;;) {
        if (mInterrupted) {
            DEBUGF("interrupted");
            return ExecuteResult::Interrupted;
        }

        // Calculate the timeout for epoll_pwait.
        now = std::chrono::steady_clock::now();
        if (now >= end) {
            VERBOSEF("timeout expired");
            return ExecuteResult::Timeout;
        }

        auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count();
        if (timeout <= 1) {
            VERBOSEF("timeout expired (less than 1 ms)");
            return ExecuteResult::Timeout;
        }

        auto timeout_ms = static_cast<int>(timeout);

        // Wait for a file descriptor to become ready. Even if the mEpollCount is 0, we still need
        // to wait for the timeout to expire.
        VERBOSEF("waiting for events (%d file descriptors, %d ms)", mEpollCount, timeout_ms);
        auto nfds = ::epoll_pwait(mEpollFd, mEvents, EVENT_COUNT, timeout_ms, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, %d, nullptr) = %d", mEpollFd, mEvents, EVENT_COUNT,
                 timeout_ms, nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                VERBOSEF("timeout expired");
                return ExecuteResult::Timeout;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return ExecuteResult::Error;
        }

        VERBOSEF("%d file descriptors are ready, processing %d", nfds, mMaxEventsPerWait);
        auto to_process = nfds < mMaxEventsPerWait ? nfds : mMaxEventsPerWait;
        for (int i = 0; i < to_process; i++) {
            now = std::chrono::steady_clock::now();
            if (now >= end) {
                VERBOSEF("timeout expired (skipping events)");
                return ExecuteResult::Timeout;
            }

            process_event(mEvents[i]);
        }

        process_deferred();
    }
}

ExecuteResult Scheduler::execute_while(std::function<bool()> condition) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return ExecuteResult::Error;
    }

    process_deferred();

    for (;;) {
        if (mInterrupted) {
            DEBUGF("interrupted");
            return ExecuteResult::Interrupted;
        }

        if (!condition()) {
            return ExecuteResult::ConditionMet;
        }

        // Wait for a file descriptor to become ready.
        VERBOSEF("waiting for events (%d file descriptors, while)", mEpollCount);
        auto nfds = ::epoll_pwait(mEpollFd, mEvents, EVENT_COUNT, -1, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, -1, nullptr) = %d", mEpollFd, mEvents, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return ExecuteResult::Error;
        }

        VERBOSEF("%d file descriptors are ready, processing %d", nfds, mMaxEventsPerWait);
        auto to_process = nfds < mMaxEventsPerWait ? nfds : mMaxEventsPerWait;
        for (int i = 0; i < to_process; i++) {
            process_event(mEvents[i]);
        }

        process_deferred();
    }
}

ExecuteResult Scheduler::execute_once() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return ExecuteResult::Error;
    }

    process_deferred();

    for (;;) {
        if (mInterrupted) {
            DEBUGF("interrupted");
            return ExecuteResult::Interrupted;
        }

        // Wait for a file descriptor to become ready.
        VERBOSEF("waiting for events (%d file descriptors, while)", mEpollCount);
        auto nfds = ::epoll_pwait(mEpollFd, mEvents, EVENT_COUNT, 0, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, 0, nullptr) = %d", mEpollFd, mEvents, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return ExecuteResult::Error;
        }

        VERBOSEF("%d file descriptors are ready, processing %d", nfds, mMaxEventsPerWait);
        auto to_process = nfds < mMaxEventsPerWait ? nfds : mMaxEventsPerWait;
        for (int i = 0; i < to_process; i++) {
            process_event(mEvents[i]);
        }

        process_deferred();
        return nfds > 0 ? ExecuteResult::ConditionMet : ExecuteResult::NoWork;
    }
}

void Scheduler::yield() NOEXCEPT {
    VSCOPE_FUNCTION();
    VERBOSEF("yielding");
    auto result = ::sched_yield();
    VERBOSEF("::sched_yield() = %d", result);
}

void Scheduler::interrupt() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mInterruptFd == -1) {
        WARNF("interrupt_fd is not initialized");
        return;
    }

    mInterrupted    = true;
    uint64_t value  = 1;
    auto     result = ::write(mInterruptFd, &value, sizeof(value));
    VERBOSEF("::write(%d, %p, %zu) = %ld", mInterruptFd, &value, sizeof(value), result);
    if (result == -1) {
        ERRORF("failed to write to eventfd: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

void Scheduler::process_event(struct epoll_event& event) NOEXCEPT {
    if (event.data.fd == mInterruptFd) {
        DEBUGF("interrupt event");
        mInterrupted = true;
        uint64_t value;
        auto     result = ::read(mInterruptFd, &value, sizeof(value));
        VERBOSEF("::read(%d, %p, %zu) = %d", mInterruptFd, &value, sizeof(value), result);
        if (result == -1) {
            ERRORF("failed to read from eventfd: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
        return;
    }

    auto  handle = decode_handle(event.data.u64);
    auto* slot   = get_slot(handle);
    if (!slot) {
        if (is_stale(handle)) {
            VERBOSEF("event stale %04x:%04x", handle.index, handle.generation);
        } else {
            ERRORF("event %04x:%04x is not registered", handle.index, handle.generation);
        }
        return;
    }

    if (slot->callback) {
        auto before_event = std::chrono::steady_clock::now();
        auto interests    = epoll_to_interests(event.events);
        slot->callback(interests);
        auto after_event = std::chrono::steady_clock::now();
        DEBUGF("event %04x:%04x \"%s\" took %lld ms", handle.index, handle.generation,
               slot->name.c_str(),
               std::chrono::duration_cast<std::chrono::milliseconds>(after_event - before_event)
                   .count());
    }
}

void Scheduler::defer(std::function<void(scheduler::Scheduler&)> callback) NOEXCEPT {
    VERBOSEF("queue deferred callback %p", &callback);
    mDeferredCallbacks.push_back(std::move(callback));
}

void Scheduler::process_deferred() {
    for (auto& slot : mEventPool) {
        if (slot.pending_free) {
            slot.pending_free = false;
            slot.callback     = nullptr;
            slot.name.clear();
            slot.fd = -1;
        }
    }

    auto callbacks = std::move(mDeferredCallbacks);
    mDeferredCallbacks.clear();

    if (callbacks.empty()) {
        return;
    }

    DEBUGF("processing %zu deferred callbacks", callbacks.size());
    for (auto& callback : callbacks) {
        if (callback) {
            callback(*this);
        }
    }
}

uint64_t Scheduler::encode_handle(ScheduledEvent h) NOEXCEPT {
    return (static_cast<uint64_t>(h.generation) << 16) | h.index;
}

ScheduledEvent Scheduler::decode_handle(uint64_t v) NOEXCEPT {
    return {static_cast<uint16_t>(v), static_cast<uint16_t>(v >> 16)};
}

uint32_t Scheduler::interests_to_epoll(EventInterest i) NOEXCEPT {
    uint32_t e = 0;
    if (i & EventInterest::Read) e |= EPOLLIN;
    if (i & EventInterest::Write) e |= EPOLLOUT;
    if (i & EventInterest::Error) e |= EPOLLERR;
    if (i & EventInterest::Hangup) e |= EPOLLHUP | EPOLLRDHUP;
    return e;
}

EventInterest Scheduler::epoll_to_interests(uint32_t e) NOEXCEPT {
    EventInterest i = EventInterest::None;
    if (e & EPOLLIN) i = i | EventInterest::Read;
    if (e & EPOLLOUT) i = i | EventInterest::Write;
    if (e & EPOLLERR) i = i | EventInterest::Error;
    if (e & (EPOLLHUP | EPOLLRDHUP)) i = i | EventInterest::Hangup;
    return i;
}

EventSlot* Scheduler::get_slot(ScheduledEvent handle) NOEXCEPT {
    if (!handle.valid()) return nullptr;
    if (handle.index >= MAX_EVENT_SLOTS) return nullptr;
    auto& slot = mEventPool[handle.index];
    if (!slot.in_use || slot.generation != handle.generation) return nullptr;
    return &slot;
}

bool Scheduler::is_stale(ScheduledEvent handle) NOEXCEPT {
    if (!handle.valid()) return false;
    if (handle.index >= MAX_EVENT_SLOTS) return true;
    auto& slot = mEventPool[handle.index];
    return slot.pending_free || slot.generation != handle.generation;
}

ScheduledEvent Scheduler::register_fd(int fd, EventInterest interests,
                                      std::function<void(EventInterest)> callback,
                                      std::string                        name) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d, %u, %s", fd, static_cast<uint32_t>(interests), name.c_str());
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return ScheduledEvent::invalid();
    }

    int slot_index = -1;
    for (int i = 0; i < MAX_EVENT_SLOTS; i++) {
        if (!mEventPool[i].in_use && !mEventPool[i].pending_free) {
            slot_index = i;
            break;
        }
    }

    if (slot_index == -1) {
        ERRORF("event pool exhausted");
        return ScheduledEvent::invalid();
    }

    auto& slot      = mEventPool[slot_index];
    slot.callback   = std::move(callback);
    slot.name       = std::move(name);
    slot.fd         = fd;
    slot.in_use     = true;
    slot.generation = static_cast<uint16_t>((slot.generation + 1) & 0xFFFF);
    if (slot.generation == 0) slot.generation = 1;
    DEBUGF("event register %04x:%04x \"%s\"", slot_index, slot.generation, slot.name.c_str());

    ScheduledEvent handle{static_cast<uint16_t>(slot_index), slot.generation};

    struct epoll_event epoll_event;
    epoll_event.events   = interests_to_epoll(interests);
    epoll_event.data.u64 = encode_handle(handle);
    auto result          = ::epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &epoll_event);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_ADD, %d, %p) = %d", mEpollFd, fd, &epoll_event, result);
    if (result == -1) {
        ERRORF("failed to add file descriptor to epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        slot.in_use   = false;
        slot.callback = nullptr;
        slot.name.clear();
        slot.fd = -1;
        return ScheduledEvent::invalid();
    }

    mEpollCount++;
    return handle;
}

void Scheduler::update_interests(ScheduledEvent event, EventInterest interests) NOEXCEPT {
    VSCOPE_FUNCTIONF("{%u, %u}, %u", event.index, event.generation,
                     static_cast<uint32_t>(interests));
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return;
    }

    auto* slot = get_slot(event);
    if (!slot) {
        ERRORF("invalid event handle or generation mismatch");
        return;
    }

    DEBUGF("event interests %04x:%04x \"%s\" to %u", event.index, event.generation,
           slot->name.c_str(), static_cast<uint32_t>(interests));

    struct epoll_event epoll_event;
    epoll_event.events   = interests_to_epoll(interests);
    epoll_event.data.u64 = encode_handle(event);
    auto result          = ::epoll_ctl(mEpollFd, EPOLL_CTL_MOD, slot->fd, &epoll_event);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_MOD, %d, %p) = %d", mEpollFd, slot->fd, &epoll_event,
             result);
    if (result == -1) {
        ERRORF("failed to modify file descriptor in epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

void Scheduler::update_callback(ScheduledEvent                     event,
                                std::function<void(EventInterest)> callback) NOEXCEPT {
    VSCOPE_FUNCTIONF("{%u, %u}", event.index, event.generation);
    auto* slot = get_slot(event);
    if (!slot) {
        ERRORF("invalid event handle or generation mismatch");
        return;
    }

    DEBUGF("event callback %04x:%04x \"%s\"", event.index, event.generation, slot->name.c_str());
    slot->callback = std::move(callback);
}

void Scheduler::unregister(ScheduledEvent event) NOEXCEPT {
    VSCOPE_FUNCTIONF("{%u, %u}", event.index, event.generation);
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return;
    }

    auto* slot = get_slot(event);
    if (!slot) {
        ERRORF("invalid event handle or generation mismatch");
        return;
    }

    DEBUGF("event unregister %04x:%04x \"%s\"", event.index, event.generation, slot->name.c_str());
    auto result = ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, slot->fd, nullptr);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_DEL, %d, nullptr) = %d", mEpollFd, slot->fd, result);
    if (result == -1) {
        ERRORF("failed to remove file descriptor from epoll instance: " ERRNO_FMT,
               ERRNO_ARGS(errno));
    }

    slot->in_use       = false;
    slot->pending_free = true;
    mEpollCount--;
}

}  // namespace scheduler

#include "scheduler/scheduler.hpp"

#include <cstring>
#include <stdexcept>
#include <sys/eventfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE(sched);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(sched)

namespace scheduler {
Scheduler::Scheduler() NOEXCEPT : mEpollFd(-1),
                                  mInterruptFd(-1),
                                  mEpollCount(0),
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

bool Scheduler::add_epoll_fd(int fd, uint32_t events, EpollEvent* event) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d, %d, %p", fd, events, event);
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return false;
    }

    struct epoll_event epoll_event;
    epoll_event.events   = events;
    epoll_event.data.ptr = event;
    auto result          = ::epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &epoll_event);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_ADD, %d, %p) = %d", mEpollFd, fd, &epoll_event, result);
    if (result == -1) {
        ERRORF("failed to add file descriptor to epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        ERRORF("  fd: %d, events: %d", fd, events);
        return false;
    }

    mEpollCount++;
#ifndef NDEBUG
    mActiveEvents[event] = fd;
    SOFT_ASSERT(event->name != nullptr, "event name is nullptr");
    DEBUGF("tracking EpollEvent %p \"%s\" for fd %d (total: %zu)", event, event->name, fd,
           mActiveEvents.size());
#endif
    return true;
}

bool Scheduler::remove_epoll_fd(int fd) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d", fd);
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return false;
    }

#ifndef NDEBUG
    EpollEvent* removed_event = nullptr;
    for (auto it = mActiveEvents.begin(); it != mActiveEvents.end(); ++it) {
        if (it->second == fd) {
            removed_event = it->first;
            mActiveEvents.erase(it);
            SOFT_ASSERT(removed_event->name != nullptr, "event name is nullptr");
            DEBUGF("untracking EpollEvent %p \"%s\" for fd %d (remaining: %zu)", removed_event,
                   removed_event->name, fd, mActiveEvents.size());
            break;
        }
    }

    if (!removed_event) {
        ERRORF("failed to find EpollEvent for fd %d", fd);
    }
#endif

    auto result = ::epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_DEL, %d, nullptr) = %d", mEpollFd, fd, result);
    if (result == -1) {
        ERRORF("failed to remove file descriptor from epoll instance: " ERRNO_FMT,
               ERRNO_ARGS(errno));
        return false;
    }

    mEpollCount--;
    return true;
}

bool Scheduler::update_epoll_fd(int fd, uint32_t events, EpollEvent* event) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d, %d, %p", fd, events, event);
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return false;
    }

    struct epoll_event epoll_event;
    epoll_event.events   = events;
    epoll_event.data.ptr = event;
    auto reslut          = ::epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &epoll_event);
    VERBOSEF("::epoll_ctl(%d, EPOLL_CTL_MOD, %d, %p) = %d", mEpollFd, fd, &epoll_event, reslut);
    if (reslut == -1) {
        ERRORF("failed to modify file descriptor to epoll instance: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    return true;
}

// More events here would be better, but right now there are systems that remove and deallocated
// based on event. This can lead to the next event (in the same iteration) to fail as the memory is
// deallocated. TODO: solve this for better performance.
#define EVENT_COUNT 1

void Scheduler::execute() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return;
    }

    // We must process the deferred events directly if they were added before the event loop started
    process_deferred();

    struct epoll_event events[EVENT_COUNT];
    for (;;) {
        if (mEpollCount == 0) {
            DEBUGF("no file descriptors to wait for");
            return;
        }

        if (mInterrupted) {
            DEBUGF("interrupted");
            return;
        }

        tick_callbacks();

        // Wait for a file descriptor to become ready.
        VERBOSEF("waiting for events (%d file descriptors)", mEpollCount);
        auto nfds = ::epoll_pwait(mEpollFd, events, EVENT_COUNT, -1, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, -1, nullptr) = %d", mEpollFd, events, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }

        VERBOSEF("%d file descriptors are ready", nfds);
        for (int i = 0; i < nfds; i++) {
            process_event(events[i]);
        }

        process_deferred();
    }
}

void Scheduler::execute_timeout(std::chrono::steady_clock::duration duration) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return;
    }

    process_deferred();
    tick_callbacks();

    auto now = std::chrono::steady_clock::now();
    auto end = now + duration;

    struct epoll_event events[EVENT_COUNT];
    for (;;) {
        if (mInterrupted) {
            DEBUGF("interrupted");
            return;
        }

        // Calculate the timeout for epoll_pwait.
        now = std::chrono::steady_clock::now();
        if (now >= end) {
            VERBOSEF("timeout expired");
            return;
        }

        auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count();
        if (timeout <= 10) {
            VERBOSEF("timeout expired (less than 10 ms)");
            return;
        }

        auto timeout_ms = static_cast<int>(timeout);

        // Wait for a file descriptor to become ready. Even if the mEpollCount is 0, we still need
        // to wait for the timeout to expire.
        VERBOSEF("waiting for events (%d file descriptors, %d ms)", mEpollCount, timeout_ms);
        auto nfds = ::epoll_pwait(mEpollFd, events, EVENT_COUNT, timeout_ms, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %s, -1, nullptr) = %d", mEpollFd, events, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                VERBOSEF("timeout expired");
                return;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }

        VERBOSEF("%d file descriptors are ready", nfds);
        for (int i = 0; i < nfds; i++) {
            now = std::chrono::steady_clock::now();
            if (now >= end) {
                VERBOSEF("timeout expired (skipping events)");
                return;
            }

            process_event(events[i]);
        }

        process_deferred();
    }
}

void Scheduler::execute_while(std::function<bool()> condition) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEpollFd == -1 || mInterruptFd == -1) {
        ERRORF("epoll_fd or interrupt_fd is not initialized");
        return;
    }

    process_deferred();
    tick_callbacks();

    struct epoll_event events[EVENT_COUNT];
    for (;;) {
        if (mInterrupted) {
            DEBUGF("interrupted");
            return;
        }

        if (!condition()) {
            return;
        }

        // Wait for a file descriptor to become ready.
        VERBOSEF("waiting for events (%d file descriptors, while)", mEpollCount);
        auto nfds = ::epoll_pwait(mEpollFd, events, EVENT_COUNT, -1, nullptr);
        VERBOSEF("::epoll_pwait(%d, %p, %d, -1, nullptr) = %d", mEpollFd, events, EVENT_COUNT,
                 nfds);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }

        VERBOSEF("%d file descriptors are ready", nfds);
        for (int i = 0; i < nfds; i++) {
            if (!condition()) {
                return;
            }

            process_event(events[i]);
        }

        process_deferred();
    }
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
        // Clear the eventfd.
        uint64_t value;
        auto     result = ::read(mInterruptFd, &value, sizeof(value));
        VERBOSEF("::read(%d, %p, %zu) = %d", mInterruptFd, &value, sizeof(value), result);
        if (result == -1) {
            ERRORF("failed to read from eventfd: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
        return;
    }

    auto epoll_event = reinterpret_cast<EpollEvent*>(event.data.ptr);
#ifndef NDEBUG
    if (epoll_event && mActiveEvents.find(epoll_event) == mActiveEvents.end()) {
        ERRORF("========================================");
        ERRORF("CRITICAL: Use-after-free detected - attempting to process deleted event!");
        ERRORF("EpollEvent %p is not in active events map", epoll_event);
        ERRORF("This event was removed during a previous callback in this batch");
        ERRORF("Skipping event processing to prevent crash");
        ERRORF("========================================");
        return;
    }
#endif

#ifndef NDEBUG
    auto it = mActiveEvents.find(epoll_event);
    VERBOSEF("processing event for EpollEvent %p (fd %d)", epoll_event, it->second);
    auto snapshot = mActiveEvents;
#endif

    if (epoll_event) {
        auto before_event = std::chrono::steady_clock::now();
        epoll_event->event(&event);
        auto after_event = std::chrono::steady_clock::now();
        auto event_name  = epoll_event->name;
        if (!event_name) {
            event_name = "unknown";
        }
        DEBUGF("event \"%s\" took %lld ms", event_name,
               std::chrono::duration_cast<std::chrono::milliseconds>(after_event - before_event)
                   .count());
    }

#ifndef NDEBUG
    for (auto const& snap : snapshot) {
        if (mActiveEvents.find(snap.first) == mActiveEvents.end()) {
            ERRORF("========================================");
            ERRORF("CRITICAL: Event removed during callback processing!");
            ERRORF("EpollEvent %p (fd %d) was removed by the callback", snap.first, snap.second);
            ERRORF("This can cause use-after-free with EVENT_COUNT > 1");
            ERRORF("Use scheduler.defer() for deletions instead of calling cancel() directly");
            ERRORF("========================================");
        }
    }
#endif
}

void Scheduler::register_tick(void* unique_ptr, std::function<void()> callback) NOEXCEPT {
    mTickCallbacks[unique_ptr] = callback;
}

void Scheduler::unregister_tick(void* unique_ptr) NOEXCEPT {
    mTickCallbacks.erase(unique_ptr);
}

void Scheduler::tick_callbacks() {
    for (auto& callback : mTickCallbacks) {
        if (callback.second) {
            callback.second();
        }
    }
}

void Scheduler::defer(std::function<void(scheduler::Scheduler&)> callback) NOEXCEPT {
    VERBOSEF("queue deferred callback %p", &callback);
    mDeferredCallbacks.push_back(std::move(callback));
}

void Scheduler::process_deferred() {
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

}  // namespace scheduler

#include "scheduler/scheduler.hpp"
#include "scheduler/task.hpp"

#include <stdexcept>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "sched"

Scheduler::Scheduler() : mEpollFd(-1), mInterruptFd(-1), mEpollCount(0) {
    DEBUGF("Scheduler()");

    mEpollFd = epoll_create1(0);
    if (mEpollFd == -1) {
        ERRORF("failed to create epoll instance: %s", strerror(errno));
        throw std::runtime_error("Failed to create epoll instance");
    }

    mInterruptFd = eventfd(0, EFD_NONBLOCK);
    if (mInterruptFd == -1) {
        ERRORF("failed to create eventfd instance: %s", strerror(errno));
        throw std::runtime_error("Failed to create eventfd instance");
    }

    struct epoll_event event;
    event.events  = EPOLLIN;
    event.data.fd = mInterruptFd;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mInterruptFd, &event) == -1) {
        ERRORF("failed to add eventfd to epoll instance: %s", strerror(errno));
        throw std::runtime_error("Failed to add eventfd to epoll instance");
    }

    DEBUGF("epoll_fd: %d", mEpollFd);
    DEBUGF("interrupt_fd: %d", mInterruptFd);
}

Scheduler::~Scheduler() {
    DEBUGF(" ~Scheduler()");
    close(mEpollFd);
    close(mInterruptFd);
}

void Scheduler::schedule(Task* task) {
    task->register_task(this);
}

void Scheduler::cancel(Task* task) {
    task->unregister_task(this);
}

bool Scheduler::add_epoll_fd(int fd, uint32_t events, EpollEvent* event) {
    VERBOSEF("add_epoll_fd(%d, %d, %p)", fd, events, event);
    struct epoll_event epoll_event;
    epoll_event.events   = events;
    epoll_event.data.ptr = event;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &epoll_event) == -1) {
        ERRORF("failed to add file descriptor to epoll instance: %s", strerror(errno));
        return false;
    }

    mEpollCount++;
    return true;
}

bool Scheduler::remove_epoll_fd(int fd) {
    VERBOSEF("remove_epoll_fd(%d)", fd);
    if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        ERRORF("failed to remove file descriptor from epoll instance: %s", strerror(errno));
        return false;
    }

    mEpollCount--;
    return true;
}

bool Scheduler::update_epoll_fd(int fd, uint32_t events, EpollEvent* event) {
    VERBOSEF("update_epoll_fd(%d, %d, %p)", fd, events, event);
    struct epoll_event epoll_event;
    epoll_event.events   = events;
    epoll_event.data.ptr = event;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &epoll_event) == -1) {
        ERRORF("failed to modify file descriptor to epoll instance: %s", strerror(errno));
        return false;
    }

    return true;
}

void Scheduler::add_timeout(TimeoutEvent* event) {
    mTimeouts.push(event);

    // Notify the scheduler that a new timeout has been added.
    uint64_t value = 1;
    if (write(mInterruptFd, &value, sizeof(value)) == -1) {
        throw std::runtime_error("Failed to write to eventfd");
    }
}

void Scheduler::remove_timeout(TimeoutEvent* event) {
    // Is this _really_ the best way to remove an element from a priority queue?
    std::priority_queue<TimeoutEvent*, std::vector<TimeoutEvent*>, CompareTimeouts> new_queue;
    while (!mTimeouts.empty()) {
        auto timeout = mTimeouts.top();
        mTimeouts.pop();
        if (timeout != event) {
            new_queue.push(timeout);
        }
    }

    mTimeouts = new_queue;

    // Notify the scheduler that a timeout has been removed.
    uint64_t value = 1;
    if (write(mInterruptFd, &value, sizeof(value)) == -1) {
        throw std::runtime_error("Failed to write to eventfd");
    }
}

TimeoutEvent* Scheduler::next_timeout() const {
    if (mTimeouts.empty()) {
        return nullptr;
    } else {
        return mTimeouts.top();
    }
}

void Scheduler::execute() {
    VERBOSEF("execute()");

    struct epoll_event events[16];
    for (;;) {
        // Notfiy all timeouts that have expired.
        auto now = std::chrono::steady_clock::now();
        VERBOSEF("timeouts: %d", mTimeouts.size());
        while (!mTimeouts.empty()) {
            auto timeout = mTimeouts.top();
            if (!timeout) {
                mTimeouts.pop();
                continue;
            }

            // If the timeout has not expired, stop processing timeouts. To limit looping when the
            // timeout is near the current time we check that the timeout is at least 1ms.
            if (timeout->time > now) {
                if (timeout->time - now > std::chrono::milliseconds(1)) {
                    break;
                }
            }

            mTimeouts.pop();
            timeout->event();
        }

        // Find the nearest timeout.
        auto next_timeout = this->next_timeout();

        // Convert the duration to a timespec structure for nanosecond precision.
        int timeout_ms = -1;
        if (next_timeout) {
            auto timeout_duration = next_timeout->time - std::chrono::steady_clock::now();
            if (timeout_duration.count() > 0) {
                timeout_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(timeout_duration).count();

                // Cap the timeout to 5 minutes. There maximum timeout value is dependent on the a
                // lot of factors and we don't want to risk overflowing and blocking forever.
                if (timeout_ms > 1000 * 60 * 5) {
                    timeout_ms = 1000 * 60 * 5;
                }
            } else {
                next_timeout = nullptr;
            }
        }

        // If there are no epoll events and no timeouts, then we will exit the loop. 
        if (mEpollCount == 0 && !next_timeout) {
            break;
        }

        // Wait for the nearest timeout or for a file descriptor to become
        // ready.
        DEBUGF("waiting for events (timeout: %dms)", timeout_ms);
        auto nfds = epoll_pwait(mEpollFd, events, 16, timeout_ms, nullptr);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERRORF("failed to wait for events: %s", strerror(errno));
            throw std::runtime_error("Failed to wait for events");
        }

        // Handle the file descriptors that are ready.
        DEBUGF("%d file descriptors are ready", nfds);
        for (int i = 0; i < nfds; i++) {
            auto& event = events[i];
            if (event.data.fd == mInterruptFd) {
                DEBUGF("interrupt event");
                // Clear the eventfd.
                uint64_t value;
                if (read(mInterruptFd, &value, sizeof(value)) == -1) {
                    throw std::runtime_error("Failed to read from eventfd");
                }
                continue;
            }

            auto epoll_event = reinterpret_cast<EpollEvent*>(event.data.ptr);
            if (epoll_event) {
                epoll_event->event(&event);
            }
        }
    }
}

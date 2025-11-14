#pragma once
#include <core/core.hpp>

#include <cstring>
#include <mutex>
#include <queue>
#include <sys/eventfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

#ifndef EVENT_QUEUE_SIZE
#define EVENT_QUEUE_SIZE 128
#endif

LOGLET_MODULE_FORWARD_REF(streamline);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(streamline)

namespace streamline {
template <typename T>
class EventQueue {
public:
    EventQueue() { mFd = eventfd(0, EFD_NONBLOCK); }
    ~EventQueue() { close(mFd); }

    void push(T&& data) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(std::move(data));
        if (mQueue.size() > EVENT_QUEUE_SIZE) {
            WARNF("queue size limit reached (%lu > %d), discarding data", mQueue.size(),
                  EVENT_QUEUE_SIZE);
            mQueue.pop();
        } else {
            uint64_t value  = 1;
            ssize_t  result = write(mFd, &value, sizeof(value));
            if (result == -1) {
                WARNF("failed to write to eventfd: " ERRNO_FMT, ERRNO_ARGS(errno));
            }
        }
    }

    T pop() {
        std::lock_guard<std::mutex> lock(mMutex);
        auto                        data = std::move(mQueue.front());
        mQueue.pop();
        return data;
    }

    NODISCARD int get_fd() const { return mFd; }
    uint64_t      poll_count() {
        uint64_t value;
        ssize_t  result = read(mFd, &value, sizeof(value));
        (void)result;
        return value;
    }

private:
    std::queue<T> mQueue;
    std::mutex    mMutex;  // TODO(ewasjon): why have a mutex if the program is single threaded?
    int           mFd;
};
}  // namespace streamline

#undef LOGLET_CURRENT_MODULE

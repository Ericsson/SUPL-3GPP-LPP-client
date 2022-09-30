#pragma once
#include <semaphore.h>

#include "utility/types.h"

class Semaphore {
public:
    explicit Semaphore(int value) : handle() { sem_init(&handle, 0, value); }
    ~Semaphore() { sem_destroy(&handle); }

    // NOTE: not always possible to move the "handle"
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore(const Semaphore&&)           = delete;
    Semaphore& operator=(const Semaphore&&) = delete;

    void wait() { sem_wait(&handle); }

    NO_DISCARD bool wait_timeout(int ms) {
        struct timespec ts {};
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += ms / 1000;
        ts.tv_nsec += (ms % 1000) * 1'000'000;
        auto result = sem_timedwait(&handle, &ts);
        return result == 0;
    }

    NO_DISCARD bool wait_timeout(timespec ts) {
        auto result = sem_timedwait(&handle, &ts);
        return result == 0;
    }

    void post() { sem_post(&handle); }

private:
    sem_t handle;
};
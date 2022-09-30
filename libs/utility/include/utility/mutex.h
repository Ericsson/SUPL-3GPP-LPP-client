#pragma once
#include <pthread.h>

#include "utility/types.h"

class Mutex {
public:
    Mutex() { pthread_mutex_init(&handle, nullptr); }
    ~Mutex() { pthread_mutex_destroy(&handle); }

    // NOTE: not always possible to move the "handle"
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex(const Mutex&&)           = delete;
    Mutex& operator=(const Mutex&&) = delete;

    void acquire() { pthread_mutex_lock(&handle); }

    void release() { pthread_mutex_unlock(&handle); }

private:
    pthread_mutex_t handle;
};

class MutexScope {
public:
    MutexScope() = delete;
    explicit MutexScope(Mutex& mutex) : reference(mutex) { reference.acquire(); }
    ~MutexScope() { reference.release(); }

private:
    Mutex& reference;
};
